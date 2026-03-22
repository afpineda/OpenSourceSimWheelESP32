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

#if CONFIG_NIMBLE_ENABLED || CONFIG_BLUEDROID_ENABLED
#define BLE_AVAILABLE 1
#else
#define BLE_AVAILABLE 0
#endif

#if !BLE_AVAILABLE && !USB_AVAILABLE
#error Your board does not support BLE nor USB
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

#if CONFIG_NIMBLE_ENABLED
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
#endif // CONFIG_NIMBLE_ENABLED

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Bluedroid
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#if CONFIG_BLUEDROID_ENABLED && !CONFIG_NIMBLE_ENABLED

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEHIDDevice.h>
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#include "sdkconfig.h"
#include "esp_gap_ble_api.h"
#include <map>

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class BLEHIDDeviceFix;
static BLEHIDDeviceFix *hidDevice = nullptr;
static BLECharacteristic *inputGamePad = nullptr;
static BLECharacteristic *battStatusChr = nullptr;
static BLEServer *pServer = nullptr;
static constexpr uint16_t serialNumberChrUuid = BLE_SERIAL_NUMBER_CHR_UUID;
static constexpr uint16_t batteryStatusChrUuid = BLE_BATTERY_STATUS_CHR_UUID;

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BLEHIDDeviceFix
//
// This subclass is a workaround for a bug in Arduino-ESP32.
// Notifications get randomly disabled in the input report.
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class BLEHIDDeviceFix : public BLEHIDDevice
{
public:
    BLECharacteristic *inputReport(uint8_t reportID)
    {
        BLECharacteristic *inputReportCharacteristic =
            hidService()->createCharacteristic(
                (uint16_t)0x2a4d, BLECharacteristic::PROPERTY_READ |
                                      BLECharacteristic::PROPERTY_NOTIFY);
        BLEDescriptor *inputReportDescriptor =
            new BLEDescriptor(BLEUUID((uint16_t)0x2908));
        BLE2902 *p2902 = new BLE2902();
        inputReportCharacteristic->setAccessPermissions(
            ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
        inputReportDescriptor->setAccessPermissions(
            ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
        p2902->setAccessPermissions(
            ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
        p2902->setNotifications(true);
        p2902->setIndications(true);

        uint8_t desc1_val[] = {reportID, 0x01};
        inputReportDescriptor->setValue((uint8_t *)desc1_val, 2);
        inputReportCharacteristic->addDescriptor(p2902);
        inputReportCharacteristic->addDescriptor(inputReportDescriptor);

        return inputReportCharacteristic;
    }

    BLEHIDDeviceFix(BLEServer *pServer) : BLEHIDDevice(pServer) {}
};

uint16_t byteswap(uint16_t value)
{
    return (value >> 8) | (value << 8);
}

class BleConnectionStatus : public BLEServerCallbacks
{
public:
    void onConnect(BLEServer *pServer) override
    {
        onBleConnectionStatus(true);
    };

    void onDisconnect(BLEServer *pServer) override
    {
        onBleConnectionStatus(false);
    };

} connectionStatus;

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// HID FEATURE REQUEST callbacks
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class FeatureReport : public BLECharacteristicCallbacks
{
public:
    void onWrite(BLECharacteristic *pCharacteristic) override;
    void onRead(BLECharacteristic *pCharacteristic) override;
    FeatureReport(uint8_t RID, uint16_t size);
    static void attachTo(
        BLEHIDDeviceFix *hidDevice,
        uint8_t RID,
        uint16_t size);

private:
    uint8_t _reportID;
    uint16_t _reportSize;
};

// RECEIVE DATA
void FeatureReport::onWrite(BLECharacteristic *pCharacteristic)
{
    size_t size = pCharacteristic->getValue().length();
    const uint8_t *data = pCharacteristic->getData();
    internals::hid::common::onSetFeature(_reportID, data, size);
}

// SEND REQUESTED DATA
void FeatureReport::onRead(BLECharacteristic *pCharacteristic)
{
    uint8_t data[_reportSize];
    internals::hid::common::onGetFeature(_reportID, data, _reportSize);
    pCharacteristic->setValue(data, _reportSize);
}

// Constructor
FeatureReport::FeatureReport(uint8_t RID, uint16_t size)
{
    _reportID = RID;
    _reportSize = size;
}

// Attach to HID device
void FeatureReport::attachTo(
    BLEHIDDeviceFix *hidDevice,
    uint8_t RID,
    uint16_t size)
{
    BLECharacteristic *reportCharacteristic = hidDevice->featureReport(RID);
    if (!reportCharacteristic)
    {
        log_e("Unable to create HID report characteristics");
        abort();
    }
    reportCharacteristic->setCallbacks(new FeatureReport(RID, size));
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// HID OUTPUT REPORT callbacks
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class OutputReport : public BLECharacteristicCallbacks
{
public:
    void onWrite(BLECharacteristic *pCharacteristic) override;
    OutputReport(uint8_t RID);
    static void attachTo(BLEHIDDeviceFix *hidDevice, uint8_t RID);

private:
    uint8_t _reportID;
};

OutputReport::OutputReport(uint8_t RID)
{
    _reportID = RID;
}

// RECEIVE DATA
void OutputReport::onWrite(BLECharacteristic *pCharacteristic)
{
    if (current_connectivity == Connectivity::BLE)
    {
        size_t size = pCharacteristic->getValue().length();
        const uint8_t *data = pCharacteristic->getData();
        internals::hid::common::onOutput(_reportID, data, size);
    }
}

// Attach to HID device
void OutputReport::attachTo(BLEHIDDeviceFix *hidDevice, uint8_t RID)
{
    BLECharacteristic *reportCharacteristic = hidDevice->outputReport(RID);
    if (!reportCharacteristic)
    {
        log_e("Unable to create HID report characteristics");
        abort();
    }
    reportCharacteristic->setCallbacks(new OutputReport(RID));
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif // CONFIG_BLUEDROID_ENABLED && !CONFIG_NIMBLE_ENABLED

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Event handling
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

inline void startBleAdvertising()
{
    // NOTE: Advertising is not started if USB-only connectivity was choosen
#if CONFIG_NIMBLE_ENABLED
    if (BLEDevice::initialized())
        BLEAdvertising::start();
#elif CONFIG_BLUEDROID_ENABLED
    if (hidDevice != nullptr)
        BLEDevice::startAdvertising();
#endif
}

inline void stopBleAdvertising()
{
#if CONFIG_NIMBLE_ENABLED
    if (BLEDevice::initialized())
        BLEAdvertising::stop();
#elif CONFIG_BLUEDROID_ENABLED
    if (hidDevice != nullptr)
        BLEDevice::stopAdvertising();
#endif
}

inline void dropBLEconnection()
{
#if CONFIG_NIMBLE_ENABLED
    if (BLEDevice::initialized())
        BLEAdvertising::disconnect();
#elif CONFIG_BLUEDROID_ENABLED
    if (pServer != nullptr)
    {
        std::map<uint16_t, conn_status_t> devices =
            pServer->getPeerDevices(false);
        for (auto device : devices)
            pServer->disconnect(device.first);
    }
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
#endif

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
#endif

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

#if CONFIG_NIMBLE_ENABLED
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

#elif CONFIG_BLUEDROID_ENABLED

    if ((hidDevice == nullptr) && ble_enable)
    {
        // Stack initialization
        BLEDevice::init(String(deviceName.c_str()));
        BLEDevice::setMTU(BLE_MTU_SIZE);
        pServer = BLEDevice::createServer();
        pServer->advertiseOnDisconnect(false);
        pServer->setCallbacks(&connectionStatus);
        BLESecurity *pSecurity = new BLESecurity();
        pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
        pSecurity->setInitEncryptionKey(
            ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

        // HID initialization
        hidDevice = new BLEHIDDeviceFix(pServer);
        if (!hidDevice)
            throw std::runtime_error("Unable to create HID device");

        hidDevice->manufacturer()->setValue(
            // Workaround for bug in
            // `hidDevice->manufacturer(deviceManufacturer)`
            String(deviceManufacturer.c_str()));

        // Note: Workaround for bug in ESP-Arduino as of version 3.0.3
        uint16_t debugged_vid = byteswap(vendorID);
        uint16_t debugged_pid = byteswap(productID);

        hidDevice->pnp(
            BLE_VENDOR_SOURCE,
            debugged_vid,
            debugged_pid,
            PRODUCT_REVISION);
        hidDevice->hidInfo(0x00, 0x01);
        hidDevice->reportMap((uint8_t *)hid_descriptor, sizeof(hid_descriptor));

        // Add the serial number to the "Device Information" service
        uint64_t serialNumber;
        if (esp_efuse_mac_get_default((uint8_t *)(&serialNumber)) == ESP_OK)
        {
            BLEService *deviceInfo = hidDevice->deviceInfo();
            if (deviceInfo)
            {
                BLECharacteristic *serialNumChr =
                    deviceInfo->getCharacteristic(serialNumberChrUuid);
                if (!serialNumChr)
                    serialNumChr =
                        deviceInfo->createCharacteristic(
                            serialNumberChrUuid,
                            BLECharacteristic::PROPERTY_READ);
                if (serialNumChr)
                {
                    char serialAsStr[9];
                    memset(serialAsStr, 0, 9);
                    snprintf(serialAsStr, 9, "%08llX", serialNumber);
                    serialNumChr->setValue(serialAsStr);
                }
            }
        }

        // Expand the battery service with a
        // "battery level status" characteristic
        BLEService *battService = hidDevice->batteryService();
        assert(battService && "BLE: hidDevice->batteryService() failed");
        battStatusChr = battService->createCharacteristic(
            batteryStatusChrUuid,
            BLECharacteristic::PROPERTY_READ |
                BLECharacteristic::PROPERTY_NOTIFY);
        assert(battStatusChr &&
               "BLE: Unable to create the battery level status characteristic");
        BLE2902 *p2902 = new BLE2902();
        p2902->setNotifications(true);
        battStatusChr->addDescriptor(p2902);

        // Create HID reports
        inputGamePad = hidDevice->inputReport(RID_INPUT_GAMEPAD);
        FeatureReport::attachTo(
            hidDevice, RID_FEATURE_CAPABILITIES, CAPABILITIES_REPORT_SIZE);
        FeatureReport::attachTo(
            hidDevice, RID_FEATURE_CONFIG, CONFIG_REPORT_SIZE);
        FeatureReport::attachTo(
            hidDevice, RID_FEATURE_BUTTONS_MAP, BUTTONS_MAP_REPORT_SIZE);
        FeatureReport::attachTo(
            hidDevice, RID_FEATURE_HARDWARE_ID, HARDWARE_ID_REPORT_SIZE);
        OutputReport::attachTo(hidDevice, RID_OUTPUT_POWERTRAIN);
        OutputReport::attachTo(hidDevice, RID_OUTPUT_ECU);
        OutputReport::attachTo(hidDevice, RID_OUTPUT_RACE_CONTROL);
        OutputReport::attachTo(hidDevice, RID_OUTPUT_GAUGES);
        OutputReport::attachTo(hidDevice, RID_OUTPUT_PIXEL);

        // Configure BLE advertising
        BLEAdvertising *pAdvertising = pServer->getAdvertising();
        pAdvertising->setAppearance(HID_GAMEPAD);
        pAdvertising->setScanResponse(true);
        pAdvertising->setMinPreferred(0x06);
        pAdvertising->setMinPreferred(0x12);
        pAdvertising->addServiceUUID(hidDevice->hidService()->getUUID());
        pAdvertising->addServiceUUID(hidDevice->batteryService()->getUUID());
        pAdvertising->addServiceUUID(hidDevice->deviceInfo()->getUUID());

        // Start services
        hidDevice->startServices();
        hidDevice->setBatteryLevel(0);
        // Initialize the battery status to 100% charge and wired.
        // Otherwise, non-battery-operated firmwares may cause
        // a low battery warning in the hosting PC
        BatteryStatus defaultBatteryStatus;
        defaultBatteryStatus.stateOfCharge = 100;
        defaultBatteryStatus.usingExternalPower = true;
        internals::hid::reportBatteryLevel(defaultBatteryStatus);
    }

#endif // CONFIG_BLUEDROID_ENABLED

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
#if CONFIG_NIMBLE_ENABLED
    if (current_connectivity == Connectivity::BLE)
        BLEHIDService::input_report.notify();
#elif CONFIG_BLUEDROID_ENABLED
    if (current_connectivity == Connectivity::BLE)
    {
        inputGamePad->setValue(inputReportData, GAMEPAD_REPORT_SIZE);
        inputGamePad->notify();
    }
#endif // CONFIG_BLUEDROID_ENABLED
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
#if CONFIG_NIMBLE_ENABLED
    if (current_connectivity == Connectivity::BLE)
        BLEHIDService::input_report.notify();
#elif CONFIG_BLUEDROID_ENABLED
    if (current_connectivity == Connectivity::BLE)
    {
        inputGamePad->setValue(inputReportData, GAMEPAD_REPORT_SIZE);
        inputGamePad->notify(true);
    }
#endif // CONFIG_BLUEDROID_ENABLED
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
#if CONFIG_NIMBLE_ENABLED
    BatteryStatusChrData result =
        internals::hid::common::toBleBatteryStatus(status);
    BLEBatteryService::batteryStatus.set(result);
    BLEBatteryService::batteryLevel.set(result.battery_level);
#elif CONFIG_BLUEDROID_ENABLED
    if (ble_connected)
    {
        // -- Battery level status characteristic
        BatteryStatusChrData result =
            internals::hid::common::toBleBatteryStatus(status);
        battStatusChr->setValue(
            (const uint8_t *)(&result),
            sizeof(result));
        battStatusChr->notify();

        // -- Battery level characteristic
        hidDevice->setBatteryLevel(result.battery_level);
    }
#endif // CONFIG_NIMBLE_ENABLED
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
