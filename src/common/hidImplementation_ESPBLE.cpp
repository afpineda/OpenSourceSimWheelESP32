/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-10-31
 * @brief Implementation of a HID device through the ESP32's native BLE stack
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
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
static BLEServer *pServer = nullptr;
static bool notifyConfigChanges = false;

// ----------------------------------------------------------------------------
// BLE Server callbacks and advertising
// ----------------------------------------------------------------------------

class BleConnectionStatus : public BLEServerCallbacks
{
public:
    BleConnectionStatus(void){};
    bool connected = false;
    void onConnect(BLEServer *pServer) override
    {
        if (autoPowerOffTimer != nullptr)
            esp_timer_stop(autoPowerOffTimer);
        connected = true;
        Serial.println("BLE connected");
        notify::connected();
        hidImplementation::reset();
    };

    void onDisconnect(BLEServer *pServer) override
    {
        connected = false;
        BLEDevice::startAdvertising();
        notify::BLEdiscovering();
        Serial.println("Advertising");
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
        int size = pCharacteristic->getLength();
        const uint8_t *data = pCharacteristic->getData();
        if ((size>0) && (data[0]>=CF_CLUTCH) && (data[0]<=CF_BUTTON)) { 
            // clutch function
            clutchState::setFunction((clutchFunction_t)data[0]);
        }
        if ((size>1) && (data[1]!=0xff))  {
            // ALT Buttons mode
            clutchState::setALTModeForALTButtons((bool)data[1]);
        }
        if ((size>2) && ((clutchValue_t)data[2]>=CLUTCH_NONE_VALUE) && ((clutchValue_t)data[2]<=CLUTCH_FULL_VALUE)) {
            // Bite point
            clutchState::setBitePoint((clutchValue_t)data[2]);
        }
        if ((size>3) && (data[3]==(uint8_t)simpleCommands_t::CMD_AXIS_RECALIBRATE)) {
            // Force analog axis recalibration
            inputs::recalibrateAxes();
        }
        if ((size>3) && (data[3]==(uint8_t)simpleCommands_t::CMD_BATT_RECALIBRATE)) {
            // Restart auto calibration algoritm
            batteryCalibration::restartAutoCalibration();
        }
    }

    // SEND REQUESTED DATA
    void onRead(BLECharacteristic *pCharacteristic)
    {
        uint8_t data[CONFIG_REPORT_SIZE];
        data[0] = (uint8_t)clutchState::currentFunction;
        data[1] = (uint8_t)clutchState::altModeForAltButtons;
        data[2] = (uint8_t)clutchState::bitePoint;
        data[3] = (uint8_t)power::getLastBatteryLevel();
        pCharacteristic->setValue(data, sizeof(data));
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
        data[0] = MAGIC_NUMBER_LOW;
        data[1] = MAGIC_NUMBER_HIGH;
        *(uint16_t *)(data+2) = DATA_MAJOR_VERSION;
        *(uint16_t *)(data+4) = DATA_MINOR_VERSION;
        *(uint16_t *)(data+6) = capabilities::flags;
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
        BLEDevice::init(deviceName);
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(&connectionStatus);

        // HID initialization
        hid = new BLEHIDDevice(pServer);
        if (!hid) {
            log_e("Unable to create HID device");
            abort();
        }
        hid->manufacturer()->setValue(deviceManufacturer); // Workaround for bug in `hid->manufacturer(deviceManufacturer)`
        hid->pnp(BLE_VENDOR_SOURCE, BLE_VENDOR_ID, BLE_PRODUCT_ID, PRODUCT_REVISION);
        hid->hidInfo(0x00, 0x01);
        BLECharacteristic* modelString_chr = hid->deviceInfo()->createCharacteristic((uint16_t)0x2A24,BLECharacteristic::PROPERTY_READ);
        modelString_chr->setValue(deviceName);
        hid->reportMap((uint8_t *)hid_descriptor, sizeof(hid_descriptor));

        // Create HID reports
        inputGamepad = hid->inputReport(RID_INPUT_GAMEPAD);
        configReport = hid->featureReport(RID_FEATURE_CONFIG);
        capabilitiesReport = hid->featureReport(RID_FEATURE_CAPABILITIES);
        if (!inputGamepad || !configReport || !capabilitiesReport) {
            log_e("Unable to create HID report characteristics");
            abort();
        }
        configReport->setCallbacks(&configFRCallbacks);
        capabilitiesReport->setCallbacks(&capabilitiesFRCallbacks);

        // Configure BLE advertising
        BLEAdvertising *pAdvertising = pServer->getAdvertising();
        pAdvertising->setAppearance(HID_GAMEPAD);
        pAdvertising->setScanResponse(true);
        pAdvertising->setMinPreferred(0x06);
        pAdvertising->setMinPreferred(0x12);
        pAdvertising->addServiceUUID(hid->hidService()->getUUID());
        pAdvertising->addServiceUUID(hid->batteryService()->getUUID());
        pAdvertising->addServiceUUID(hid->deviceInfo()->getUUID());

        // Configure BLE security
        //BLESecurity *pSecurity = new BLESecurity();
        //pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

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
        inputGamepad->setValue(report, GAMEPAD_REPORT_SIZE);
        inputGamepad->notify();
    }
}

void hidImplementation::reportInput(
        inputBitmap_t globalState,
        bool altEnabled,
        uint8_t POVstate = 0)
{
    if (connectionStatus.connected)
    {
        Serial.print("reportInput (connected)");
        uint8_t report[GAMEPAD_REPORT_SIZE];
        if (altEnabled)
        {
            report[0] = 0;
            report[1] = 0;
            report[2] = 0;
            report[3] = 0;
            report[4] = 0;
            report[5] = 0;
            report[6] = 0;
            report[7] = 0;
            report[8] = ((uint8_t *)&globalState)[0];
            report[9] = ((uint8_t *)&globalState)[1];
            report[10] = ((uint8_t *)&globalState)[2];
            report[11] = ((uint8_t *)&globalState)[3];
            report[12] = ((uint8_t *)&globalState)[4];
            report[13] = ((uint8_t *)&globalState)[5];
            report[14] = ((uint8_t *)&globalState)[6];
            report[15] = ((uint8_t *)&globalState)[7];
        }
        else
        {
            report[0] = ((uint8_t *)&globalState)[0];
            report[1] = ((uint8_t *)&globalState)[1];
            report[2] = ((uint8_t *)&globalState)[2];
            report[3] = ((uint8_t *)&globalState)[3];
            report[4] = ((uint8_t *)&globalState)[4];
            report[5] = ((uint8_t *)&globalState)[5];
            report[6] = ((uint8_t *)&globalState)[6];
            report[7] = ((uint8_t *)&globalState)[7];
            report[8] = 0;
            report[9] = 0;
            report[10] = 0;
            report[11] = 0;
            report[12] = 0;
            report[13] = 0;
            report[14] = 0;
            report[15] = 0;
        }
        report[16] = (uint8_t)clutchState::combinedAxis;
        report[17] = (uint8_t)clutchState::leftAxis;
        report[18] = (uint8_t)clutchState::rightAxis;
        report[19] = POVstate;
        if (notifyConfigChanges) {
            report[19] |= (RID_FEATURE_CONFIG << 4);
            notifyConfigChanges = false;
        }
        inputGamepad->setValue(report, GAMEPAD_REPORT_SIZE);
        inputGamepad->notify();
    }
    else 
    Serial.print("reportInput (NOT connected)");
}

void hidImplementation::reportBatteryLevel(int level)
{
    if (level > 100)
        level = 100;
    else if (level < 0)
        level = 0;

    hid->setBatteryLevel(level);
    //hid->batteryLevel()->notify();
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
