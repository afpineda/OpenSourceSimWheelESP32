/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Implementation of a HID device through the NimBLE stack
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

// Implementation heavely inspired by
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
//#include <Arduino.h>

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

// Related to auto power off
static esp_timer_handle_t autoPowerOffTimer = nullptr;

// Related to Nordic UART Service (NuS)
//
#define UART_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
NimBLEService *uartService = nullptr;
NimBLECharacteristic *uartTXCharacteristic = nullptr;

// Related to HID device
static NimBLEHIDDevice *hid = nullptr;
static NimBLECharacteristic *inputGamepad = nullptr;
static NimBLECharacteristic *configReport = nullptr;
static NimBLECharacteristic *capabilitiesReport = nullptr;
static NimBLEServer *pServer = nullptr;

// ----------------------------------------------------------------------------
// BLE Server callbacks and advertising
// ----------------------------------------------------------------------------

void bleAdvertise()
{
    if (pServer != nullptr)
    {
        NimBLEAdvertising *pAdvertising = pServer->getAdvertising();
        pAdvertising->setAppearance(HID_GAMEPAD);
        pAdvertising->addServiceUUID(hid->hidService()->getUUID());
        if (uartService)
            pAdvertising->addServiceUUID(UART_SERVICE_UUID);
        pAdvertising->start();
    }
}

class BleConnectionStatus : public NimBLEServerCallbacks
{
public:
    BleConnectionStatus(void){};
    bool connected = false;
    void onConnect(NimBLEServer *pServer)
    {
        if (autoPowerOffTimer != nullptr)
            esp_timer_stop(autoPowerOffTimer);
        connected = true;
        ui::showConnectedNotice();
        hidImplementation::reset();
    };
    void onDisconnect(NimBLEServer *pServer)
    {
        connected = false;
        bleAdvertise();
        if (autoPowerOffTimer != nullptr)
            esp_timer_start_once(autoPowerOffTimer, AUTO_POWER_OFF_DELAY_SECS * 1000000);
        ui::showBLEDiscoveringNotice();
    };
} connectionStatus;

// ----------------------------------------------------------------------------
// UART Callbacks
// ----------------------------------------------------------------------------

class UARTCallbacks : public NimBLECharacteristicCallbacks
{
    // RECEIVE DATA
    void onWrite(NimBLECharacteristic *pCharacteristic)
    {
        uartServer::onReceive((char *)pCharacteristic->getValue().c_str());
        // Serial.print(pCharacteristic->getUUID().toString().c_str());
        // Serial.print(": onWrite(), value: ");
        // Serial.println(pCharacteristic->getValue().c_str());
    };

} uartHandler;

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
        if ((size>0) && (data[0]>=CF_CLUTCH) && (data[0]<=CF_BUTTON)) { 
            // clutch function
            inputHub::setClutchFunction((clutchFunction_t)data[0],true);
        }
        if ((size>1) && (data[1]!=0x80)  {
            // ALT Buttons mode
            inputHub::setALTFunction((bool)data[1],true);
        }
        if ((size>2) && ((clutchValue_t)data[2]>=CLUTCH_NONE_VALUE) && ((clutchValue_t)data[2]<=CLUTCH_FULL_VALUE)) {
            // Bite point
            inputHub::setClutchBitePoint((clutchValue_t)data[2],true);
        }
    };

    // SEND REQUESTED DATA
    void onRead(NimBLECharacteristic *pCharacteristic)
    {
        uint8_t data[3];
        data[0] = (uint8_t)inputHub::getClutchFunction();
        data[1] = (uint8_t)inputHub::getALTFunction();
        data[2] = (uint8_t)inputHub::getClutchBitePoint();
        pCharacteristic->setValue(data, sizeof(data));
    }
} configFRCallbacks;

class CapabilitiesFRCallbacks : public NimBLECharacteristicCallbacks
{
    // RECEIVE DATA
    void onWrite(NimBLECharacteristic *pCharacteristic)
    {
        // IGNORED: Data is read-only
    };

    // SEND REQUESTED DATA
    void onRead(NimBLECharacteristic *pCharacteristic)
    {
        uint8_t data[4];
        data[0] = 0;
        data[1] = 0;
        data[2] = 0;
        data[3] = 0;
        if (inputHub::hasClutchPaddles())
            data[0] = data[0] | 0x01;
        if (inputHub::hasALTButtons())
            data[0] = data[0] | 0x02;
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
    bool enableAutoPowerOff,
    bool enableUART)
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
        pServer = NimBLEDevice::createServer();
        pServer->setCallbacks(&connectionStatus);

        // HID initialization
        hid = new NimBLEHIDDevice(pServer);
        hid->manufacturer()->setValue(deviceManufacturer);
        hid->pnp(0x00, OPEN_SOURCE_VENDOR_ID, PROJECT_PRODUCT_ID, PRODUCT_REVISION);
        hid->hidInfo(0x00, 0x01);
        NimBLESecurity *pSecurity = new NimBLESecurity();
        pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);
        hid->reportMap((uint8_t *)hid_descriptor, sizeof(hid_descriptor));
        hid->setBatteryLevel(100);

        inputGamepad = hid->inputReport(RID_INPUT_GAMEPAD);
        configReport = hid->featureReport(RID_FEATURE_CONFIG);
        capabilitiesReport = hid->featureReport(RID_FEATURE_CAPABILITIES);
        if (!inputGamepad || !configReport || !capabilitiesReport) {
            log_e("Unable to create HID report characteristics");
            abort();
        }
        configReport->setCallbacks(&configFRCallbacks);
        capabilitiesReport->setCallbacks(&capabilitiesFRCallbacks);

        if (enableUART)
        {
            // UART Service initialization
            uartService = pServer->createService(UART_SERVICE_UUID);
            if (uartService)
            {
                uartTXCharacteristic = uartService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    NIMBLE_PROPERTY::NOTIFY);
                NimBLECharacteristic *rx = uartService->createCharacteristic(
                    CHARACTERISTIC_UUID_RX,
                    NIMBLE_PROPERTY::WRITE);
                rx->setCallbacks(&uartHandler);
            } else {
                log_e("Error while creating UART service");
            }
        }

        // Start services
        hid->startServices();
        if (uartService)
            uartService->start();
        connectionStatus.onDisconnect(pServer);
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
        report[17] = 0;
        inputGamepad->setValue((const uint8_t *)report, GAMEPAD_REPORT_SIZE);
        inputGamepad->notify();
    }
}

void hidImplementation::reportInput(
    inputBitmap_t globalState,
    bool altEnabled,
    clutchValue_t clutchValue,
    uint8_t POVstate)
{
    if (connectionStatus.connected)
    {
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
        report[16] = (uint8_t)clutchValue;
        report[17] = POVstate;
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

    if ((hid != nullptr) && (connectionStatus.connected))
        hid->setBatteryLevel(level);
}

// ----------------------------------------------------------------------------
// Status
// ----------------------------------------------------------------------------

bool hidImplementation::isConnected()
{
    return connectionStatus.connected;
}

// ----------------------------------------------------------------------------
// UART
// ----------------------------------------------------------------------------

bool hidImplementation::uartSendText(char *text)
{
    if (uartTXCharacteristic != nullptr)
    {
        uartTXCharacteristic->setValue((uint8_t *)text, strlen(text));
        uartTXCharacteristic->notify();
        return true;
    }
    return false;
}
