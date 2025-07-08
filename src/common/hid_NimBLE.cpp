/**
 * @file hid_NimBLE.cpp
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

// ----------------------------------------------------------------------------
// NOTE:
// Do not delete commented out code. It may be reused in the future.
// The NimBLEHIDDeviceFix class is used to implement workarounds
// for bugs in the NimBLE-Arduino library, when required.
// ----------------------------------------------------------------------------

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
// class NimBLEHIDDeviceFix;
#define NimBLEHIDDeviceFix NimBLEHIDDevice
static NimBLEHIDDeviceFix *hidDevice = nullptr;
static NimBLECharacteristic *inputGamePad = nullptr;
static NimBLEServer *pServer = nullptr;
static bool notifyConfigChanges = false;
static constexpr uint16_t serialNumberChrUuid = BLE_SERIAL_NUMBER_CHR_UUID;

// ----------------------------------------------------------------------------
// NimBLEHIDDeviceFix
//
// This subclass is a workaround for a bug in NimBLE-Arduino v2.1.0
// ----------------------------------------------------------------------------
/*
static constexpr uint16_t hidReportChrUuid = 0x2a4d;
static constexpr uint16_t hidReportChrDscUuid = 0x2908;
static constexpr uint16_t hidReport2902DscUuid = 0x2902;

class NimBLEHIDDeviceFix : public NimBLEHIDDevice
{
public:
    NimBLECharacteristic *getOutputReport(uint8_t reportId);
    NimBLECharacteristic *getFeatureReport(uint8_t reportId);
    NimBLECharacteristic *getInputReport(uint8_t reportId);
    NimBLEHIDDeviceFix(NimBLEServer *server) : NimBLEHIDDevice(server) {};
}; // class NimBLEHIDDeviceFix

NimBLECharacteristic *NimBLEHIDDeviceFix::getOutputReport(uint8_t reportId)
{
    NimBLECharacteristic *outputReportChr =
        getHidService()->createCharacteristic(
            hidReportChrUuid,
            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR |
                NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::WRITE_ENC,
            2);
    NimBLEDescriptor *outputReportDsc = outputReportChr->createDescriptor(
        hidReportChrDscUuid,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::WRITE_ENC);

    uint8_t desc1_val[] = {reportId, 0x02};
    outputReportDsc->setValue(desc1_val, 2);

    return outputReportChr;
} // getOutputReport

NimBLECharacteristic *NimBLEHIDDeviceFix::getFeatureReport(uint8_t reportId)
{
    NimBLECharacteristic *featureReportChr = getHidService()->createCharacteristic(
        hidReportChrUuid,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::WRITE_ENC);

    NimBLEDescriptor *featureReportDsc = featureReportChr->createDescriptor(
        hidReportChrDscUuid,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::WRITE_ENC,
        2);

    uint8_t desc1_val[] = {reportId, 0x03};
    featureReportDsc->setValue(desc1_val, 2);

    return featureReportChr;
} // getFeatureReport

NimBLECharacteristic *NimBLEHIDDeviceFix::getInputReport(uint8_t reportId)
{
    // Note: the NOTIFY flag will internally create the 0x2902 descriptor
    NimBLECharacteristic *inputReportChr =
        getHidService()->createCharacteristic(
            hidReportChrUuid,
            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ_ENC);

    NimBLEDescriptor *inputReportDsc =
        inputReportChr->createDescriptor(
            hidReportChrDscUuid,
            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC,
            2);
    uint8_t desc1_val[] = {reportId, 0x01};
    inputReportDsc->setValue(desc1_val, 2);

    return inputReportChr;
} // getInputReport
*/
// ----------------------------------------------------------------------------
// PHY configuration
// ----------------------------------------------------------------------------

bool setDefaultPhy(uint8_t txPhyMask, uint8_t rxPhyMask)
{
    int rc = ble_gap_set_prefered_default_le_phy(txPhyMask, rxPhyMask);
    return rc == 0;
}

// ----------------------------------------------------------------------------
// BLE Server callbacks and advertising
// ----------------------------------------------------------------------------

void startBLEAdvertising()
{
    NimBLEDevice::startAdvertising();
    OnDisconnected::notify();
    if (autoPowerOffTimer != nullptr)
        esp_timer_start_once(autoPowerOffTimer, AUTO_POWER_OFF_DELAY_SECS * 1000000);
}

class BleConnectionStatus : public NimBLEServerCallbacks
{
public:
    BleConnectionStatus(void) {};
    bool connected = false;

    // Not needed for now
    // void onConnect(
    //     NimBLEServer *pServer,
    //     NimBLEConnInfo &connInfo) override {
    //     pServer->updateConnParams(connInfo.getConnHandle(), 6, 7, 0, 600);
    // };

    // Fix Windows notifications not being sent on reconnection
    // See https://github.com/lemmingDev/ESP32-BLE-Gamepad/pull/257/files
    void onAuthenticationComplete(NimBLEConnInfo &connInfo) override
    {
        if (autoPowerOffTimer != nullptr)
            esp_timer_stop(autoPowerOffTimer);
        connected = true;
        //************************************************
        // Do not call internals::hid::reset() here
        //************************************************
        // Quoting h2zero:
        //
        // When Windows bonds with a device and subscribes to notifications/indications
        // of the device characteristics it does not re-subscribe on subsequent connections.
        // If a notification is sent when Windows reconnects it will overwrite the stored subscription value
        // in the NimBLE stack configuration with an invalid value which
        // results in notifications/indications not being sent.
        OnConnected::notify();
    }

    void onDisconnect(
        NimBLEServer *pServer,
        NimBLEConnInfo &connInfo,
        int reason) override
    {
        connected = false;
        startBLEAdvertising();
    };

} connectionStatus;

// ----------------------------------------------------------------------------
// HID FEATURE REQUEST callbacks
// ----------------------------------------------------------------------------

class FeatureReport : public NimBLECharacteristicCallbacks
{
public:
    void onWrite(
        NimBLECharacteristic *pCharacteristic,
        NimBLEConnInfo &connInfo) override;
    void onRead(
        NimBLECharacteristic *pCharacteristic,
        NimBLEConnInfo &connInfo) override;
    FeatureReport(uint8_t RID, uint16_t size);
    static void attachTo(NimBLEHIDDeviceFix *hidDevice, uint8_t RID, uint16_t size);

private:
    uint8_t _reportID;
    uint16_t _reportSize;
};

// RECEIVE DATA
void FeatureReport::onWrite(
    NimBLECharacteristic *pCharacteristic,
    NimBLEConnInfo &connInfo)
{
    size_t size = pCharacteristic->getValue().length();
    const uint8_t *data = pCharacteristic->getValue().data();
    internals::hid::common::onSetFeature(_reportID, data, size);

    // Workaround for bug in NimBLE-Arduino v2.1.0
    onRead(pCharacteristic, connInfo);
}

// SEND REQUESTED DATA
void FeatureReport::onRead(
    NimBLECharacteristic *pCharacteristic,
    NimBLEConnInfo &connInfo)
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
void FeatureReport::attachTo(NimBLEHIDDeviceFix *hidDevice, uint8_t RID, uint16_t size)
{
    NimBLECharacteristic *reportCharacteristic = hidDevice->getFeatureReport(RID);
    if (!reportCharacteristic)
    {
        log_e("Unable to create HID report characteristics");
        abort();
    }
    reportCharacteristic->setCallbacks(new FeatureReport(RID, size));

    // Make the data available on first read
    uint8_t data[size];
    internals::hid::common::onGetFeature(RID, data, size);
    reportCharacteristic->setValue(data, size);
}

// ----------------------------------------------------------------------------
// HID OUTPUT REPORT callbacks
// ----------------------------------------------------------------------------

class OutputReport : public NimBLECharacteristicCallbacks
{
public:
    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override;
    OutputReport(uint8_t RID);
    static void attachTo(NimBLEHIDDeviceFix *hidDevice, uint8_t RID);

private:
    uint8_t _reportID;
};

OutputReport::OutputReport(uint8_t RID)
{
    _reportID = RID;
}

// RECEIVE DATA
void OutputReport::onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo)
{
    size_t size = pCharacteristic->getValue().length();
    const uint8_t *data = pCharacteristic->getValue().data();
    internals::hid::common::onOutput(_reportID, data, size);
}

// Attach to HID device
void OutputReport::attachTo(NimBLEHIDDeviceFix *hidDevice, uint8_t RID)
{
    NimBLECharacteristic *reportCharacteristic = hidDevice->getOutputReport(RID);
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
        // Auto-power-off initialization
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
        // NimBLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_BOND);
        NimBLEDevice::setSecurityAuth(true, false, false);
        NimBLEDevice::setMTU(BLE_MTU_SIZE);
        setDefaultPhy(BLE_GAP_LE_PHY_2M_MASK, BLE_GAP_LE_PHY_2M_MASK);
        pServer = NimBLEDevice::createServer();
        pServer->setCallbacks(&connectionStatus);

        // HID initialization
        hidDevice = new NimBLEHIDDeviceFix(pServer);
        if (!hidDevice)
        {
            log_e("Unable to create HID device");
            abort();
        }
        hidDevice->setManufacturer(deviceManufacturer);
        hidDevice->setPnp(BLE_VENDOR_SOURCE, vendorID, productID, PRODUCT_REVISION);
        hidDevice->setHidInfo(0x00, 0x01);
        hidDevice->setReportMap((uint8_t *)hid_descriptor, sizeof(hid_descriptor));

        // Add the serial number to the "Device Information" service
        uint64_t serialNumber;
        if (esp_efuse_mac_get_default((uint8_t *)(&serialNumber)) == ESP_OK)
        {
            NimBLEService *deviceInfo = hidDevice->getDeviceInfoService();
            if (deviceInfo)
            {
                NimBLECharacteristic *serialNumChr = deviceInfo->getCharacteristic(serialNumberChrUuid);
                if (!serialNumChr)
                    serialNumChr =
                        deviceInfo->createCharacteristic(serialNumberChrUuid, NIMBLE_PROPERTY::READ);
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
        inputGamePad = hidDevice->getInputReport(RID_INPUT_GAMEPAD);
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
        NimBLEAdvertising *pAdvertising = pServer->getAdvertising();
        pAdvertising->setAppearance(HID_GAMEPAD);
        pAdvertising->setName(deviceName);
        pAdvertising->enableScanResponse(true);
        pAdvertising->addServiceUUID(hidDevice->getHidService()->getUUID());

        // Start services
        hidDevice->startServices();
        hidDevice->setBatteryLevel(UNKNOWN_BATTERY_LEVEL);
        startBLEAdvertising();
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
        inputGamePad->setValue((const uint8_t *)report, GAMEPAD_REPORT_SIZE);
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
        inputGamePad->setValue((const uint8_t *)report, GAMEPAD_REPORT_SIZE);
        inputGamePad->notify();
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
        hidDevice->setBatteryLevel(level, true);
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
