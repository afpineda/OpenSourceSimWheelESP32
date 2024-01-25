/**
 * @file hidImplementation_NimBLE.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Implementation of a HID device through the NimBLE stack
 *
 * @copyright Licensed under the EUPL
 *
 */

// Implementation heavily inspired by
// https://github.com/lemmingDev/ESP32-BLE-Gamepad
//
// Use this app for testing:
// http://www.planetpointy.co.uk/joystick-test-application/

#include <NimBLEDevice.h>
#include <NimBLEUtils.h>
#include <NimBLEServer.h>
#include "NimBLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#include "nimconfig.h"
#include "sdkconfig.h"
#include <NimBLEServer.h>
#include "NimBLECharacteristic.h"

#include "SimWheel.h"
#include "HID_definitions.h"
// #include <Arduino.h> // For debug

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

// Related to auto power off
static esp_timer_handle_t autoPowerOffTimer = nullptr;

// Related to HID device
static NimBLEHIDDevice *hid = nullptr;
static NimBLECharacteristic *inputGamepad = nullptr;
static NimBLECharacteristic *configReport = nullptr;
static NimBLECharacteristic *capabilitiesReport = nullptr;
static NimBLECharacteristic *buttonsMapReport = nullptr;
static NimBLEServer *pServer = nullptr;
static bool notifyConfigChanges = false;

// ----------------------------------------------------------------------------
// BLE Server callbacks and advertising
// ----------------------------------------------------------------------------

class BleConnectionStatus : public NimBLEServerCallbacks
{
public:
    BleConnectionStatus(void){};
    bool connected = false;
    void onConnect(NimBLEServer *pServer)
    {
        if (autoPowerOffTimer != nullptr)
            esp_timer_stop(autoPowerOffTimer);
        hidImplementation::reset();
        connected = true;
        notify::connected();
    };

    void onDisconnect(NimBLEServer *pServer)
    {
        connected = false;
        NimBLEDevice::startAdvertising();
        notify::BLEdiscovering();
        if (autoPowerOffTimer != nullptr)
            esp_timer_start_once(autoPowerOffTimer, AUTO_POWER_OFF_DELAY_SECS * 1000000);
    };

} connectionStatus;

// ----------------------------------------------------------------------------
// HID FEATURE REQUEST callbacks
// ----------------------------------------------------------------------------

class ConfigFRCallbacks : public NimBLECharacteristicCallbacks
{
    // RECEIVE DATA
    void onWrite(NimBLECharacteristic *pCharacteristic)
    {
        size_t size = pCharacteristic->getValue().length();
        const uint8_t *data = pCharacteristic->getValue().data();
        hidImplementation::common::onSetFeature(RID_FEATURE_CONFIG, data, size);
    }

    // SEND REQUESTED DATA
    void onRead(NimBLECharacteristic *pCharacteristic)
    {
        uint8_t data[CONFIG_REPORT_SIZE];
        hidImplementation::common::onGetFeature(RID_FEATURE_CONFIG, data, CONFIG_REPORT_SIZE);
        pCharacteristic->setValue(data, CONFIG_REPORT_SIZE);
    }

} configFRCallbacks;

// ----------------------------------------------------------------------------

class CapabilitiesFRCallbacks : public NimBLECharacteristicCallbacks
{
    // RECEIVED DATA is ignored (read-only)

    // SEND REQUESTED DATA
    void onRead(NimBLECharacteristic *pCharacteristic)
    {
        uint8_t data[CAPABILITIES_REPORT_SIZE];
        hidImplementation::common::onGetFeature(RID_FEATURE_CAPABILITIES, data, CAPABILITIES_REPORT_SIZE);
        pCharacteristic->setValue(data, sizeof(data));
    }
} capabilitiesFRCallbacks;

// ----------------------------------------------------------------------------

class ButtonsMapFRCallbacks : public NimBLECharacteristicCallbacks
{
    // RECEIVED DATA:
    // Select firmware-defined input number or set button map
    void onWrite(NimBLECharacteristic *pCharacteristic)
    {
        size_t size = pCharacteristic->getValue().length();
        const uint8_t *data = pCharacteristic->getValue().data();
        hidImplementation::common::onSetFeature(RID_FEATURE_BUTTONS_MAP, data, size);
    }

    // SEND DATA:
    // Map for selected button
    void onRead(NimBLECharacteristic *pCharacteristic)
    {
        uint8_t data[BUTTONS_MAP_REPORT_SIZE];
        hidImplementation::common::onGetFeature(RID_FEATURE_BUTTONS_MAP, data, BUTTONS_MAP_REPORT_SIZE);
        pCharacteristic->setValue(data, sizeof(data));
    }

} buttonsMapFRCallbacks;

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
    bool enableAutoPowerOff)
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
        NimBLEDevice::init(deviceName);
        NimBLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_BOND);
        pServer = NimBLEDevice::createServer();
        pServer->setCallbacks(&connectionStatus);

        // HID initialization
        hid = new NimBLEHIDDevice(pServer);
        if (!hid)
        {
            log_e("Unable to create HID device");
            abort();
        }
        hid->manufacturer()->setValue(deviceManufacturer); // Workaround for bug in `hid->manufacturer(deviceManufacturer)`
        hid->pnp(BLE_VENDOR_SOURCE, BLE_VENDOR_ID, BLE_PRODUCT_ID, PRODUCT_REVISION);
        hid->hidInfo(0x00, 0x01);
        hid->reportMap((uint8_t *)hid_descriptor, sizeof(hid_descriptor));

        // Create HID reports
        inputGamepad = hid->inputReport(RID_INPUT_GAMEPAD);
        configReport = hid->featureReport(RID_FEATURE_CONFIG);
        capabilitiesReport = hid->featureReport(RID_FEATURE_CAPABILITIES);
        buttonsMapReport = hid->featureReport(RID_FEATURE_BUTTONS_MAP);
        if (!inputGamepad || !configReport || !capabilitiesReport || !buttonsMapReport)
        {
            log_e("Unable to create HID report characteristics");
            abort();
        }
        configReport->setCallbacks(&configFRCallbacks);
        capabilitiesReport->setCallbacks(&capabilitiesFRCallbacks);
        buttonsMapReport->setCallbacks(&buttonsMapFRCallbacks);

        // Configure BLE advertising
        NimBLEAdvertising *pAdvertising = pServer->getAdvertising();
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
        inputGamepad->setValue((const uint8_t *)report, GAMEPAD_REPORT_SIZE);
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
        inputGamepad->setValue((const uint8_t *)report, GAMEPAD_REPORT_SIZE);
        inputGamepad->notify();
    }
}

void hidImplementation::reportBatteryLevel(int level)
{
    if (level > 100)
        level = 100;
    else if (level < 0)
        level = 0;

    hid->setBatteryLevel(level);
    hid->batteryLevel()->notify();
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
