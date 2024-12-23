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
#include "esp_gap_ble_api.h"

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
class BLEHIDDeviceFix;
static BLEHIDDeviceFix *hid = nullptr;
static BLECharacteristic *inputGamepad = nullptr;
static BLEServer *pServer = nullptr;
static bool notifyConfigChanges = false;

// ----------------------------------------------------------------------------
// BLEHIDDeviceFix
//
// This subclass is a workaround for a bug in Arduino-ESP32
// For unknown reasons,
// notifications get randomly disabled in the input report.
// ----------------------------------------------------------------------------

class BLEHIDDeviceFix : public BLEHIDDevice
{
public:
    BLECharacteristic *inputReport(uint8_t reportID)
    {
        BLECharacteristic *inputReportCharacteristic =
            hidService()->createCharacteristic((uint16_t)0x2a4d, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
        BLEDescriptor *inputReportDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2908));
        BLE2902 *p2902 = new BLE2902();
        inputReportCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
        inputReportDescriptor->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
        p2902->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
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

// ----------------------------------------------------------------------------
// PHY configuration
// ----------------------------------------------------------------------------

bool setDefaultPhy(
    esp_ble_gap_prefer_phy_options_t txPhyMask,
    esp_ble_gap_prefer_phy_options_t rxPhyMask)
{
    esp_err_t rc = esp_ble_gap_set_preferred_default_phy(txPhyMask, rxPhyMask);
    return rc == ESP_OK;
}

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

class FeatureReport : public BLECharacteristicCallbacks
{
public:
    void onWrite(BLECharacteristic *pCharacteristic) override;
    void onRead(BLECharacteristic *pCharacteristic) override;
    FeatureReport(uint8_t RID, uint16_t size);
    static void attachTo(BLEHIDDeviceFix *hid, uint8_t RID, uint16_t size);

private:
    uint8_t _reportID;
    uint16_t _reportSize;
};

// RECEIVE DATA
void FeatureReport::onWrite(BLECharacteristic *pCharacteristic)
{
    size_t size = pCharacteristic->getValue().length();
    const uint8_t *data = pCharacteristic->getData();
    hidImplementation::common::onSetFeature(_reportID, data, size);
}

// SEND REQUESTED DATA
void FeatureReport::onRead(BLECharacteristic *pCharacteristic)
{
    uint8_t data[_reportSize];
    hidImplementation::common::onGetFeature(_reportID, data, _reportSize);
    pCharacteristic->setValue(data, _reportSize);
}

// Constructor
FeatureReport::FeatureReport(uint8_t RID, uint16_t size)
{
    _reportID = RID;
    _reportSize = size;
}

// Attach to HID device
void FeatureReport::attachTo(BLEHIDDeviceFix *hid, uint8_t RID, uint16_t size)
{
    BLECharacteristic *reportCharacteristic = hid->featureReport(RID);
    if (!reportCharacteristic)
    {
        log_e("Unable to create HID report characteristics");
        abort();
    }
    reportCharacteristic->setCallbacks(new FeatureReport(RID, size));
}

// ----------------------------------------------------------------------------
// HID OUTPUT REPORT callbacks
// ----------------------------------------------------------------------------

class OutputReport : public BLECharacteristicCallbacks
{
public:
    void onWrite(BLECharacteristic *pCharacteristic) override;
    OutputReport(uint8_t RID);
    static void attachTo(BLEHIDDeviceFix *hid, uint8_t RID);

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
    size_t size = pCharacteristic->getValue().length();
    const uint8_t *data = pCharacteristic->getData();
    hidImplementation::common::onOutput(_reportID, data, size);
}

// Attach to HID device
void OutputReport::attachTo(BLEHIDDeviceFix *hid, uint8_t RID)
{
    BLECharacteristic *reportCharacteristic = hid->outputReport(RID);
    if (!reportCharacteristic)
    {
        log_e("Unable to create HID report characteristics");
        abort();
    }
    reportCharacteristic->setCallbacks(new OutputReport(RID));
}

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
        BLEDevice::setMTU(BLE_MTU_SIZE);
        setDefaultPhy(ESP_BLE_GAP_PHY_2M_PREF_MASK, ESP_BLE_GAP_PHY_2M_PREF_MASK);
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(&connectionStatus);
        BLESecurity *pSecurity = new BLESecurity();
        pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
        pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

        // PNP hardware ID
        uint16_t custom_vid = BLE_VENDOR_ID;
        uint16_t custom_pid = (productID == 0) ? BLE_PRODUCT_ID : productID;
        hidImplementation::common::setFactoryHardwareID(custom_vid, custom_pid);
        if (productID != TEST_PRODUCT_ID)
            hidImplementation::common::loadHardwareID(custom_vid, custom_pid);

        // HID initialization
        hid = new BLEHIDDeviceFix(pServer);
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
        FeatureReport::attachTo(hid, RID_FEATURE_CAPABILITIES, CAPABILITIES_REPORT_SIZE);
        FeatureReport::attachTo(hid, RID_FEATURE_CONFIG, CONFIG_REPORT_SIZE);
        FeatureReport::attachTo(hid, RID_FEATURE_BUTTONS_MAP, BUTTONS_MAP_REPORT_SIZE);
        FeatureReport::attachTo(hid, RID_FEATURE_HARDWARE_ID, HARDWARE_ID_REPORT_SIZE);
        OutputReport::attachTo(hid, RID_OUTPUT_POWERTRAIN);
        OutputReport::attachTo(hid, RID_OUTPUT_ECU);
        OutputReport::attachTo(hid, RID_OUTPUT_RACE_CONTROL);
        OutputReport::attachTo(hid, RID_OUTPUT_GAUGES);
        OutputReport::attachTo(hid, RID_OUTPUT_PIXEL);

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
