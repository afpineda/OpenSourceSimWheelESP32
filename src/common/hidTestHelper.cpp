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
#include "Testing.hpp"

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
        debugPrintf("CMD: recalibrate axes\n");
    }

    virtual void reverseLeftAxis() override
    {
        debugPrintf("CMD: reverse left axis\n");
    }

    virtual void reverseRightAxis() override
    {
        debugPrintf("CMD: reverse right axis\n");
    }

    virtual void setRotaryPulseWidthMultiplier(
        PulseWidthMultiplier multiplier,
        bool save) override
    {
        debugPrintf("CMD: pulse width x%hhu\n", (uint8_t)multiplier);
    }

} inputMock;

//------------------------------------------------------------------

class BattCalMock : public BatteryCalibrationService
{
    virtual void restartAutoCalibration() override
    {
        debugPrintf("CMD: recalibrate battery\n");
    }
} battCalMock;

//------------------------------------------------------------------

class PowerMock : public PowerService
{
public:
    virtual void shutdown()
    {
        debugPrintf("*** POWER OFF ***\n");
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
    debugPrintf("*** CONNECTED ***\n");
}

void onDisconnectedCallback()
{
    debugPrintf("*** DISCOVERING ***\n");
}

//------------------------------------------------------------------

void internals::pixels::set(
    PixelGroup group,
    uint8_t pixelIndex,
    uint8_t red,
    uint8_t green,
    uint8_t blue)
{
    debugPrintf("pixels::set(%hhu,%hhu,%hhu,%hhu,%hhu)\n",
                (uint8_t)group,
                pixelIndex,
                red,
                green,
                blue);
}

void internals::pixels::reset()
{
    debugPrintf("pixels::reset()\n");
}

void internals::pixels::show()
{
    debugPrintf("pixels::show()\n");
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
    if (telemetry::data.frameID != lastFrameID)
    {
        lastFrameID = telemetry::data.frameID;
        debugPrintf("powertrain: %c %u %u %u %u %u %u %u\n",
                    telemetry::data.powertrain.gear,
                    telemetry::data.powertrain.rpm,
                    telemetry::data.powertrain.rpmPercent,
                    telemetry::data.powertrain.shiftLight1,
                    telemetry::data.powertrain.shiftLight2,
                    telemetry::data.powertrain.revLimiter,
                    telemetry::data.powertrain.engineStarted,
                    telemetry::data.powertrain.speed);
        debugPrintf("ecu: %u %u %u %u %u %u %u %u %u\n",
                    telemetry::data.ecu.absEngaged,
                    telemetry::data.ecu.tcEngaged,
                    telemetry::data.ecu.drsEngaged,
                    telemetry::data.ecu.pitLimiter,
                    telemetry::data.ecu.lowFuelAlert,
                    telemetry::data.ecu.absLevel,
                    telemetry::data.ecu.tcLevel,
                    telemetry::data.ecu.tcCut,
                    telemetry::data.ecu.brakeBias);
        debugPrintf("race control: %u %u %u %u %u %u %u %u %u\n",
                    telemetry::data.raceControl.blackFlag,
                    telemetry::data.raceControl.blueFlag,
                    telemetry::data.raceControl.checkeredFlag,
                    telemetry::data.raceControl.greenFlag,
                    telemetry::data.raceControl.orangeFlag,
                    telemetry::data.raceControl.whiteFlag,
                    telemetry::data.raceControl.yellowFlag,
                    telemetry::data.raceControl.remainingLaps,
                    telemetry::data.raceControl.remainingMinutes);
        debugPrintf("gauges: %u %.2f %u %.2f %u %u %u\n",
                    telemetry::data.gauges.relativeTurboPressure,
                    telemetry::data.gauges.absoluteTurboPressure,
                    telemetry::data.gauges.waterTemperature,
                    telemetry::data.gauges.oilPressure,
                    telemetry::data.gauges.oilTemperature,
                    telemetry::data.gauges.relativeRemainingFuel,
                    telemetry::data.gauges.absoluteRemainingFuel);
    }
}

void printBatteryStatus()
{
    debugPrintf("Current battery status:\n");
    debugPrintf(" State of charge: ");
    if (!battStatus.stateOfCharge.has_value())
        debugPrintf("unknown\n");
    else
        debugPrintf("yes\n");

    debugPrintf(" Wired power: ");
    if (!battStatus.usingExternalPower.has_value())
        debugPrintf("unknown");
    else if (battStatus.usingExternalPower.value())
        debugPrintf("yes\n");
    else
        debugPrintf("no\n");

    debugPrintf(" Charging: ");
    if (!battStatus.isCharging.has_value())
        debugPrintf("unknown\n");
    else if (battStatus.isCharging.value())
        debugPrintf("yes\n");
    else
        debugPrintf("no\n");

    debugPrintf(" Battery presence: ");
    if (!battStatus.isBatteryPresent.has_value())
        debugPrintf("unknown\n");
    else if (battStatus.isBatteryPresent.value())
        debugPrintf("yes\n");
    else
        debugPrintf("no\n");
} // printBatteryStatus()

void executeSerialCommands()
{
    int chr = -1;
#if ARDUINO_USB_CDC_ON_BOOT && !ARDUINO_USB_MODE
    chr = USBSerial.read();
#endif
#if ARDUINO_USB_CDC_ON_BOOT && ARDUINO_USB_MODE
    if (chr < 0)
        chr = HWCDCSerial.read();
#endif
    if (chr < 0)
        chr = Serial0.read();

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
    else if (chr == '?')
    {
        printBatteryStatus();
    }
} // executeSerialCommands()

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    debugPrintBegin();
    debugPrintf("--START--\n");
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
#if BLE_ONLY
    hid::connectivity(Connectivity::BLE);
#elif USB_ONLY
    hid::connectivity(Connectivity::USB);
#else
    hid::connectivity(Connectivity::USB_BLE_EXCLUSIVE);
#endif
    hid::configure(
        HID_TESTER,
        "Mamandurrio",
        true,
        TEST_HARDWARE_ID,
        TEST_HARDWARE_ID);
    internals::hid::common::getReady();
    OnStart::notify();

    if (!internals::hid::supportsCustomHardwareID())
        debugPrintf("Actual VID / PID depends on DevKit (not BLE)\n");
    else
        debugPrintf(
            "Actual VID / PID: %04x / %04x ..OR.. 303A / %04x",
            BLE_VENDOR_ID,
            BLE_PRODUCT_ID,
            BLE_PRODUCT_ID);
    debugPrintf("--GO--\n");
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
        debugPrintf("(Reset required)\n");
        for (;;)
            ;
    }

    if (!internals::hid::isConnected())
    {
        debugPrintf("(Waiting for connection)\n");
    }
    else
    {
        uint128_t data{};
        data.low = (1ULL << btnIndex);
        data.high = data.low;
        internals::hid::reportInput(
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