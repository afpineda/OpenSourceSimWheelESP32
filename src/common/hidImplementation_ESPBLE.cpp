/**
 * @file hidImplementation_ESPBLE.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-10-31
 * @brief Implementation of a HID device through the ESP32's native BLE stack
 *
 * @copyright Licensed under the EUPL
 *
 */

// Implementation inspired by
// https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/examples/Server/Server.ino
//
// Use this app for testing:
// http://www.planetpointy.co.uk/joystick-test-application/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEHIDDevice.h>
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#include "sdkconfig.h"

#include "SimWheel.h"
#include "HID_definitions.h"
#include <arduino.h>
#include <bit>

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

// Related to auto power off
static esp_timer_handle_t autoPowerOffTimer = nullptr;

// Related to HID device
static BLEHIDDevice *hid = nullptr;
static BLECharacteristic *inputGamepad = nullptr;
static BLECharacteristic *configReport = nullptr;
static BLECharacteristic *capabilitiesReport = nullptr;
static BLECharacteristic *buttonsMapReport = nullptr;
static BLECharacteristic *hardwareIdReport = nullptr;
static BLEServer *pServer = nullptr;
static bool notifyConfigChanges = false;

// ----------------------------------------------------------------------------
// BLE Server callbacks and advertising
// ----------------------------------------------------------------------------

class BleConnectionStatus : public BLEServerCallbacks
{
public:
    BleConnectionStatus(void) {};
    bool connected = false;
    void onConnect(BLEServer *pServer) override
    {
        if (autoPowerOffTimer != nullptr)
            esp_timer_stop(autoPowerOffTimer);
        hidImplementation::reset();
        connected = true;
        notify::connected();
    };

    void onDisconnect(BLEServer *pServer) override
    {
        connected = false;
        BLEDevice::startAdvertising();
        notify::BLEdiscovering();
        if (autoPowerOffTimer != nullptr)
            esp_timer_start_once(autoPowerOffTimer, AUTO_POWER_OFF_DELAY_SECS * 1000000);
    };

} connectionStatus;

// ----------------------------------------------------------------------------
// HID FEATURE REQUEST callbacks
// ----------------------------------------------------------------------------

class ConfigFRCallbacks : public BLECharacteristicCallbacks
{
    // RECEIVE DATA
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        size_t size = pCharacteristic->getLength();
        const uint8_t *data = pCharacteristic->getData();
        hidImplementation::common::onSetFeature(RID_FEATURE_CONFIG, data, size);
    }

    // SEND REQUESTED DATA
    void onRead(BLECharacteristic *pCharacteristic)
    {
        uint8_t data[CONFIG_REPORT_SIZE];
        hidImplementation::common::onGetFeature(RID_FEATURE_CONFIG, data, CONFIG_REPORT_SIZE);
        pCharacteristic->setValue(data, CONFIG_REPORT_SIZE);
    }

} configFRCallbacks;

// ----------------------------------------------------------------------------

class CapabilitiesFRCallbacks : public BLECharacteristicCallbacks
{
    // RECEIVED DATA is ignored (read-only)

    // SEND REQUESTED DATA
    void onRead(BLECharacteristic *pCharacteristic)
    {
        uint8_t data[CAPABILITIES_REPORT_SIZE];
        hidImplementation::common::onGetFeature(RID_FEATURE_CAPABILITIES, data, CAPABILITIES_REPORT_SIZE);
        pCharacteristic->setValue(data, CAPABILITIES_REPORT_SIZE);
    }
} capabilitiesFRCallbacks;

// ----------------------------------------------------------------------------

class ButtonsMapFRCallbacks : public BLECharacteristicCallbacks
{
    inputNumber_t selectedInput = UNSPECIFIED_INPUT_NUMBER;

    // RECEIVED DATA:
    // Select firmware-defined input number or set button map
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        size_t size = pCharacteristic->getLength();
        const uint8_t *data = pCharacteristic->getData();
        hidImplementation::common::onSetFeature(RID_FEATURE_BUTTONS_MAP, data, size);
    }

    // SEND DATA:
    // Map for selected button
    void onRead(BLECharacteristic *pCharacteristic)
    {
        uint8_t data[BUTTONS_MAP_REPORT_SIZE];
        hidImplementation::common::onGetFeature(RID_FEATURE_BUTTONS_MAP, data, BUTTONS_MAP_REPORT_SIZE);
        pCharacteristic->setValue(data, BUTTONS_MAP_REPORT_SIZE);
    }

} buttonsMapFRCallbacks;

// ----------------------------------------------------------------------------

class HardwareID_FRCallbacks : public BLECharacteristicCallbacks
{
    // RECEIVE DATA
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        size_t size = pCharacteristic->getValue().length();
        const uint8_t *data = pCharacteristic->getData();
        hidImplementation::common::onSetFeature(RID_FEATURE_HARDWARE_ID, data, size);
    }

    // SEND REQUESTED DATA
    void onRead(BLECharacteristic *pCharacteristic)
    {
        uint8_t data[HARDWARE_ID_REPORT_SIZE];
        hidImplementation::common::onGetFeature(RID_FEATURE_HARDWARE_ID, data, HARDWARE_ID_REPORT_SIZE);
        pCharacteristic->setValue(data, HARDWARE_ID_REPORT_SIZE);
    }

} hardwareID_FRCallbacks;

// ----------------------------------------------------------------------------
// Auto power-off
// ----------------------------------------------------------------------------

void autoPowerOffCallback(void *unused)
{
    power::powerOff();
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void hidImplementation::begin(
    std::string deviceName,
    std::string deviceManufacturer,
    bool enableAutoPowerOff,
    uint16_t productID)
{
    if (hid == nullptr)
    {
        // Auto power-off initialization
        if (enableAutoPowerOff)
        {
            esp_timer_create_args_t args;
            args.callback = &autoPowerOffCallback;
            args.arg = nullptr;
            args.name = nullptr;
            args.dispatch_method = ESP_TIMER_TASK;
            ESP_ERROR_CHECK(esp_timer_create(&args, &autoPowerOffTimer));
        }

        // Stack initialization
        BLEDevice::init(String(deviceName.c_str()));
        // NOTE: this library lacks BLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_BOND);
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(&connectionStatus);

        // PNP hardware ID
        uint16_t custom_vid = BLE_VENDOR_ID;
        uint16_t custom_pid = (productID == 0) ? BLE_PRODUCT_ID : productID;
        hidImplementation::common::setFactoryHardwareID(custom_vid, custom_pid);
        if (productID != TEST_PRODUCT_ID)
            hidImplementation::common::loadHardwareID(custom_vid, custom_pid);

        // HID initialization
        hid = new BLEHIDDevice(pServer);
        if (!hid)
        {
            log_e("Unable to create HID device");
            abort();
        }
        hid->manufacturer()->setValue(String(deviceManufacturer.c_str())); // Workaround for bug in `hid->manufacturer(deviceManufacturer)`

        // Note: Workaround for bug in ESP-Arduino as of version 3.0.3
        uint16_t debugged_vid = std::byteswap(custom_vid);
        uint16_t debugged_pid = std::byteswap(custom_pid);

        hid->pnp(BLE_VENDOR_SOURCE, debugged_vid, debugged_pid, PRODUCT_REVISION);
        hid->hidInfo(0x00, 0x01);
        hid->reportMap((uint8_t *)hid_descriptor, sizeof(hid_descriptor));

        // Create HID reports
        inputGamepad = hid->inputReport(RID_INPUT_GAMEPAD);
        capabilitiesReport = hid->featureReport(RID_FEATURE_CAPABILITIES);
        configReport = hid->featureReport(RID_FEATURE_CONFIG);
        buttonsMapReport = hid->featureReport(RID_FEATURE_BUTTONS_MAP);
        hardwareIdReport = hid->featureReport(RID_FEATURE_HARDWARE_ID);

        if (!inputGamepad || !configReport || !capabilitiesReport || !buttonsMapReport || !hardwareIdReport)
        {
            log_e("Unable to create HID report characteristics");
            abort();
        }
        configReport->setCallbacks(&configFRCallbacks);
        capabilitiesReport->setCallbacks(&capabilitiesFRCallbacks);
        buttonsMapReport->setCallbacks(&buttonsMapFRCallbacks);
        hardwareIdReport->setCallbacks(&hardwareID_FRCallbacks);

        // Configure BLE advertising
        BLEAdvertising *pAdvertising = pServer->getAdvertising();
        pAdvertising->setAppearance(HID_GAMEPAD);
        pAdvertising->setScanResponse(true);
        pAdvertising->setMinPreferred(0x06);
        pAdvertising->setMinPreferred(0x12);
        pAdvertising->addServiceUUID(hid->hidService()->getUUID());
        pAdvertising->addServiceUUID(hid->batteryService()->getUUID());
        pAdvertising->addServiceUUID(hid->deviceInfo()->getUUID());

        // Start services
        hid->startServices();
        hid->setBatteryLevel(UNKNOWN_BATTERY_LEVEL);
        connectionStatus.onDisconnect(pServer); // start advertising
    }
}

// ----------------------------------------------------------------------------
// HID profile
// ----------------------------------------------------------------------------

void hidImplementation::reset()
{
    if (connectionStatus.connected)
    {
        uint8_t report[GAMEPAD_REPORT_SIZE];
        hidImplementation::common::onReset(report);
        inputGamepad->setValue(report, GAMEPAD_REPORT_SIZE);
        inputGamepad->notify();
    }
}

void hidImplementation::reportInput(
    inputBitmap_t inputsLow,
    inputBitmap_t inputsHigh,
    uint8_t POVstate,
    clutchValue_t leftAxis,
    clutchValue_t rightAxis,
    clutchValue_t clutchAxis)
{
    if (connectionStatus.connected)
    {
        uint8_t report[GAMEPAD_REPORT_SIZE];
        hidImplementation::common::onReportInput(
            report,
            notifyConfigChanges,
            inputsLow,
            inputsHigh,
            POVstate,
            leftAxis,
            rightAxis,
            clutchAxis);
        inputGamepad->setValue(report, GAMEPAD_REPORT_SIZE);
        inputGamepad->setNotifyProperty(true);
        inputGamepad->notify(true);
    }
}

void hidImplementation::reportBatteryLevel(int level)
{
    if (level > 100)
        level = 100;
    else if (level < 0)
        level = 0;

    hid->setBatteryLevel(level);
    // hid->batteryLevel()->notify(); not needed
}

void hidImplementation::reportChangeInConfig()
{
    notifyConfigChanges = true; // Will be reported in next input report
}

// ----------------------------------------------------------------------------
// Status
// ----------------------------------------------------------------------------

bool hidImplementation::isConnected()
{
    return connectionStatus.connected;
}
