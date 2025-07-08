/**
 * @file hid_ESPBLE.cpp
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

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEHIDDevice.h>
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#include "sdkconfig.h"
#include "esp_gap_ble_api.h"

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "HID_definitions.hpp"
#include "esp_mac.h" // For esp_efuse_mac_get_default()
// #include <Arduino.h> // For debugging

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

// Related to auto power off
static esp_timer_handle_t autoPowerOffTimer = nullptr;

// Related to HID device
class BLEHIDDeviceFix;
static BLEHIDDeviceFix *hidDevice = nullptr;
static BLECharacteristic *inputGamePad = nullptr;
static BLEServer *pServer = nullptr;
static bool notifyConfigChanges = false;
static constexpr uint16_t serialNumberChrUuid = BLE_SERIAL_NUMBER_CHR_UUID;

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
#if defined(CONFIG_IDF_TARGET_ESP32S3)
    esp_err_t rc = esp_ble_gap_set_preferred_default_phy(txPhyMask, rxPhyMask);
    return rc == ESP_OK;
#else
    return true;
#endif
}

// ----------------------------------------------------------------------------
// Utility
// ----------------------------------------------------------------------------

uint16_t byteswap(uint16_t value)
{
    return (value >> 8) | (value << 8);
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
        internals::hid::reset();
        connected = true;
        OnConnected::notify();
    };

    void onDisconnect(BLEServer *pServer) override
    {
        connected = false;
        BLEDevice::startAdvertising();
        OnDisconnected::notify();
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
    static void attachTo(BLEHIDDeviceFix *hidDevice, uint8_t RID, uint16_t size);

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
void FeatureReport::attachTo(BLEHIDDeviceFix *hidDevice, uint8_t RID, uint16_t size)
{
    BLECharacteristic *reportCharacteristic = hidDevice->featureReport(RID);
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
    size_t size = pCharacteristic->getValue().length();
    const uint8_t *data = pCharacteristic->getData();
    internals::hid::common::onOutput(_reportID, data, size);
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

// ----------------------------------------------------------------------------
// Auto power-off
// ----------------------------------------------------------------------------

void autoPowerOffCallback(void *unused)
{
    PowerService::call::shutdown();
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void internals::hid::begin(
    std::string deviceName,
    std::string deviceManufacturer,
    bool enableAutoPowerOff,
    uint16_t vendorID,
    uint16_t productID)
{
    if (hidDevice == nullptr)
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

        // HID initialization
        hidDevice = new BLEHIDDeviceFix(pServer);
        if (!hidDevice)
            throw std::runtime_error("Unable to create HID device");

        hidDevice->manufacturer()->setValue(String(deviceManufacturer.c_str())); // Workaround for bug in `hidDevice->manufacturer(deviceManufacturer)`

        // Note: Workaround for bug in ESP-Arduino as of version 3.0.3
        uint16_t debugged_vid = byteswap(vendorID);
        uint16_t debugged_pid = byteswap(productID);

        hidDevice->pnp(BLE_VENDOR_SOURCE, debugged_vid, debugged_pid, PRODUCT_REVISION);
        hidDevice->hidInfo(0x00, 0x01);
        hidDevice->reportMap((uint8_t *)hid_descriptor, sizeof(hid_descriptor));

        // Add the serial number to the "Device Information" service
        uint64_t serialNumber;
        if (esp_efuse_mac_get_default((uint8_t *)(&serialNumber)) == ESP_OK)
        {
            BLEService *deviceInfo = hidDevice->deviceInfo();
            if (deviceInfo)
            {
                BLECharacteristic *serialNumChr = deviceInfo->getCharacteristic(serialNumberChrUuid);
                if (!serialNumChr)
                    serialNumChr =
                        deviceInfo->createCharacteristic(serialNumberChrUuid, BLECharacteristic::PROPERTY_READ);
                if (serialNumChr)
                {
                    char serialAsStr[9];
                    memset(serialAsStr, 0, 9);
                    snprintf(serialAsStr, 9, "%08llX", serialNumber);
                    serialNumChr->setValue(serialAsStr);
                }
            }
        }

        // Create HID reports
        inputGamePad = hidDevice->inputReport(RID_INPUT_GAMEPAD);
        FeatureReport::attachTo(hidDevice, RID_FEATURE_CAPABILITIES, CAPABILITIES_REPORT_SIZE);
        FeatureReport::attachTo(hidDevice, RID_FEATURE_CONFIG, CONFIG_REPORT_SIZE);
        FeatureReport::attachTo(hidDevice, RID_FEATURE_BUTTONS_MAP, BUTTONS_MAP_REPORT_SIZE);
        FeatureReport::attachTo(hidDevice, RID_FEATURE_HARDWARE_ID, HARDWARE_ID_REPORT_SIZE);
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
        hidDevice->setBatteryLevel(UNKNOWN_BATTERY_LEVEL);
        connectionStatus.onDisconnect(pServer); // start advertising
    }
}

// ----------------------------------------------------------------------------
// HID profile
// ----------------------------------------------------------------------------

void internals::hid::reset()
{
    if (connectionStatus.connected)
    {
        uint8_t report[GAMEPAD_REPORT_SIZE];
        internals::hid::common::onReset(report);
        inputGamePad->setValue(report, GAMEPAD_REPORT_SIZE);
        inputGamePad->notify();
    }
}

void internals::hid::reportInput(
    uint64_t inputsLow,
    uint64_t inputsHigh,
    uint8_t POVstate,
    uint8_t leftAxis,
    uint8_t rightAxis,
    uint8_t clutchAxis)
{
    if (connectionStatus.connected)
    {
        uint8_t report[GAMEPAD_REPORT_SIZE];
        internals::hid::common::onReportInput(
            report,
            notifyConfigChanges,
            inputsLow,
            inputsHigh,
            POVstate,
            leftAxis,
            rightAxis,
            clutchAxis);
        inputGamePad->setValue(report, GAMEPAD_REPORT_SIZE);
        inputGamePad->notify(true);
    }
}

void internals::hid::reportBatteryLevel(int level)
{
    if (hidDevice)
    {
        if (level > 100)
            level = 100;
        else if (level < 0)
            level = 0;
        hidDevice->setBatteryLevel(level);
    }
}

void internals::hid::reportChangeInConfig()
{
    notifyConfigChanges = true; // Will be reported in next input report
}

bool internals::hid::supportsCustomHardwareID() { return true; }

// ----------------------------------------------------------------------------
// Status
// ----------------------------------------------------------------------------

bool internals::hid::isConnected()
{
    return connectionStatus.connected;
}
