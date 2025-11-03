/**
 * @file hidTestHelper.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-09-21
 *
 * @brief Helper code for the testing of HID implementations.
 *
 * @copyright Licensed under the EUPL
 *
 */

#ifdef HID_TESTER

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "HID_definitions.hpp"
#include <HardwareSerial.h>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

bool powerSim = true;
extern uint16_t _factoryVID;
extern uint16_t _factoryPID;

uint32_t lastFrameID = 0;
BatteryStatus battStatus;

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

class UIServiceMock : public UIService
{
public:
    virtual uint8_t getMaxFPS() override { return 50; }
} uiMock;

//------------------------------------------------------------------

class InputServiceMock : public InputService
{
public:
    virtual void recalibrateAxes() override
    {
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
        Serial.println("CMD: recalibrate axes");
#endif
    }

    virtual void reverseLeftAxis() override
    {
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
        Serial.println("CMD: reverse left axis");
#endif
    }

    virtual void reverseRightAxis() override
    {
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
        Serial.println("CMD: reverse right axis");
#endif
    }

    virtual void setRotaryPulseWidthMultiplier(
        PulseWidthMultiplier multiplier,
        bool save) override
    {
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
        Serial.printf("CMD: pulse width x%hhu\n", (uint8_t)multiplier);
#endif
    }

} inputMock;

//------------------------------------------------------------------

class BattCalMock : public BatteryCalibrationService
{
    virtual void restartAutoCalibration() override
    {
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
        Serial.println("CMD: recalibrate battery");
#endif
    }

} battCalMock;

//------------------------------------------------------------------

class PowerMock : public PowerService
{
public:
    virtual void shutdown()
    {
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
        Serial.println("*** POWER OFF ***");
#endif
        powerSim = false;
    }

    struct call
    {
        VOID_SINGLETON_INVOKER(shutdown(), shutdown())
    };
} powerMock;

//------------------------------------------------------------------

void onConnectedCallback()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("*** CONNECTED ***");
#endif
}

void onDisconnectedCallback()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("*** DISCOVERING ***");
#endif
}

//------------------------------------------------------------------

void internals::pixels::set(
    PixelGroup group,
    uint8_t pixelIndex,
    uint8_t red,
    uint8_t green,
    uint8_t blue)
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.printf("pixels::set(%hhu,%hhu,%hhu,%hhu,%hhu)\n",
                  (uint8_t)group,
                  pixelIndex,
                  red,
                  green,
                  blue);
#endif
}

void internals::pixels::reset()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("pixels::reset()");
#endif
}

void internals::pixels::show()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("pixels::show()");
#endif
}

uint8_t internals::pixels::getCount(PixelGroup group)
{
    return 8;
}

void internals::pixels::getReady() {}

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void checkAndPrintTelemetryData()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    if (telemetry::data.frameID != lastFrameID)
    {
        lastFrameID = telemetry::data.frameID;
        Serial.printf("powertrain: %c %u %u %u %u %u %u %u\n",
                      telemetry::data.powertrain.gear,
                      telemetry::data.powertrain.rpm,
                      telemetry::data.powertrain.rpmPercent,
                      telemetry::data.powertrain.shiftLight1,
                      telemetry::data.powertrain.shiftLight2,
                      telemetry::data.powertrain.revLimiter,
                      telemetry::data.powertrain.engineStarted,
                      telemetry::data.powertrain.speed);
        Serial.printf("ecu: %u %u %u %u %u %u %u %u %u\n",
                      telemetry::data.ecu.absEngaged,
                      telemetry::data.ecu.tcEngaged,
                      telemetry::data.ecu.drsEngaged,
                      telemetry::data.ecu.pitLimiter,
                      telemetry::data.ecu.lowFuelAlert,
                      telemetry::data.ecu.absLevel,
                      telemetry::data.ecu.tcLevel,
                      telemetry::data.ecu.tcCut,
                      telemetry::data.ecu.brakeBias);
        Serial.printf("race control: %u %u %u %u %u %u %u %u %u\n",
                      telemetry::data.raceControl.blackFlag,
                      telemetry::data.raceControl.blueFlag,
                      telemetry::data.raceControl.checkeredFlag,
                      telemetry::data.raceControl.greenFlag,
                      telemetry::data.raceControl.orangeFlag,
                      telemetry::data.raceControl.whiteFlag,
                      telemetry::data.raceControl.yellowFlag,
                      telemetry::data.raceControl.remainingLaps,
                      telemetry::data.raceControl.remainingMinutes);
        Serial.printf("gauges: %u %.2f %u %.2f %u %u %u\n",
                      telemetry::data.gauges.relativeTurboPressure,
                      telemetry::data.gauges.absoluteTurboPressure,
                      telemetry::data.gauges.waterTemperature,
                      telemetry::data.gauges.oilPressure,
                      telemetry::data.gauges.oilTemperature,
                      telemetry::data.gauges.relativeRemainingFuel,
                      telemetry::data.gauges.absoluteRemainingFuel);
    }
#endif
}

void printBatteryStatus()
{
    Serial.println("Current battery status:");
    Serial.print(" State of charge: ");
    if (!battStatus.stateOfCharge.has_value())
        Serial.println("unknown");
    else
        Serial.println("yes");

    Serial.print(" Wired power: ");
    if (!battStatus.usingExternalPower.has_value())
        Serial.println("unknown");
    else if (battStatus.usingExternalPower.value())
        Serial.println("yes");
    else
        Serial.println("no");

    Serial.print(" Charging: ");
    if (!battStatus.isCharging.has_value())
        Serial.println("unknown");
    else if (battStatus.isCharging.value())
        Serial.println("yes");
    else
        Serial.println("no");

    Serial.print(" Battery presence: ");
    if (!battStatus.isBatteryPresent.has_value())
        Serial.println("unknown");
    else if (battStatus.isBatteryPresent.value())
        Serial.println("yes");
    else
        Serial.println("no");
}

void executeSerialCommands()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    int chr = Serial.read();
    if (chr == 'l' || chr == 'L')
    {
        if (battStatus.stateOfCharge.has_value())
            battStatus.stateOfCharge.reset();
        else
            battStatus.stateOfCharge = 99;
        printBatteryStatus();
    }
    else if (chr == 'w' || chr == 'W')
    {
        if (battStatus.usingExternalPower.has_value())
        {
            if (battStatus.usingExternalPower.value())
                battStatus.usingExternalPower = false;
            else
                battStatus.usingExternalPower.reset();
        }
        else
            battStatus.usingExternalPower = true;
        printBatteryStatus();
    }
    else if (chr == 'c' || chr == 'C')
    {
        if (battStatus.isCharging.has_value())
        {
            if (battStatus.isCharging.value())
                battStatus.isCharging = false;
            else
                battStatus.isCharging.reset();
        }
        else
            battStatus.isCharging = true;
        printBatteryStatus();
    }
    else if (chr == 'b' || chr == 'B')
    {
        if (battStatus.isBatteryPresent.has_value())
        {
            if (battStatus.isBatteryPresent.value())
                battStatus.isBatteryPresent = false;
            else
                battStatus.isBatteryPresent.reset();
        }
        else
            battStatus.isBatteryPresent = true;
        printBatteryStatus();
    }

#endif
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.begin(115200);
    Serial.println("--START--");
#endif
    battStatus.stateOfCharge = 99;
    battStatus.isBatteryPresent = true;
    DeviceCapabilities::setFlag(DeviceCapability::CLUTCH_ANALOG);
    DeviceCapabilities::setFlag(DeviceCapability::DPAD);
    DeviceCapabilities::setFlag(DeviceCapability::ALT);
    DeviceCapabilities::setFlag(DeviceCapability::TELEMETRY_POWERTRAIN);
    DeviceCapabilities::setFlag(DeviceCapability::TELEMETRY_ECU);
    DeviceCapabilities::setFlag(DeviceCapability::TELEMETRY_RACE_CONTROL);
    DeviceCapabilities::setFlag(DeviceCapability::TELEMETRY_GAUGES);
    DeviceCapabilities::setFlag(DeviceCapability::ROTARY_ENCODERS);
    InputNumber::bookAll();

    UIService::inject(&uiMock);
    InputService::inject(&inputMock);
    BatteryCalibrationService::inject(&battCalMock);
    PowerService::inject(&powerMock);
    OnConnected::subscribe(onConnectedCallback);
    OnDisconnected::subscribe(onDisconnectedCallback);
    hid::configure(
        HID_TESTER,
        "Mamandurrio",
        true,
        TEST_HARDWARE_ID,
        TEST_HARDWARE_ID);
    internals::hid::common::getReady();
    OnStart::notify();

#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    if (!internals::hid::supportsCustomHardwareID())
        Serial.println("Actual VID / PID depends on DevKit (not BLE)");
    else
        Serial.printf(
            "Actual VID / PID: %04x / %04x\n",
            BLE_VENDOR_ID,
            BLE_PRODUCT_ID);
    Serial.println("--GO--");
#endif
}

//------------------------------------------------------------------

uint8_t btnIndex = 0;
uint8_t axis = CLUTCH_NONE_VALUE;
uint8_t POV = 0;

void loop()
{
    if (!powerSim)
    {
        // Simulate power off
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
        Serial.println("(Reset required)");
#endif
        for (;;)
            ;
    }

    if (!internals::hid::isConnected())
    {
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
        Serial.println("(Waiting for connection)");
#endif
    }
    else
    {
        uint64_t data = (1ULL << btnIndex);
        internals::hid::reportInput(
            data,
            data,
            POV,
            axis,
            axis,
            axis);

        // Update pressed buttons
        btnIndex++;
        if (btnIndex > 63)
            btnIndex = 0;

        // Update DPAD state
        POV = POV + 1;
        if (POV > 8)
        {
            POV = 0;
            internals::hid::reportChangeInConfig();
        }

        // Update battery info
        if (battStatus.stateOfCharge.has_value())
        {
            battStatus.stateOfCharge = battStatus.stateOfCharge.value() - 1;
            if (battStatus.stateOfCharge.value() < 50)
                battStatus.stateOfCharge = 100;
        }
        internals::hid::reportBatteryLevel(battStatus);

        // Update analog axis values
        axis = axis + 5;
        if (axis >= CLUTCH_FULL_VALUE - 5)
            axis = CLUTCH_NONE_VALUE;

        // Print telemetry data (if any)
        checkAndPrintTelemetryData();
    }

    // Execute commands placed in the UART (if any)
    executeSerialCommands();

    // Wait a second
    delay(1000);
}

#endif