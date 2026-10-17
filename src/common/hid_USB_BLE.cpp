/**
 * @file hid_USB_BLE.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2026-02-19
 * @brief Implementation of a HID device through the tinyUSB,
 *        Bluedroid and NimBLE stacks
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "USB.h"
#include "USBHID.h"
#include "USBCDC.h"
#include "NimBLEWrapper.hpp"

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "HID_definitions.hpp"
#include "esp_mac.h"       // For esp_efuse_mac_get_default()
#include "esp32-hal-log.h" // Logging
#include <cstring>         // For memcpy
#include <stdexcept>       // For runtime_error

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

// Related to available connectivity
#if SOC_USB_OTG_SUPPORTED && CONFIG_TINYUSB_ENABLED && !USB_SERIAL_IS_DEFINED
// Note: If USB_SERIAL_IS_DEFINED, the PC shows a "driver error",
// so we disable USB HID in this case.
#define USB_AVAILABLE 1
#else
#define USB_AVAILABLE 0
#endif

#if CONFIG_NIMBLE_ENABLED || H2ZERO_NIMBLE_AVAILABLE
#define BLE_AVAILABLE 1
#else
#define BLE_AVAILABLE 0
#endif

#if !BLE_AVAILABLE && !USB_AVAILABLE
#error No support for either BLE or USB-OTG. Check Arduino IDE board config.
#endif

// This is the current interface being used,
// either DUMMY (no connection), USB or BLE.
// Other values are not permitted.
static Connectivity current_connectivity = Connectivity::DUMMY;
// BLE connection status
static bool ble_connected = false;
// USB connection status
static bool usb_connected = false;
// USB priority over BLE
static bool usb_priority = false;

// Related to automatic shutdown
static esp_timer_handle_t autoPowerOffTimer = nullptr;

// Related to HID
static uint8_t inputReportData[GAMEPAD_REPORT_SIZE] = {0};
static bool notifyConfigChanges = false;

// Forward declaration
void onBleConnectionStatus(bool connected);
void onUsbConnectionStatus(bool connected);

// DEVELOPER NOTE:
// If the device is simultaneously connected in USB and BLE,
// we take care to ignore output reports from one of them.
// Otherwise pixel control and telemetry data will be received twice,
// thus stressing the CPU and leading to potential multithreading issues.

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Automatic shutdown
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void autoPowerOffCallback(void *unused)
{
    PowerService::call().shutdown();
}

inline void start_shutdown_timer() noexcept
{
    if (autoPowerOffTimer != nullptr)
        esp_timer_start_once(
            autoPowerOffTimer,
            AUTO_POWER_OFF_DELAY_SECS * 1000000);
}

inline void stop_shutdown_timer() noexcept
{
    if (autoPowerOffTimer != nullptr)
        esp_timer_stop(autoPowerOffTimer);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// USB
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#if USB_AVAILABLE

USBHID usbHid;
// DEVELOPER NOTE:
// The logging API consumes A LOT of stack space.
// Do not set Core Debug Level to anything beyond "Error",
// otherwise a stack overflow will happen in the USB task.
// You may uncomment the following workaround for testing,
// but it takes too much memory for a production release.
// ESPUSB USB_instance(8192);
// #define USB USB_instance

class SimWheelUsbHid : public USBHIDDevice
{
    virtual uint16_t _onGetDescriptor(uint8_t *buffer) override
    {
        memcpy(buffer, hid_descriptor, sizeof(hid_descriptor));
        return sizeof(hid_descriptor);
    }

    virtual uint16_t _onGetFeature(
        uint8_t report_id,
        uint8_t *buffer,
        uint16_t len) override
    {
        return internals::hid::common::onGetFeature(report_id, buffer, len);
    }

    virtual void _onSetFeature(
        uint8_t report_id,
        const uint8_t *buffer,
        uint16_t len) override
    {
        if (current_connectivity != Connectivity::USB)
            return;
        // Note: for unknown reasons, output reports trigger this callback
        // instead of _onOutput()
        if (report_id >= RID_OUTPUT_POWERTRAIN)
            // Output report
            internals::hid::common::onOutput(report_id, buffer, len);
        else
            // Feature report
            internals::hid::common::onSetFeature(report_id, buffer, len);
    }

    virtual void _onOutput(
        uint8_t report_id,
        const uint8_t *buffer,
        uint16_t len) override
    {
        // Note: never gets called unless report_id is zero. Reason unknown.

        if (current_connectivity == Connectivity::USB)
            internals::hid::common::onOutput(report_id, buffer, len);
    }

public:
    static void event_callback(
        void *arg,
        esp_event_base_t event_base,
        int32_t event_id,
        void *event_data)
    {
        if (event_base == ARDUINO_USB_EVENTS)
        {
            switch (event_id)
            {
            case ARDUINO_USB_STARTED_EVENT:
                log_i("USB PLUGGED");
                onUsbConnectionStatus(true);
                break;
            case ARDUINO_USB_SUSPEND_EVENT:
                // DEVELOPER NOTE:
                // This should be "case ARDUINO_USB_STOPPED_EVENT:"
                // but I guess there is a bug in Arduino-ESP32
                log_i("USB UNPLUGGED");
                onUsbConnectionStatus(false);
                break;
            default:
                log_d("USB EVENT %u %u\n", event_base, event_id);
                break;
            }
        }
        else
            log_d("HID EVENT %u %u\n", event_base, event_id);
    }
} usbSimWheel;

#endif // USB_AVAILABLE

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// NimBLE
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#if BLE_AVAILABLE
uint16_t onInputReport(uint8_t reportId, uint8_t *data, uint16_t size)
{
    memcpy(data, inputReportData, size);
    return size;
}

void onNimBLEOutputReport(
    uint8_t report_id,
    const uint8_t *buffer,
    uint16_t len)
{
    if (current_connectivity == Connectivity::BLE)
        internals::hid::common::onOutput(report_id, buffer, len);
}

/**
 * @brief Convert a battery status to the data format
 *        required by the BAS specification (BLE only)
 *
 * @param status Battery status
 * @return BatteryStatusChrData Converted battery status
 */
BatteryStatusChrData toBleBatteryStatus(
    const BatteryStatus &status)
{
    BatteryStatusChrData result{};
    // Battery presence
    if (status.isBatteryPresent.value_or(false))
        result.ps_battery_present = 1;

    // Wired power
    if (status.usingExternalPower.has_value())
    {
        if (status.usingExternalPower.value())
            result.ps_wired_ext_power = 1; // = yes
        else
            result.ps_wired_ext_power = 0; // = no
    }
    else
        result.ps_wired_ext_power = 2; // = unknown

    // Charging status
    if (status.isBatteryPresent.value_or(false) &&
        status.isCharging.has_value())
    {
        result.ps_battery_charge_state =
            (status.isCharging.value())
                ? 1  // = charging
                : 2; // = discharging (active)
    }
    else
        result.ps_battery_charge_state = 0; // = unknown

    // Battery level
    // (must be identical to the value of the battery level characteristic)
    if (!status.stateOfCharge.has_value() &&
        status.isCharging.value_or(false))
    {
        // If the state of charge is unknown but the battery is charging,
        // we report a non-zero state of charge to avoid unnecessary
        // low battery warnings in the host computer.
        // This is a design choice not conforming to the BLE standard.
        result.battery_level = status.stateOfCharge.value_or(66);
    }
    else
        result.battery_level = status.stateOfCharge.value_or(0);
    if (result.battery_level > 100)
        result.battery_level = 100;

    // Battery charge level (summarized)
    if (status.stateOfCharge.has_value())
    {
        if (result.battery_level < 8)
            result.ps_battery_charge_level = 3; // = critical
        else if (result.battery_level < 15)
            result.ps_battery_charge_level = 2; // = low
        else
            result.ps_battery_charge_level = 1; // = good
    }
    else
        result.ps_battery_charge_level = 0; // = unknown

    return result;
}

#endif // BLE_AVAILABLE

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Event handling
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

inline void startBleAdvertising()
{
    // NOTE: Advertising is not started if USB-only connectivity was choosen
#if BLE_AVAILABLE
    if (BLEDevice::initialized())
        BLEAdvertising::start();
#endif
}

inline void stopBleAdvertising()
{
#if BLE_AVAILABLE
    if (BLEDevice::initialized())
        BLEAdvertising::stop();
#endif
}

inline void dropBLEconnection()
{
#if BLE_AVAILABLE
    if (BLEDevice::initialized())
        BLEAdvertising::disconnect();
#endif
}

void onStart()
{
    ble_connected = false;
    usb_connected = false;
    OnDisconnected();
    // Delay advertising a bit to have time to detect USB connection
    // vTaskDelay(pdMS_TO_TICKS(800));
    startBleAdvertising();
    start_shutdown_timer();
}

#if BLE_AVAILABLE
void onBleConnectionStatus(bool connected)
{
    if (connected)
    {
        stop_shutdown_timer();
        ble_connected = true;
        if (current_connectivity == Connectivity::DUMMY)
        {
            current_connectivity = Connectivity::BLE;
            OnConnected();
        }
    }
    else
    {
        ble_connected = false;
        if (usb_connected)
        {
            current_connectivity = Connectivity::USB;
            if (!usb_priority)
                startBleAdvertising();
        }
        else // if (!usb_connected)
        {
            current_connectivity = Connectivity::DUMMY;
            OnDisconnected();
            startBleAdvertising();
            start_shutdown_timer();
        }
    }
}
#endif // BLE_AVAILABLE

#if USB_AVAILABLE
void onUsbConnectionStatus(bool connected)
{
    if (connected)
    {
        stop_shutdown_timer();
        usb_connected = true;
        if (usb_priority)
            stopBleAdvertising();
        if (current_connectivity == Connectivity::DUMMY)
        {
            current_connectivity = Connectivity::USB;
            OnConnected();
        }
        else if (usb_priority) // && (current_connectivity==Connectivity::BLE)
        {
            current_connectivity = Connectivity::USB;
            dropBLEconnection();
        }
    }
    else
    {
        usb_connected = false;
        if (ble_connected)
            current_connectivity = Connectivity::BLE;
        else
        {
            current_connectivity = Connectivity::DUMMY;
            OnDisconnected();
            startBleAdvertising();
            start_shutdown_timer();
        }
    }
}
#endif // USB_AVAILABLE

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void internals::hid::begin(
    std::string deviceName,
    std::string deviceManufacturer,
    bool enableAutoPowerOff,
    uint16_t vendorID,
    uint16_t productID,
    bool usb_enable,
    bool ble_enable,
    bool exclusive)
{
    internals::hid::common::onReset(inputReportData);
    usb_priority = exclusive;

#if USB_AVAILABLE
    if (usb_enable)
        vendorID = 0x303A; // Expressif's licensed VID
#else
    usb_enable = false;
#endif
#if !BLE_AVAILABLE
    ble_enable = false;
#endif

    if (!usb_enable && !ble_enable)
        throw ::std::runtime_error("There is no HID connectivity");

    if (enableAutoPowerOff && !autoPowerOffTimer)
    {
        esp_timer_create_args_t args;
        args.callback = &autoPowerOffCallback;
        args.arg = nullptr;
        args.name = nullptr;
        args.dispatch_method = ESP_TIMER_TASK;
        ESP_ERROR_CHECK(esp_timer_create(&args, &autoPowerOffTimer));
    }

#if BLE_AVAILABLE
    if (!BLEDevice::initialized() && ble_enable)
    {
        // BLE initialization
        BLEAdvertising::onConnectionStatus = onBleConnectionStatus;
        BLEHIDService::onReadInputReport = onInputReport;
        BLEHIDService::onGetFeatureReport =
            internals::hid::common::onGetFeature;
        BLEHIDService::onSetFeatureReport =
            internals::hid::common::onSetFeature;
        BLEHIDService::onWriteOutputReport = onNimBLEOutputReport;
        BLEDeviceInfoService::manufacturer.name = deviceManufacturer;
        BLEDeviceInfoService::pnpInfo.vid = vendorID;
        BLEDeviceInfoService::pnpInfo.pid = productID;
        BLEDevice::init(deviceName);
    }
#endif // BLE_AVAILABLE

#if USB_AVAILABLE
    if (!usbHid.ready() && usb_enable)
    {
        // USB Initialization
        USB.productName(deviceName.c_str());
        USB.manufacturerName(deviceManufacturer.c_str());
        USB.usbClass(0x03); // HID device class
        USB.usbSubClass(0); // No subclass
        USB.PID(productID);
        uint64_t serialNumber;
        if (esp_efuse_mac_get_default((uint8_t *)(&serialNumber)) == ESP_OK)
        {
            char serialAsStr[9];
            snprintf(serialAsStr, 9, "%08llX", serialNumber);
            USB.serialNumber(serialAsStr);
        }
        usbHid.addDevice(&usbSimWheel, sizeof(hid_descriptor));
        // Note:
        // event_callback() must be subscribed to both USB and usbHid.
        // Otherwise USB events are not received.
        // Probably a bug in Arduino-ESP32.
        USB.onEvent(SimWheelUsbHid::event_callback);
        usbHid.onEvent(SimWheelUsbHid::event_callback);
        usbHid.begin();
        USB.begin();
    }
#endif // USB_AVAILABLE

    onStart();
}

// ----------------------------------------------------------------------------
// HID profile
// ----------------------------------------------------------------------------

void internals::hid::reset()
{
    internals::hid::common::onReset(inputReportData);
#if BLE_AVAILABLE
    if (current_connectivity == Connectivity::BLE)
        BLEHIDService::input_report.notify();
#endif
#if USB_AVAILABLE
    if (current_connectivity == Connectivity::USB)
        usbHid.SendReport(
            RID_INPUT_GAMEPAD,
            inputReportData,
            GAMEPAD_REPORT_SIZE);
#endif // USB_AVAILABLE
}

void internals::hid::reportInput(
    const uint128_t &inputs,
    uint8_t POVstate,
    uint8_t leftAxis,
    uint8_t rightAxis,
    uint8_t clutchAxis)
{
    internals::hid::common::onReportInput(
        inputReportData,
        notifyConfigChanges,
        inputs,
        POVstate,
        leftAxis,
        rightAxis,
        clutchAxis);
#if BLE_AVAILABLE
    if (current_connectivity == Connectivity::BLE)
        BLEHIDService::input_report.notify();
#endif // BLE_AVAILABLE
#if USB_AVAILABLE
    if (current_connectivity == Connectivity::USB)
        usbHid.SendReport(
            RID_INPUT_GAMEPAD,
            inputReportData,
            GAMEPAD_REPORT_SIZE);
#endif // USB_AVAILABLE
}

void internals::hid::reportBatteryLevel(const BatteryStatus &status)
{
#if BLE_AVAILABLE
    BatteryStatusChrData result = toBleBatteryStatus(status);
    BLEBatteryService::batteryStatus.set(result);
    BLEBatteryService::batteryLevel.set(result.battery_level);
#endif // BLE_AVAILABLE
}

void internals::hid::reportChangeInConfig()
{
    notifyConfigChanges = true; // Will be reported in the next input report
}

bool internals::hid::supportsCustomHardwareID() { return true; }

// ----------------------------------------------------------------------------
// Status
// ----------------------------------------------------------------------------

bool internals::hid::isConnected()
{
    return usb_connected || ble_connected;
}
