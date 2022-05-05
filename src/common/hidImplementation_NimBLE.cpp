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

// Related to HID profile
//
#define HID_REPORT_ID 3
#define CONTROLLER_TYPE_GAMEPAD 0x05
#define CONTROLLER_TYPE_JOYSTICK 0x04
#define BUTTON_COUNT 128
//#define CUSTOM_REPORT_SIZE 17 // For old descriptor
#define CUSTOM_REPORT_SIZE 18

static NimBLEHIDDevice *hid = nullptr;
static NimBLECharacteristic *inputGamepad = nullptr;
static NimBLEServer *pServer = nullptr;

// HID DESCRIPTOR
/* first working descriptor
static const uint8_t hid_descriptor[] = {
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x15, 0x00, // LOGICAL_MINIMUM (0)
    0x09, CONTROLLER_TYPE_GAMEPAD,
    //    0x09, CONTROLLER_TYPE_JOYSTICK, // USAGE (Joystick)
    0xa1, 0x01,          // COLLECTION (Application)
    0x85, HID_REPORT_ID, // REPORT ID
    0x05, 0x09,          //   USAGE_PAGE (Button)
    0x19, 0x01,          //   USAGE_MINIMUM (Button 1)
    0x29, BUTTON_COUNT,  //   USAGE_MAXIMUM
    0x15, 0x00,          //   LOGICAL_MINIMUM (0)
    0x25, 0x01,          //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,          //   REPORT_SIZE (1)
    0x95, BUTTON_COUNT,  //   REPORT_COUNT
    0x55, 0x00,          //   UNIT_EXPONENT (0)
    0x65, 0x00,          //   UNIT (None)
    0x81, 0x02,          //   INPUT (Data,Var,Abs)
    0x05, 0x02,          //   USAGE_PAGE (Simulation Controls)
    0x15, 0x81,          // LOGICAL_MINIMUM (-127)
    0x25, 0x7F,          // LOGICAL MAXIMUM (127)
    0x75, 0x08,          // REPORT_SIZE (8)
    0x95, 0x01,          // REPORT_COUNT (simulationCount) = 1
    0xA1, 0x00,          // COLLECTION (Physical)
    0x09, 0xBB,          // USAGE (Throttle)
    0x81, 0x02,          // INPUT (Data,Var,Abs)
    0xc0,                // END_COLLECTION (Physical)
    0xc0                 // END_COLLECTION (Application)
};
*/

static const uint8_t hid_descriptor[] = {
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x15, 0x00, // LOGICAL_MINIMUM (0)
    0x09, CONTROLLER_TYPE_GAMEPAD,
    //    0x09, CONTROLLER_TYPE_JOYSTICK, // USAGE (Joystick)
    0xa1, 0x01,          // COLLECTION (Application)
    0x85, HID_REPORT_ID, // REPORT ID

    // __ Buttons __ (16 bytes=128 bits)
    0x05, 0x09,         //   USAGE_PAGE (Button)
    0x19, 0x01,         //   USAGE_MINIMUM (Button 1)
    0x29, BUTTON_COUNT, //   USAGE_MAXIMUM
    0x15, 0x00,         //   LOGICAL_MINIMUM (0)
    0x25, 0x01,         //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,         //   REPORT_SIZE (1)
    0x95, BUTTON_COUNT, //   REPORT_COUNT
    0x55, 0x00,         //   UNIT_EXPONENT (0)
    0x65, 0x00,         //   UNIT (None)
    0x81, 0x02,         //   INPUT (Data,Var,Abs)

    // __ Clutch __ (1 byte)
    0x05, 0x02, //   USAGE_PAGE (Simulation Controls)
    0x15, 0x81, // LOGICAL_MINIMUM (-127)
    0x25, 0x7F, // LOGICAL MAXIMUM (127)
    0x75, 0x08, // REPORT_SIZE (8)
    0x95, 0x01, // REPORT_COUNT (1)
    0xA1, 0x00, // COLLECTION (Physical)
    // 0x02, 0xC6, // USAGE (Clutch)
    0x09, 0xBB, // USAGE (Throttle)
    0x81, 0x02, // INPUT (Data,Var,Abs)
    0xc0,       // END_COLLECTION (Physical)

    // __ Hat swith (DPAD) __ (1 byte)
    0xA1, 0x00,       // COLLECTION (Physical)
    0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
    0x09, 0x39,       // USAGE (Hat Switch)
    0x15, 0x01,       // Logical Min (1)
    0x25, 0x08,       // Logical Max (8)
    0x35, 0x00,       // Physical Min (0)
    0x46, 0x3B, 0x01, // Physical Max (315)
    0x65, 0x12,       // Unit (SI Rot : Ang Pos)
    0x75, 0x08,       // Report Size (8)
    0x95, 0x01,       // Report count (1)
    0x81, 0x42,       // Input (Data, Variable, Absolute)
    0xc0,             // END_COLLECTION (Physical)

    0xc0 // END_COLLECTION (Application)
};

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
        inputGamepad = hid->inputReport(HID_REPORT_ID);
        hid->manufacturer()->setValue(deviceManufacturer);
        hid->pnp(0x01, 0x02e5, 0xabbb, 0x0110);
        hid->hidInfo(0x00, 0x01);
        NimBLESecurity *pSecurity = new NimBLESecurity();
        pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);
        hid->reportMap((uint8_t *)hid_descriptor, sizeof(hid_descriptor));
        hid->setBatteryLevel(100);

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
        uint8_t report[CUSTOM_REPORT_SIZE];
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
        inputGamepad->setValue((const uint8_t *)report, CUSTOM_REPORT_SIZE);
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
        uint8_t report[CUSTOM_REPORT_SIZE];
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
        inputGamepad->setValue((const uint8_t *)report, CUSTOM_REPORT_SIZE);
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
