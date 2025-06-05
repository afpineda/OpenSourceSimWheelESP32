/**
 * @file inputs.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-05
 * @brief Everything related to hardware inputs and their events.
 *
 * @copyright Licensed under the EUPL
 *
 */

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InputHardware.hpp"
#include "HAL.hpp"
#include "InternalServices.hpp"
#include "InputValidation.hpp"

#include <cassert>
#include <cstring> // For memset()
#include <forward_list>
#include <algorithm> // For find()

#if !CD_CI

#include "freertos/FreeRTOS.h"
static QueueHandle_t decouplingQueue = nullptr;

#else

#include <thread>
#include <chrono>

#endif

//-------------------------------------------------------------------
// Globals
//-------------------------------------------------------------------

// Input hardware
static std::forward_list<DigitalInput *> digitalInputsChain = {};
static AnalogInput *leftAxis = nullptr;
static AnalogInput *rightAxis = nullptr;
static bool _reverseLeftAxis = false;
static bool _reverseRightAxis = false;

// Polling daemon
#define DEBOUNCE_MS 30
#define POLLING_TASK_STACK_SIZE 2 * 1024
static bool forceUpdate;

// Hub daemon
#define HUB_STACK_SIZE 4 * 1024

// Decoupling queue
#define EVENT_SIZE sizeof(DecouplingEvent)

//-------------------------------------------------------------------
// Input hardware
//-------------------------------------------------------------------

static void abortIfStarted()
{
    if (FirmwareService::call::isRunning())
        throw std::runtime_error("Input service already started");
}

//-------------------------------------------------------------------

uint8_t getI2CFullAddress(uint8_t I2CAddress, bool isFullAddress, I2CBus bus)
{
    static bool firstRun = true;
    static std::vector<uint8_t> i2cAddressesFromProbe;

    // Retrieve all 7-bit addresses found in the bus (only once)
    if (firstRun)
    {
        firstRun = false;
        internals::hal::i2c::probe(i2cAddressesFromProbe, bus);
    }

    // Check if a device is responding to the given
    // I2C address
    uint8_t fullAddress;
    if (isFullAddress)
    {
        if (std::find(
                i2cAddressesFromProbe.begin(),
                i2cAddressesFromProbe.end(),
                I2CAddress) != i2cAddressesFromProbe.end())
            fullAddress = I2CAddress;
        else
            fullAddress = 0xFF; // Device not found
    }
    else
    {
        fullAddress = internals::hal::i2c::findFullAddress(
            i2cAddressesFromProbe,
            I2CAddress);
    }

    if (fullAddress == 0xFF)
        throw i2c_device_not_found(I2CAddress, static_cast<int>(bus));
    else if (fullAddress == 0xFE)
        throw i2c_full_address_unknown(I2CAddress, static_cast<int>(bus));

    return fullAddress;
}

//-------------------------------------------------------------------

void inputs::addButton(InputGPIO pin, InputNumber inputNumber)
{
    abortIfStarted();
    internals::inputs::validate::button(pin, inputNumber);
#if !CD_CI
    digitalInputsChain.push_front(new DigitalButton(pin, inputNumber));
#endif
}

//-------------------------------------------------------------------

void inputs::addRotaryEncoder(
    InputGPIO clkPin,
    InputGPIO dtPin,
    InputNumber cwInputNumber,
    InputNumber ccwInputNumber,
    bool useAlternateEncoding)
{
    abortIfStarted();
    internals::inputs::validate::rotaryEncoder(dtPin, clkPin, cwInputNumber, ccwInputNumber);
#if !CD_CI
    digitalInputsChain.push_front(
        new RotaryEncoderInput(
            clkPin,
            dtPin,
            cwInputNumber,
            ccwInputNumber,
            useAlternateEncoding));
#endif
}

//-------------------------------------------------------------------

void inputs::addButtonMatrix(
    const ButtonMatrix &matrix,
    bool negativeLogic)
{
    abortIfStarted();
    internals::inputs::validate::buttonMatrix(matrix);
#if !CD_CI
    digitalInputsChain.push_front(
        new ButtonMatrixInput(matrix, negativeLogic));
#endif
}

//-------------------------------------------------------------------

void inputs::addAnalogMultiplexerGroup(
    OutputGPIO selectorPin1,
    OutputGPIO selectorPin2,
    OutputGPIO selectorPin3,
    const AnalogMultiplexerGroup<Mux8Pin> &chips)
{
    abortIfStarted();
    internals::inputs::validate::analogMultiplexer(
        {selectorPin1, selectorPin2, selectorPin3},
        chips);
#if !CD_CI
    digitalInputsChain.push_front(
        new AnalogMultiplexerInput(
            selectorPin1,
            selectorPin2,
            selectorPin3,
            chips));
#endif
}

void inputs::addAnalogMultiplexerGroup(
    OutputGPIO selectorPin1,
    OutputGPIO selectorPin2,
    OutputGPIO selectorPin3,
    OutputGPIO selectorPin4,
    const AnalogMultiplexerGroup<Mux16Pin> &chips)
{
    abortIfStarted();
    internals::inputs::validate::analogMultiplexer(
        {selectorPin1, selectorPin2, selectorPin3, selectorPin4},
        chips);
#if !CD_CI
    digitalInputsChain.push_front(
        new AnalogMultiplexerInput(
            selectorPin1,
            selectorPin2,
            selectorPin3,
            selectorPin4,
            chips));
#endif
}

void inputs::addAnalogMultiplexerGroup(
    OutputGPIO selectorPin1,
    OutputGPIO selectorPin2,
    OutputGPIO selectorPin3,
    OutputGPIO selectorPin4,
    OutputGPIO selectorPin5,
    const AnalogMultiplexerGroup<Mux32Pin> &chips)
{
    abortIfStarted();
    internals::inputs::validate::analogMultiplexer(
        {selectorPin1, selectorPin2, selectorPin3, selectorPin4, selectorPin5},
        chips);
#if !CD_CI
    digitalInputsChain.push_front(
        new AnalogMultiplexerInput(
            selectorPin1,
            selectorPin2,
            selectorPin3,
            selectorPin4,
            selectorPin5,
            chips));
#endif
}

//-------------------------------------------------------------------

void inputs::addMCP23017Expander(
    const MCP23017Expander &chip,
    uint8_t address,
    bool isFullAddress,
    I2CBus bus)
{
    abortIfStarted();
    internals::inputs::validate::GPIOExpander<MCP23017Pin>(chip);
#if !CD_CI
    uint8_t fullAddress = getI2CFullAddress(address, isFullAddress, bus);
    digitalInputsChain.push_front(
        new MCP23017ButtonsInput(chip, fullAddress, bus));
#endif
}

void inputs::addPCF8574Expander(
    const PCF8574Expander &chip,
    uint8_t address,
    bool isFullAddress,
    I2CBus bus)
{
    abortIfStarted();
    internals::inputs::validate::GPIOExpander<PCF8574Pin>(chip);
#if !CD_CI
    uint8_t fullAddress = getI2CFullAddress(address, isFullAddress, bus);
    digitalInputsChain.push_front(
        new PCF8574ButtonsInput(chip, fullAddress, bus));
#endif
}

//-------------------------------------------------------------------

void inputs::add74HC165NChain(
    OutputGPIO loadPin,
    OutputGPIO nextPin,
    InputGPIO inputPin,
    const ShiftRegisterChain &chain,
    InputNumber SER_inputNumber,
    const bool negativeLogic)
{
    abortIfStarted();
    internals::inputs::validate::shiftRegisterChain(
        loadPin,
        nextPin,
        inputPin,
        chain);
    // Instantiate
#if !CD_CI
    digitalInputsChain.push_front(
        new ShiftRegistersInput(
            loadPin,
            nextPin,
            inputPin,
            chain,
            negativeLogic));
#endif
}

//-------------------------------------------------------------------

void inputs::addRotaryCodedSwitch(
    const RotaryCodedSwitch &spec,
    InputGPIO pin0,
    InputGPIO pin1,
    InputGPIO pin2,
    bool complementaryCode)
{
    InputGPIOCollection pins = {pin0, pin1, pin2};
    internals::inputs::validate::codedRotarySwitch(spec, pins);
#if !CD_CI
    digitalInputsChain.push_front(
        new RotaryCodedSwitchInput(
            spec,
            pins,
            complementaryCode));
#endif
}

void inputs::addRotaryCodedSwitch(
    const RotaryCodedSwitch &spec,
    InputGPIO pin0,
    InputGPIO pin1,
    InputGPIO pin2,
    InputGPIO pin3,
    bool complementaryCode)
{
    InputGPIOCollection pins = {pin0, pin1, pin2, pin3};
    internals::inputs::validate::codedRotarySwitch(spec, pins);
#if !CD_CI
    digitalInputsChain.push_front(
        new RotaryCodedSwitchInput(
            spec,
            pins,
            complementaryCode));
#endif
}

void inputs::addRotaryCodedSwitch(
    const RotaryCodedSwitch &spec,
    InputGPIO pin0,
    InputGPIO pin1,
    InputGPIO pin2,
    InputGPIO pin3,
    InputGPIO pin4,
    bool complementaryCode)
{
    InputGPIOCollection pins = {pin0, pin1, pin2, pin3, pin4};
    internals::inputs::validate::codedRotarySwitch(spec, pins);
#if !CD_CI
    digitalInputsChain.push_front(
        new RotaryCodedSwitchInput(
            spec,
            pins,
            complementaryCode));
#endif
}

//-------------------------------------------------------------------

void inputs::setAnalogClutchPaddles(
    ADC_GPIO leftClutchPin,
    ADC_GPIO rightClutchPin)
{
    abortIfStarted();
    if (leftAxis != nullptr)
        throw std::runtime_error("inputs::setAnalogClutchPaddles() called twice");
    DeviceCapabilities::setFlag(DeviceCapability::CLUTCH_ANALOG);
#if !CD_CI
    leftAxis = new AnalogClutchInput(leftClutchPin);
    rightAxis = new AnalogClutchInput(rightClutchPin);
#endif
}

//-------------------------------------------------------------------

void internals::inputs::addFakeInput(FakeInput *instance)
{
    abortIfStarted();
    if (leftAxis == nullptr)
    {
        leftAxis = new FakeAxis(instance, true);
        rightAxis = new FakeAxis(instance, false);
        DeviceCapabilities::setFlag(DeviceCapability::CLUTCH_ANALOG);
    }
    digitalInputsChain.push_front(new FakeDigitalInput(instance));
}

//-------------------------------------------------------------------
// Input service
//-------------------------------------------------------------------

class InputServiceProvider : public InputService
{
public:
    virtual void recalibrateAxes() override
    {
        if (leftAxis)
        {
            leftAxis->resetCalibrationData();
            rightAxis->resetCalibrationData();
        }
    }

    virtual void reverseLeftAxis() override
    {
        _reverseLeftAxis = !_reverseLeftAxis;
        SaveSetting::notify(UserSetting::AXIS_POLARITY);
    }

    virtual void reverseRightAxis() override
    {
        _reverseRightAxis = !_reverseRightAxis;
        SaveSetting::notify(UserSetting::AXIS_POLARITY);
    }

    virtual void setRotaryPulseWidthMultiplier(
        PulseWidthMultiplier multiplier,
        bool save) override
    {
        RotaryEncoderInput::setPulseMultiplier((uint8_t)multiplier);
        if (save)
            SaveSetting::notify(UserSetting::PULSE_WIDTH);
    }

    virtual PulseWidthMultiplier getRotaryPulseWidthMultiplier() override
    {
        return (PulseWidthMultiplier)RotaryEncoderInput::pulseMultiplier;
    };

    virtual bool getAxisCalibration(
        int &minLeft,
        int &maxLeft,
        int &minRight,
        int &maxRight)
    {
        if (leftAxis)
        {
            leftAxis->getCalibrationData(minLeft, maxLeft);
            rightAxis->getCalibrationData(minRight, maxRight);
            return true;
        }
        return false;
    }

    virtual void setAxisCalibration(
        int minLeft,
        int maxLeft,
        int minRight,
        int maxRight,
        bool save)
    {
        if (leftAxis)
        {
            leftAxis->setCalibrationData(minLeft, maxLeft);
            rightAxis->setCalibrationData(minRight, maxRight);
            if (save)
                SaveSetting::notify(UserSetting::AXIS_CALIBRATION);
        }
    }

    virtual void getAxisPolarity(
        bool &leftAxisReversed,
        bool &rightAxisReversed) override
    {
        leftAxisReversed = _reverseLeftAxis;
        rightAxisReversed = _reverseRightAxis;
    }

    virtual void setAxisPolarity(
        bool leftAxisReversed,
        bool rightAxisReversed,
        bool save) override
    {
        _reverseLeftAxis = leftAxisReversed;
        _reverseRightAxis = rightAxisReversed;
        if (save)
            SaveSetting::notify(UserSetting::AXIS_POLARITY);
    }

    virtual void update()
    {
        forceUpdate = true;
    }
};

// ----------------------------------------------------------------------------
// Poll daemon
// ----------------------------------------------------------------------------

void inputPollingLoop(void *param)
{
    // Initialize
    DecouplingEvent currentState, previousState;
    bool leftAxisAutocalibrated = false;
    bool rightAxisAutocalibrated = false;
    bool stateChanged;
    currentState.leftAxisValue = CLUTCH_NONE_VALUE;
    currentState.rightAxisValue = CLUTCH_NONE_VALUE;
    currentState.rawInputBitmap = 0ULL;
    previousState = currentState;
    forceUpdate = true;

    // loop
    while (true)
    {
        // Read digital inputs
        currentState.rawInputBitmap = 0ULL;
        for (DigitalInput *input : digitalInputsChain)
        {
            // currentState.rawInputBitmap =
            //     (currentState.rawInputBitmap & input->mask) |
            //     input->read(previousState.rawInputBitmap);
            currentState.rawInputBitmap |= input->read(previousState.rawInputBitmap);
        }
        currentState.rawInputChanges = currentState.rawInputBitmap ^ previousState.rawInputBitmap;
        stateChanged = forceUpdate || (currentState.rawInputChanges);

        // Read analog inputs
        if (leftAxis)
        {
            // Left clutch axis
            leftAxis->read(currentState.leftAxisValue, leftAxisAutocalibrated);
            if (_reverseLeftAxis)
                currentState.leftAxisValue = CLUTCH_FULL_VALUE - currentState.leftAxisValue;

            // Right clutch axis
            rightAxis->read(currentState.rightAxisValue, rightAxisAutocalibrated);
            if (_reverseRightAxis)
                currentState.rightAxisValue = CLUTCH_FULL_VALUE - currentState.rightAxisValue;

            if (leftAxisAutocalibrated || rightAxisAutocalibrated)
                SaveSetting::notify(UserSetting::AXIS_CALIBRATION);

            stateChanged =
                stateChanged ||
                (currentState.leftAxisValue != previousState.leftAxisValue) ||
                (currentState.rightAxisValue != previousState.rightAxisValue);
        }

        // Check for a state change
        if (stateChanged)
        {
            // Push state into the decoupling queue
            internals::inputs::notifyInputEvent(currentState);
            previousState = currentState;
        }

        // Prepare for the next iteration
        forceUpdate = false;

        // wait for the next sampling interval
        DELAY_MS(DEBOUNCE_MS * 2);
    }
}

// ----------------------------------------------------------------------------
// Input Hub daemon
// ----------------------------------------------------------------------------

#if !CD_CI
void hubLoop(void *unused)
{
    DecouplingEvent currentState;

    while (true)
    {
        if (xQueueReceive(decouplingQueue, &currentState, portMAX_DELAY))
            internals::inputHub::onRawInput(currentState);
    } // end while
}
#endif

// ----------------------------------------------------------------------------
// Decoupling
// ----------------------------------------------------------------------------

inline void internals::inputs::notifyInputEvent(const DecouplingEvent &input)
{
#if CD_CI
    DecouplingEvent copy = input;
    internals::inputHub::onRawInput(copy);
#else
    xQueueSend(decouplingQueue, &input, 0);
#endif
}

// ----------------------------------------------------------------------------
// Start
// ----------------------------------------------------------------------------

void inputStart()
{
    if (!FirmwareService::call::isRunning())
    {
        if (DeviceCapabilities::hasFlag(DeviceCapability::ROTARY_ENCODERS))
            LoadSetting::notify(UserSetting::PULSE_WIDTH);
        if (leftAxis)
        {
            LoadSetting::notify(UserSetting::AXIS_CALIBRATION);
            LoadSetting::notify(UserSetting::AXIS_POLARITY);
        }

#if !CD_CI

        // Create event queue
        decouplingQueue = xQueueCreate(64, EVENT_SIZE);
        if (decouplingQueue == nullptr)
            throw std::runtime_error("Unable to create decoupling queue");

        TaskHandle_t task = nullptr;
        // Create and run the input hub
        xTaskCreate(hubLoop, "hub", HUB_STACK_SIZE, (void *)nullptr, INPUT_TASK_PRIORITY, &task);
        if (task == nullptr)
            throw std::runtime_error("Unable to create inputHub task");

        // Create and run the polling task
        task = nullptr;
        xTaskCreate(
            inputPollingLoop,
            "PolledInputs",
            POLLING_TASK_STACK_SIZE,
            nullptr,
            INPUT_TASK_PRIORITY,
            &task);
        if (task == nullptr)
            throw std::runtime_error("Unable to create polling task");

#else

        std::jthread pollingThread(inputPollingLoop, nullptr);
        pollingThread.detach();

#endif
    }
}

// ----------------------------------------------------------------------------
// Injection
// ----------------------------------------------------------------------------

void internals::inputs::getReady()
{
    InputService::inject<InputServiceProvider>(new InputServiceProvider());
    OnStart::subscribe(inputStart);
}