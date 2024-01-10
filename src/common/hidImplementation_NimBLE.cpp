/**
 * @file hidImplementation_NimBLE.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Implementation of a HID device through the NimBLE stack
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
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
        int size = pCharacteristic->getValue().length();
        const uint8_t *data = pCharacteristic->getValue().data();
        if ((size > 0) && (data[0] >= CF_CLUTCH) && (data[0] <= CF_BUTTON))
        {
            // clutch function
            userSettings::setCPWorkingMode((clutchFunction_t)data[0]);
        }
        if ((size > 1) && (data[1] != 0xff))
        {
            // ALT Buttons mode
            userSettings::setALTButtonsWorkingMode((bool)data[1]);
        }
        if ((size > 2) && ((clutchValue_t)data[2] >= CLUTCH_NONE_VALUE) && ((clutchValue_t)data[2] <= CLUTCH_FULL_VALUE))
        {
            // Bite point
            userSettings::setBitePoint((clutchValue_t)data[2]);
        }
        if ((size > 3) && (data[3] == (uint8_t)simpleCommands_t::CMD_AXIS_RECALIBRATE))
        {
            // Force analog axis recalibration
            inputs::recalibrateAxes();
        }
        if ((size > 3) && (data[3] == (uint8_t)simpleCommands_t::CMD_BATT_RECALIBRATE))
        {
            // Restart auto calibration algoritm
            batteryCalibration::restartAutoCalibration();
        }
    }

    // SEND REQUESTED DATA
    void onRead(NimBLECharacteristic *pCharacteristic)
    {
        uint8_t data[CONFIG_REPORT_SIZE];
        data[0] = (uint8_t)userSettings::cpWorkingMode;
        data[1] = (uint8_t)userSettings::altButtonsWorkingMode;
        data[2] = (uint8_t)userSettings::bitePoint;
        data[3] = (uint8_t)power::getLastBatteryLevel();
        pCharacteristic->setValue(data, sizeof(data));
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
        data[0] = MAGIC_NUMBER_LOW;
        data[1] = MAGIC_NUMBER_HIGH;
        *(uint16_t *)(data + 2) = DATA_MAJOR_VERSION;
        *(uint16_t *)(data + 4) = DATA_MINOR_VERSION;
        *(uint16_t *)(data + 6) = capabilities::flags;
        pCharacteristic->setValue(data, sizeof(data));
    }
} capabilitiesFRCallbacks;

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
        if (!inputGamepad || !configReport || !capabilitiesReport)
        {
            log_e("Unable to create HID report characteristics");
            abort();
        }
        configReport->setCallbacks(&configFRCallbacks);
        capabilitiesReport->setCallbacks(&capabilitiesFRCallbacks);

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
        report[0] = 0;
        report[1] = 0;
        report[2] = 0;
        report[3] = 0;
        report[4] = 0;
        report[5] = 0;
        report[6] = 0;
        report[7] = 0;
        report[8] = 0;
        report[9] = 0;
        report[10] = 0;
        report[11] = 0;
        report[12] = 0;
        report[13] = 0;
        report[14] = 0;
        report[15] = 0;
        report[16] = CLUTCH_NONE_VALUE;
        report[17] = CLUTCH_NONE_VALUE;
        report[18] = CLUTCH_NONE_VALUE;
        report[19] = 0;
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
        report[0] = ((uint8_t *)&inputsLow)[0];
        report[1] = ((uint8_t *)&inputsLow)[1];
        report[2] = ((uint8_t *)&inputsLow)[2];
        report[3] = ((uint8_t *)&inputsLow)[3];
        report[4] = ((uint8_t *)&inputsLow)[4];
        report[5] = ((uint8_t *)&inputsLow)[5];
        report[6] = ((uint8_t *)&inputsLow)[6];
        report[7] = ((uint8_t *)&inputsLow)[7];
        report[8] = ((uint8_t *)&inputsHigh)[0];
        report[9] = ((uint8_t *)&inputsHigh)[1];
        report[10] = ((uint8_t *)&inputsHigh)[2];
        report[11] = ((uint8_t *)&inputsHigh)[3];
        report[12] = ((uint8_t *)&inputsHigh)[4];
        report[13] = ((uint8_t *)&inputsHigh)[5];
        report[14] = ((uint8_t *)&inputsHigh)[6];
        report[15] = ((uint8_t *)&inputsHigh)[7];
        report[16] = (uint8_t)clutchAxis;
        report[17] = (uint8_t)leftAxis;
        report[18] = (uint8_t)rightAxis;
        report[19] = POVstate;
        if (notifyConfigChanges)
        {
            report[19] |= (RID_FEATURE_CONFIG << 4);
            notifyConfigChanges = false;
        }
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
