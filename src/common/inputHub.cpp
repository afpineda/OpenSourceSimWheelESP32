/**
 * @file inputHub.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Everything related to the combined state of
 *        all inputs and their treatment.
 *
 * @copyright Licensed under the EUPL
 *
 */

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "SimWheel.hpp"
#include <vector>

// #include <iostream> // For debug
// using namespace std;

//-------------------------------------------------------------------
// Globals
//-------------------------------------------------------------------

// Related to ALT buttons

static uint128_t altBitmap{};

// Related to clutch

#define CALIBRATION_INCREMENT 3
static uint8_t calibrateUp{0xFF};
static uint8_t calibrateDown{0xFF};
static uint8_t leftClutch{0xFF};
static uint8_t rightClutch{0xFF};

// Related to wheel functions

static uint128_t cycleALTWorkingModeBitmap{};
static uint128_t cycleClutchWorkingModeBitmap{};
static uint128_t cmdAxisAutocalibrationBitmap{};
static uint128_t cycleDPADWorkingModeBitmap{};
static uint128_t cycleSecurityLockBitmap{};
static uint128_t routedInputBitmap{};
static uint128_t routedInputBitmapNeg = uint128_t::neg();

// Related to POV buttons

#define DPAD_CENTERED 0
#define DPAD_UP 1
#define DPAD_UP_RIGHT 2
#define DPAD_RIGHT 3
#define DPAD_DOWN_RIGHT 4
#define DPAD_DOWN 5
#define DPAD_DOWN_LEFT 6
#define DPAD_LEFT 7
#define DPAD_UP_LEFT 8
static uint8_t dpadInputNumbers[9] =
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint128_t dpadMask = uint128_t::neg();

// Related to the neutral gear

static bool neutralWasEngaged = false;
static uint8_t neutralSwitch{0xFF};
static uint128_t neutralCombinationBitmap{};

// Related to coded switches

struct CodedSwitch
{
    inline static uint128_t mask = uint128_t::neg();
    inline static uint128_t decodedMask = uint128_t::neg();

    uint8_t bit1;
    uint8_t bit2;
    uint8_t bit4;
    uint8_t bit8;
    uint8_t bit16;
    ::std::vector<uint8_t> decodedIN{};

    template <typename T>
    CodedSwitch(
        const T &source,
        uint8_t b1,
        uint8_t b2,
        uint8_t b4,
        uint8_t b8 = 0xFF,
        uint8_t b16 = 0xFF) noexcept
        : bit1{b1}, bit2{b2}, bit4{b4}, bit8{b8}, bit16{b16}, decodedIN{}
    {
        decodedIN.resize(source.size());
        for (size_t i = 0; i < source.size(); i++)
        {
            decodedIN[i] = source[i];
            decodedMask.set_bit(decodedIN[i], false);
        }
        mask.set_bit(bit1, false);
        mask.set_bit(bit2, false);
        mask.set_bit(bit4, false);
        mask.set_bit(bit8, false);
        mask.set_bit(bit16, false);
    }

    CodedSwitch(const CodedSwitch &) = default;
    CodedSwitch(CodedSwitch &&) = default;
};

static std::vector<CodedSwitch> _codedSwitches;

// Related to the input hub itself

static uint128_t previousInputBitmap{};

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Public API
//-------------------------------------------------------------------
//-------------------------------------------------------------------

void inputHub::clutch::inputs(
    InputNumber leftInputNumber,
    InputNumber rightInputNumber)
{
    if ((leftInputNumber == UNSPECIFIED::VALUE) ||
        (rightInputNumber == UNSPECIFIED::VALUE))
        throw invalid_input_number();
    if (leftInputNumber == rightInputNumber)
        throw std::runtime_error(
            "You can not assign the same input number for the left and right clutch paddles");

    leftClutch = leftInputNumber;
    rightClutch = rightInputNumber;
    if (DeviceCapabilities::hasFlag(DeviceCapability::CLUTCH_ANALOG))
    {
        leftInputNumber.book();
        rightInputNumber.book();
    }
    else
        DeviceCapabilities::setFlag(DeviceCapability::CLUTCH_BUTTON);
}

void inputHub::clutch::bitePointInputs(
    InputNumber increase,
    InputNumber decrease)
{
    if ((increase == UNSPECIFIED::VALUE) ||
        (decrease == UNSPECIFIED::VALUE))
        throw invalid_input_number();
    if (increase == decrease)
        throw std::runtime_error(
            "You can not assign the same input number "
            "for increase and decrease bite point");

    calibrateUp = increase;
    calibrateDown = decrease;
}

void inputHub::clutch::cycleWorkingModeInputs(
    const InputNumberCombination &inputNumbers)
{
    cycleClutchWorkingModeBitmap = inputNumbers;
}

void inputHub::clutch::cmdRecalibrateAxisInputs(
    const InputNumberCombination &inputNumbers)
{
    cmdAxisAutocalibrationBitmap = inputNumbers;
}

//-------------------------------------------------------------------

void inputHub::dpad::inputs(
    InputNumber padUpNumber,
    InputNumber padDownNumber,
    InputNumber padLeftNumber,
    InputNumber padRightNumber)
{
    if ((padUpNumber == UNSPECIFIED::VALUE) ||
        (padDownNumber == UNSPECIFIED::VALUE) ||
        (padLeftNumber == UNSPECIFIED::VALUE) ||
        (padRightNumber == UNSPECIFIED::VALUE))
        throw invalid_input_number();

    if ((padUpNumber == padDownNumber) ||
        (padUpNumber == padLeftNumber) ||
        (padUpNumber == padRightNumber) ||
        (padDownNumber == padLeftNumber) ||
        (padLeftNumber == padRightNumber))
        throw std::runtime_error(
            "You can not assign the same input number to two DPAD inputs");

    dpadInputNumbers[DPAD_UP] = padUpNumber;
    dpadInputNumbers[DPAD_DOWN] = padDownNumber;
    dpadInputNumbers[DPAD_LEFT] = padLeftNumber;
    dpadInputNumbers[DPAD_RIGHT] = padRightNumber;
    dpadMask.set_bit(padUpNumber, false);
    dpadMask.set_bit(padDownNumber, false);
    dpadMask.set_bit(padLeftNumber, false);
    dpadMask.set_bit(padRightNumber, false);
    DeviceCapabilities::setFlag(DeviceCapability::DPAD, true);
}

void inputHub::dpad::cycleWorkingModeInputs(
    const InputNumberCombination &inputNumbers)
{
    cycleDPADWorkingModeBitmap = (inputNumbers);
}

//-------------------------------------------------------------------

void inputHub::altButtons::inputs(
    const InputNumberCombination &inputNumbers)
{
    altBitmap = (inputNumbers);
    DeviceCapabilities::setFlag(DeviceCapability::ALT, (bool)altBitmap);
}

void inputHub::altButtons::cycleWorkingModeInputs(
    const InputNumberCombination &inputNumbers)
{
    cycleALTWorkingModeBitmap = (inputNumbers);
}

//-------------------------------------------------------------------

void inputHub::securityLock::cycleWorkingModeInputs(
    const InputNumberCombination &inputNumbers)
{
    cycleSecurityLockBitmap = (inputNumbers);
}

//-------------------------------------------------------------------

void inputHub::neutralGear::set(
    InputNumber neutral,
    const InputNumberCombination &combination)
{
    if (combination.size() < 2)
        throw std::runtime_error(
            "For neutral gear, a combination of "
            "two or more hardware inputs is required");
    neutral.book();
    neutralSwitch = neutral;
    neutralCombinationBitmap = combination;
}

//-------------------------------------------------------------------

void throw_repeated_input_number()
{
    throw std::runtime_error(
        "input numbers used in all coded switches must be unique");
}

void inputHub::codedSwitch::add(
    InputNumber bit1,
    InputNumber bit2,
    InputNumber bit4,
    const CodedSwitch8 &spec)
{
    if ((bit1 == UNSPECIFIED::VALUE) ||
        (bit2 == UNSPECIFIED::VALUE) ||
        (bit4 == UNSPECIFIED::VALUE))
        throw invalid_input_number();

    if ((bit1 == bit2) || (bit1 == bit4) || (bit2 == bit4))
        throw_repeated_input_number();

    for (auto sw : _codedSwitches)
    {
        if (((uint8_t)bit1 == sw.bit1) || ((uint8_t)bit1 == sw.bit2) ||
            ((uint8_t)bit1 == sw.bit4) || ((uint8_t)bit1 == sw.bit8) ||
            ((uint8_t)bit1 == sw.bit16) || ((uint8_t)bit2 == sw.bit1) ||
            ((uint8_t)bit2 == sw.bit2) || ((uint8_t)bit2 == sw.bit4) ||
            ((uint8_t)bit2 == sw.bit8) || ((uint8_t)bit2 == sw.bit16) ||
            ((uint8_t)bit4 == sw.bit1) || ((uint8_t)bit4 == sw.bit2) ||
            ((uint8_t)bit4 == sw.bit4) || ((uint8_t)bit4 == sw.bit8) ||
            ((uint8_t)bit4 == sw.bit16))
            throw_repeated_input_number();
    }

    CodedSwitch another(
        spec,
        bit1,
        bit2,
        bit4);
    _codedSwitches.push_back(another);
}

void inputHub::codedSwitch::add(
    InputNumber bit1,
    InputNumber bit2,
    InputNumber bit4,
    InputNumber bit8,
    const CodedSwitch16 &spec)
{
    if ((bit1 == UNSPECIFIED::VALUE) ||
        (bit2 == UNSPECIFIED::VALUE) ||
        (bit4 == UNSPECIFIED::VALUE) ||
        (bit8 == UNSPECIFIED::VALUE))
        throw invalid_input_number();

    if ((bit1 == bit2) || (bit1 == bit4) || (bit1 == bit8) ||
        (bit2 == bit4) || (bit2 == bit8) || (bit4 == bit8))
        throw_repeated_input_number();

    for (auto sw : _codedSwitches)
    {
        if (((uint8_t)bit1 == sw.bit1) || ((uint8_t)bit1 == sw.bit2) ||
            ((uint8_t)bit1 == sw.bit4) || ((uint8_t)bit1 == sw.bit8) ||
            ((uint8_t)bit1 == sw.bit16) || ((uint8_t)bit2 == sw.bit1) ||
            ((uint8_t)bit2 == sw.bit2) || ((uint8_t)bit2 == sw.bit4) ||
            ((uint8_t)bit2 == sw.bit8) || ((uint8_t)bit2 == sw.bit16) ||
            ((uint8_t)bit4 == sw.bit1) || ((uint8_t)bit4 == sw.bit2) ||
            ((uint8_t)bit4 == sw.bit4) || ((uint8_t)bit4 == sw.bit8) ||
            ((uint8_t)bit4 == sw.bit16) || ((uint8_t)bit8 == sw.bit1) ||
            ((uint8_t)bit8 == sw.bit2) || ((uint8_t)bit8 == sw.bit4) ||
            ((uint8_t)bit8 == sw.bit8) || ((uint8_t)bit8 == sw.bit16))
            throw_repeated_input_number();
    }

    CodedSwitch another(
        spec,
        bit1,
        bit2,
        bit4,
        bit8);
    _codedSwitches.push_back(another);
}

void inputHub::codedSwitch::add(
    InputNumber bit1,
    InputNumber bit2,
    InputNumber bit4,
    InputNumber bit8,
    InputNumber bit16,
    const CodedSwitch32 &spec)
{
    if ((bit1 == UNSPECIFIED::VALUE) ||
        (bit2 == UNSPECIFIED::VALUE) ||
        (bit4 == UNSPECIFIED::VALUE) ||
        (bit8 == UNSPECIFIED::VALUE) ||
        (bit16 == UNSPECIFIED::VALUE))
        throw invalid_input_number();

    if ((bit1 == bit2) || (bit1 == bit4) || (bit1 == bit8) || (bit1 == bit16) ||
        (bit2 == bit4) || (bit2 == bit8) || (bit2 == bit16) ||
        (bit4 == bit8) || (bit4 == bit16) || (bit8 == bit16))
        throw_repeated_input_number();

    for (auto sw : _codedSwitches)
    {
        if (((uint8_t)bit1 == sw.bit1) || ((uint8_t)bit1 == sw.bit2) ||
            ((uint8_t)bit1 == sw.bit4) || ((uint8_t)bit1 == sw.bit8) ||
            ((uint8_t)bit1 == sw.bit16) || ((uint8_t)bit2 == sw.bit1) ||
            ((uint8_t)bit2 == sw.bit2) || ((uint8_t)bit2 == sw.bit4) ||
            ((uint8_t)bit2 == sw.bit8) || ((uint8_t)bit2 == sw.bit16) ||
            ((uint8_t)bit4 == sw.bit1) || ((uint8_t)bit4 == sw.bit2) ||
            ((uint8_t)bit4 == sw.bit4) || ((uint8_t)bit4 == sw.bit8) ||
            ((uint8_t)bit4 == sw.bit16) || ((uint8_t)bit8 == sw.bit1) ||
            ((uint8_t)bit8 == sw.bit2) || ((uint8_t)bit8 == sw.bit4) ||
            ((uint8_t)bit8 == sw.bit8) || ((uint8_t)bit8 == sw.bit16) ||
            ((uint8_t)bit16 == sw.bit1) || ((uint8_t)bit16 == sw.bit2) ||
            ((uint8_t)bit16 == sw.bit4) || ((uint8_t)bit16 == sw.bit8) ||
            ((uint8_t)bit8 == sw.bit16))
            throw_repeated_input_number();
    }

    CodedSwitch another(
        spec,
        bit1,
        bit2,
        bit4,
        bit8,
        bit16);
    _codedSwitches.push_back(another);
}

//-------------------------------------------------------------------

void inputHub::route_to_ui::add(InputNumber inputNumber)
{
    routedInputBitmap.set_bit(inputNumber, true);
    routedInputBitmapNeg.set_bit(inputNumber, false);
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Internal API
//-------------------------------------------------------------------
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// Service class
//-------------------------------------------------------------------

class InputHubServiceProvider : public InputHubService
{
public:
    inline static bool securityLock = false;
    inline static uint8_t bitePoint = CLUTCH_NONE_VALUE;
    inline static ClutchWorkingMode clutchWorkingMode =
        ClutchWorkingMode::CLUTCH;
    inline static AltButtonsWorkingMode altButtonsWorkingMode =
        AltButtonsWorkingMode::ALT;
    inline static DPadWorkingMode dpadWorkingMode =
        DPadWorkingMode::Navigation;

    virtual bool getSecurityLock() override
    {
        return securityLock;
    }

    virtual uint8_t getBitePoint() override
    {
        return bitePoint;
    }

    virtual ClutchWorkingMode getClutchWorkingMode() override
    {
        return clutchWorkingMode;
    }

    virtual AltButtonsWorkingMode getAltButtonsWorkingMode() override
    {
        return altButtonsWorkingMode;
    }

    virtual DPadWorkingMode getDPadWorkingMode() override
    {
        return dpadWorkingMode;
    }

    virtual void setBitePoint(uint8_t value, bool save) override
    {
        if ((value < CLUTCH_INVALID_VALUE) && (value != bitePoint))
        {
            bitePoint = value;
            InputService::call().update();
            OnBitePoint(value);
            if (save)
                SaveSetting(UserSetting::BITE_POINT);
        }
    }

    virtual void setClutchWorkingMode(
        ClutchWorkingMode mode,
        bool save) override
    {
        if (mode != clutchWorkingMode)
        {
            clutchWorkingMode = mode;
            if (save)
                SaveSetting(UserSetting::CLUTCH_WORKING_MODE);
        }
    }

    virtual void setAltButtonsWorkingMode(
        AltButtonsWorkingMode mode,
        bool save) override
    {
        if (mode != altButtonsWorkingMode)
        {
            altButtonsWorkingMode = mode;
            if (save)
                SaveSetting(UserSetting::ALT_WORKING_MODE);
        }
    }

    virtual void setDPadWorkingMode(DPadWorkingMode mode, bool save) override
    {
        if (mode != dpadWorkingMode)
        {
            dpadWorkingMode = mode;
            if (save)
                SaveSetting(UserSetting::DPAD_WORKING_MODE);
        }
    }

    virtual void setSecurityLock(bool value, bool save) override
    {
        if (securityLock != value)
        {
            securityLock = value;
            if (save)
                SaveSetting(UserSetting::SECURITY_LOCK);
        }
    }

    static void cycleClutchWorkingMode()
    {
        uint8_t next = (uint8_t)clutchWorkingMode + 1;
        if (next > (uint8_t)ClutchWorkingMode::_MAX_VALUE)
            next = 0;
        clutchWorkingMode = (ClutchWorkingMode)next;
        SaveSetting(UserSetting::CLUTCH_WORKING_MODE);
    }

    static void cycleAltButtonsWorkingMode()
    {
        uint8_t next = (uint8_t)altButtonsWorkingMode + 1;
        if (next > (uint8_t)AltButtonsWorkingMode::_MAX_VALUE)
            next = 0;
        altButtonsWorkingMode = (AltButtonsWorkingMode)next;
        SaveSetting(UserSetting::ALT_WORKING_MODE);
    }

    static void cycleDPadWorkingMode()
    {
        uint8_t next = (uint8_t)dpadWorkingMode + 1;
        if (next > (uint8_t)DPadWorkingMode::_MAX_VALUE)
            next = 0;
        dpadWorkingMode = (DPadWorkingMode)next;
        SaveSetting(UserSetting::DPAD_WORKING_MODE);
    }

    static void cycleSecurityLock()
    {
        securityLock = !securityLock;
        SaveSetting(UserSetting::SECURITY_LOCK);
    }

    static void increaseBitePoint()
    {
        if (bitePoint >= CLUTCH_FULL_VALUE)
            return;
        unsigned int next = bitePoint + CALIBRATION_INCREMENT;
        if (next > CLUTCH_FULL_VALUE)
            next = CLUTCH_FULL_VALUE;
        bitePoint = (uint8_t)next;
        OnBitePoint(bitePoint);
        SaveSetting(UserSetting::BITE_POINT);
    }

    static void decreaseBitePoint()
    {
        if (bitePoint == CLUTCH_NONE_VALUE)
            return;
        int prev = bitePoint - CALIBRATION_INCREMENT;
        if (prev < CLUTCH_NONE_VALUE)
            prev = CLUTCH_NONE_VALUE;
        bitePoint = (uint8_t)prev;
        OnBitePoint(bitePoint);
        SaveSetting(UserSetting::BITE_POINT);
    }
};

//-------------------------------------------------------------------
// Get ready
//-------------------------------------------------------------------

void abortOnUnknownIN(uint8_t in, const std::string &reason)
{
    if ((in < 128) && !InputNumber::booked(in))
        throw unknown_input_number(reason);
}

void abortOnUnknownIN(
    const uint128_t &bitmap,
    const std::string &reason)
{
    uint128_t test = (bitmap ^ InputNumber::booked()) & bitmap;
    if (test)
        throw unknown_input_number(reason);
}

void inputHubStart()
{
    LoadSetting(UserSetting::CLUTCH_WORKING_MODE);
    LoadSetting(UserSetting::BITE_POINT);
    LoadSetting(UserSetting::ALT_WORKING_MODE);
    LoadSetting(UserSetting::DPAD_WORKING_MODE);
    LoadSetting(UserSetting::SECURITY_LOCK);
}

void internals::inputHub::getReady()
{
    // Unbook coded input numbers (rotary coded switches)
    for (CodedSwitch &csw : _codedSwitches)
    {
        abortOnUnknownIN(csw.bit1, "Coded switch");
        abortOnUnknownIN(csw.bit2, "Coded switch");
        abortOnUnknownIN(csw.bit4, "Coded switch");
        abortOnUnknownIN(csw.bit8, "Coded switch");
        abortOnUnknownIN(csw.bit16, "Coded switch");
        InputNumber::unbook(csw.bit1);
        InputNumber::unbook(csw.bit2);
        InputNumber::unbook(csw.bit4);
        InputNumber::unbook(csw.bit8);
        InputNumber::unbook(csw.bit16);
    }

    // Book decoded input numbers (rotary coded switches)
    for (CodedSwitch &csw : _codedSwitches)
        for (uint8_t i = 0; i < csw.decodedIN.size(); i++)
            InputNumber::book(csw.decodedIN[i]);

    // Unbook routed input numbers
    for (uint8_t i = 0; i < 128; i++)
        if (routedInputBitmap.bit(i))
        {
            abortOnUnknownIN(i, "routed input number");
            InputNumber::unbook(i);
        }

    // Validate inputs for sim wheel functions
    abortOnUnknownIN(calibrateUp, "bite point (+) calibration");
    abortOnUnknownIN(calibrateDown, "bite point (-) calibration");
    abortOnUnknownIN(cycleClutchWorkingModeBitmap, "cycle clutch working mode");
    abortOnUnknownIN(cmdAxisAutocalibrationBitmap, "recalibrate axis");
    abortOnUnknownIN(neutralCombinationBitmap, "neutral gear");
    for (int n = 1; n < 9; n++)
        abortOnUnknownIN(dpadInputNumbers[n], "dpad input numbers");
    abortOnUnknownIN(cycleDPADWorkingModeBitmap, "cycle DPAD working mode");
    abortOnUnknownIN(altBitmap, "ALT buttons");
    abortOnUnknownIN(
        cycleALTWorkingModeBitmap,
        "cycle ALT buttons working mode");
    abortOnUnknownIN(
        cycleSecurityLockBitmap,
        "cycle security lock working mode");

    // Check for incoherent sim wheel functions
    if (DeviceCapabilities::hasFlag(DeviceCapability::CLUTCH_ANALOG))
    {
        if (leftClutch > 127)
            throw std::runtime_error(
                "You have analog clutch paddles, "
                "but you forgot to call inputHub::clutch::inputs()");
    }
    else
    {
        abortOnUnknownIN(leftClutch, "left clutch paddle");
        abortOnUnknownIN(rightClutch, "right clutch paddle");
        if (cmdAxisAutocalibrationBitmap)
            throw std::runtime_error(
                "There are no analog clutch paddles, "
                "but you called cmdRecalibrateAxisInputs()");
    }
    if (!DeviceCapabilities::hasFlag(DeviceCapability::CLUTCH_ANALOG) &&
        !DeviceCapabilities::hasFlag(DeviceCapability::CLUTCH_BUTTON))
    {
        if (calibrateUp < 128)
            throw std::runtime_error(
                "There are no clutch paddles, but you called "
                "inputHub::clutch::bitePointInputs()");
        if (cycleClutchWorkingModeBitmap)
            throw std::runtime_error(
                "There are no clutch paddles, but you called "
                "inputHub::clutch::cycleWorkingModeInputs()");
    }
    if (!DeviceCapabilities::hasFlag(DeviceCapability::DPAD) &&
        (cycleDPADWorkingModeBitmap))
        throw std::runtime_error(
            "There is no DPAD, but you called "
            "inputHub::dpad::cycleWorkingModeInputs()");
    if (!DeviceCapabilities::hasFlag(DeviceCapability::ALT) &&
        (cycleALTWorkingModeBitmap))
        throw std::runtime_error(
            "There are no ALT buttons, but you called "
            "inputHub::altButtons::cycleWorkingModeInputs()");

    // Prepare to run
    InputHubService::inject(new InputHubServiceProvider());
    OnStart.subscribe(inputHubStart);
}

//-------------------------------------------------------------------
// Input processing
//-------------------------------------------------------------------

/**
 * @brief Decode binary-coded switches
 *
 * @example 0b111 -> 0b01000000
 */
void inputHub_decode_bin_coded_switches(uint128_t &globalState)
{
    uint128_t decoded_bitmap{};
    for (auto sw : _codedSwitches)
    {
        uint8_t positionIndex = 0;
        if (globalState.bit(sw.bit1))
            positionIndex = 1;
        if (globalState.bit(sw.bit2))
            positionIndex += 2;
        if (globalState.bit(sw.bit4))
            positionIndex += 4;
        if ((sw.decodedIN.size() > 15) && globalState.bit(sw.bit8))
            positionIndex += 8;
        if ((sw.decodedIN.size() > 31) && globalState.bit(sw.bit16))
            positionIndex += 16;

        // Set decoded input
        decoded_bitmap.set_bit(sw.decodedIN[positionIndex], true);
    }
    globalState &= CodedSwitch::mask;
    globalState |= decoded_bitmap;
}

//-------------------------------------------------------------------

/**
 * @brief Executes user-requested commands in response to
 *        button press combinations
 *
 * @return true If a command has been issued
 * @return false Otherwise
 */
bool inputHub_commands_filter(
    const uint128_t &globalState,
    const uint128_t &changes)
{
    // Look for input events requesting a change in functionality
    // These input events never translate into a HID report
    if ((changes & cycleALTWorkingModeBitmap) &&
        (globalState == cycleALTWorkingModeBitmap))
    {
        InputHubServiceProvider::cycleAltButtonsWorkingMode();
        return true;
    }
    if ((changes & cycleClutchWorkingModeBitmap) &&
        (globalState == cycleClutchWorkingModeBitmap))
    {
        InputHubServiceProvider::cycleClutchWorkingMode();
        return true;
    }
    if ((changes & cycleDPADWorkingModeBitmap) &&
        (globalState == cycleDPADWorkingModeBitmap))
    {
        InputHubServiceProvider::cycleDPadWorkingMode();
        return true;
    }
    if ((changes & cmdAxisAutocalibrationBitmap) &&
        (globalState == cmdAxisAutocalibrationBitmap))
    {
        InputService::call().recalibrateAxes();
        return true;
    }
    if ((changes & cycleSecurityLockBitmap) &&
        (globalState == cycleSecurityLockBitmap))
    {
        InputHubServiceProvider::cycleSecurityLock();
        return true;
    }
    if ((changes & routedInputBitmap))
    {
        for (uint8_t input_number = 0; input_number < 128; input_number++)
        {
            if (routedInputBitmap.bit(input_number) &&
                changes.bit(input_number) &&
                !globalState.bit(input_number))
            {
                internals::ui::routeInput(input_number);
            }
        }
    }
    return false;
}

//-------------------------------------------------------------------

inline bool paddleIsPressed(uint8_t value)
{
    return (value > CLUTCH_3_4_VALUE);
}

inline bool paddleIsReleased(uint8_t value)
{
    return (value == CLUTCH_NONE_VALUE);
}

/**
 * @brief Executes bite point calibration from user input
 *
 */
void inputHub_bitePointCalibration_filter(
    DecouplingEvent &input,
    const uint128_t &changes)
{
    bool isCalibrationInProgress = false;
    switch (InputHubServiceProvider::clutchWorkingMode)
    {
    case ClutchWorkingMode::CLUTCH:
        isCalibrationInProgress =
            paddleIsPressed(input.leftAxisValue) &&
            paddleIsReleased(input.rightAxisValue);
        isCalibrationInProgress =
            isCalibrationInProgress ||
            (paddleIsReleased(input.leftAxisValue) &&
             paddleIsPressed(input.rightAxisValue));
        break;
    case ClutchWorkingMode::LAUNCH_CONTROL_MASTER_LEFT:
        isCalibrationInProgress =
            paddleIsReleased(input.leftAxisValue) &&
            paddleIsPressed(input.rightAxisValue);
        break;
    case ClutchWorkingMode::LAUNCH_CONTROL_MASTER_RIGHT:
        isCalibrationInProgress =
            paddleIsPressed(input.leftAxisValue) &&
            paddleIsReleased(input.rightAxisValue);
        break;
    default:
        break;
    }

    if (isCalibrationInProgress)
    {
        // One and only one clutch paddle is pressed
        // Check for bite point calibration events
        if (changes.bit(calibrateUp) &&
            input.rawInputBitmap.bit(calibrateUp))
            InputHubServiceProvider::increaseBitePoint();
        else if (changes.bit(calibrateDown) &&
                 input.rawInputBitmap.bit(calibrateDown))
            InputHubServiceProvider::decreaseBitePoint();
        input.rawInputBitmap.set_bit(calibrateUp, false);
        input.rawInputBitmap.set_bit(calibrateDown, false);
    }
}

//-------------------------------------------------------------------

/**
 * @brief Transforms input state into axis position and vice-versa,
 *        depending on the working mode of the clutch paddles.
 *
 */
void inputHub_AxisButton_filter(DecouplingEvent &input)
{
    if (DeviceCapabilities::hasFlag(DeviceCapability::CLUTCH_ANALOG) &&
        (InputHubServiceProvider::clutchWorkingMode ==
         ClutchWorkingMode::BUTTON))
    {
        // Transform analog axis position into an input state
        if (input.leftAxisValue >= CLUTCH_3_4_VALUE)
        {
            input.rawInputBitmap.set_bit(leftClutch, true);
        }
        else if (input.leftAxisValue <= CLUTCH_1_4_VALUE)
        {
            input.rawInputBitmap.set_bit(leftClutch, false);
        }
        if (input.rightAxisValue >= CLUTCH_3_4_VALUE)
        {
            input.rawInputBitmap.set_bit(rightClutch, true);
        }
        else if (input.rightAxisValue <= CLUTCH_1_4_VALUE)
        {
            input.rawInputBitmap.set_bit(rightClutch, false);
        }
        input.leftAxisValue = CLUTCH_NONE_VALUE;
        input.rightAxisValue = CLUTCH_NONE_VALUE;
        return;
    }

    if (DeviceCapabilities::hasFlag(DeviceCapability::CLUTCH_BUTTON))
    {
        bool isAxisMode =
            (InputHubServiceProvider::clutchWorkingMode ==
             ClutchWorkingMode::AXIS) ||
            (InputHubServiceProvider::clutchWorkingMode ==
             ClutchWorkingMode::CLUTCH) ||
            (InputHubServiceProvider::clutchWorkingMode ==
             ClutchWorkingMode::LAUNCH_CONTROL_MASTER_LEFT) ||
            (InputHubServiceProvider::clutchWorkingMode ==
             ClutchWorkingMode::LAUNCH_CONTROL_MASTER_RIGHT);
        if (isAxisMode)
        {
            // Transform input state into an axis position
            if (input.rawInputBitmap.bit(leftClutch))
                input.leftAxisValue = CLUTCH_FULL_VALUE;
            else
                input.leftAxisValue = CLUTCH_NONE_VALUE;
            if (input.rawInputBitmap.bit(rightClutch))
                input.rightAxisValue = CLUTCH_FULL_VALUE;
            else
                input.rightAxisValue = CLUTCH_NONE_VALUE;
            input.rawInputBitmap.set_bit(leftClutch, false);
            input.rawInputBitmap.set_bit(rightClutch, false);
        }
    }
}

//-------------------------------------------------------------------

/**
 * @brief Computes a combined clutch position from analog axes,
 *        when needed
 *
 */
void inputHub_combinedAxis_filter(
    uint8_t &leftAxis,
    uint8_t &rightAxis,
    uint8_t &clutchAxis)
{
    switch (InputHubServiceProvider::clutchWorkingMode)
    {
    case ClutchWorkingMode::CLUTCH:
        if (leftAxis > rightAxis)
            clutchAxis =
                (leftAxis * InputHubServiceProvider::bitePoint +
                 (rightAxis * (255 - InputHubServiceProvider::bitePoint))) /
                255;
        else
            clutchAxis =
                (rightAxis * InputHubServiceProvider::bitePoint +
                 (leftAxis * (255 - InputHubServiceProvider::bitePoint))) /
                255;
        leftAxis = CLUTCH_NONE_VALUE;
        rightAxis = CLUTCH_NONE_VALUE;
        break;

    case ClutchWorkingMode::LAUNCH_CONTROL_MASTER_LEFT:
        if (paddleIsPressed(rightAxis))
            clutchAxis = InputHubServiceProvider::bitePoint;
        else
            clutchAxis = CLUTCH_NONE_VALUE;
        if (leftAxis > clutchAxis)
            clutchAxis = leftAxis;
        leftAxis = CLUTCH_NONE_VALUE;
        rightAxis = CLUTCH_NONE_VALUE;
        break;

    case ClutchWorkingMode::LAUNCH_CONTROL_MASTER_RIGHT:
        if (paddleIsPressed(leftAxis))
            clutchAxis = InputHubServiceProvider::bitePoint;
        else
            clutchAxis = CLUTCH_NONE_VALUE;
        if (rightAxis > clutchAxis)
            clutchAxis = rightAxis;
        leftAxis = CLUTCH_NONE_VALUE;
        rightAxis = CLUTCH_NONE_VALUE;
        break;

    case ClutchWorkingMode::AXIS:
        clutchAxis = CLUTCH_NONE_VALUE;
        break;

    default:
        leftAxis = CLUTCH_NONE_VALUE;
        rightAxis = CLUTCH_NONE_VALUE;
        clutchAxis = CLUTCH_NONE_VALUE;
        break;
    }
}

//-------------------------------------------------------------------

/**
 * @brief Check if ALT mode is engaged by the user
 *
 */
void inputHub_AltRequest_filter(
    uint128_t &rawInputBitmap,
    uint8_t &leftAxis,
    uint8_t &rightAxis,
    bool &isAltRequested)
{
    if (InputHubServiceProvider::altButtonsWorkingMode ==
        AltButtonsWorkingMode::ALT)
    {
        isAltRequested = (bool)(rawInputBitmap & altBitmap);
        rawInputBitmap &= ~altBitmap;
    }
    if (InputHubServiceProvider::clutchWorkingMode == ClutchWorkingMode::ALT)
    {
        isAltRequested =
            isAltRequested ||
            (leftAxis >= CLUTCH_DEFAULT_VALUE) ||
            (rightAxis >= CLUTCH_DEFAULT_VALUE) ||
            rawInputBitmap.bit(leftClutch) ||
            rawInputBitmap.bit(rightClutch);
        leftAxis = CLUTCH_NONE_VALUE;
        rightAxis = CLUTCH_NONE_VALUE;
        rawInputBitmap.set_bit(leftClutch, false);
        rawInputBitmap.set_bit(rightClutch, false);
    }
}

//-------------------------------------------------------------------

/**
 * @brief Transform DPAD input into navigational input depending
 *        on user preferences
 *
 */
void inputHub_DPAD_filter(
    uint128_t &rawInputBitmap,
    uint8_t &povInput)
{
    povInput = DPAD_CENTERED;
    if (InputHubServiceProvider::dpadWorkingMode == DPadWorkingMode::Navigation)
    {
        // Map directional buttons to POV input as needed
        bool up = rawInputBitmap.bit(dpadInputNumbers[DPAD_UP]) ||
                  rawInputBitmap.bit(dpadInputNumbers[DPAD_UP_RIGHT]) ||
                  rawInputBitmap.bit(dpadInputNumbers[DPAD_UP_LEFT]);
        bool down = rawInputBitmap.bit(dpadInputNumbers[DPAD_DOWN]) ||
                    rawInputBitmap.bit(dpadInputNumbers[DPAD_DOWN_RIGHT]) ||
                    rawInputBitmap.bit(dpadInputNumbers[DPAD_DOWN_LEFT]);
        bool left = rawInputBitmap.bit(dpadInputNumbers[DPAD_LEFT]) ||
                    rawInputBitmap.bit(dpadInputNumbers[DPAD_UP_LEFT]) ||
                    rawInputBitmap.bit(dpadInputNumbers[DPAD_DOWN_LEFT]);
        bool right = rawInputBitmap.bit(dpadInputNumbers[DPAD_RIGHT]) ||
                     rawInputBitmap.bit(dpadInputNumbers[DPAD_UP_RIGHT]) ||
                     rawInputBitmap.bit(dpadInputNumbers[DPAD_DOWN_RIGHT]);
        rawInputBitmap &= dpadMask;

        if ((up && down) || (left && right))
            // incompatible inputs: do nothing
            return;
        else if (up && left)
            povInput = DPAD_UP_LEFT;
        else if (up && right)
            povInput = DPAD_UP_RIGHT;
        else if (down && left)
            povInput = DPAD_DOWN_LEFT;
        else if (down && right)
            povInput = DPAD_DOWN_RIGHT;
        else if (up)
            povInput = DPAD_UP;
        else if (down)
            povInput = DPAD_DOWN;
        else if (left)
            povInput = DPAD_LEFT;
        else if (right)
            povInput = DPAD_RIGHT;
    }
}

/**
 * @brief Engage or disengage neutral gear
 *
 */
void inputHub_neutralGear_filter(uint128_t &rawInputBitmap)
{
    if (neutralSwitch < 128)
    {
        bool combinationPressed =
            (!(~rawInputBitmap & neutralCombinationBitmap));
        if (neutralWasEngaged &&
            (!(rawInputBitmap & neutralCombinationBitmap)))
            // all buttons in the combination are now released at the same time
            neutralWasEngaged = false;
        else if (!neutralWasEngaged && combinationPressed)
            // all buttons in the combination are now pressed at the same time
            neutralWasEngaged = true;
        if (neutralWasEngaged)
        {
            // Remove the button combination
            rawInputBitmap &= ~neutralCombinationBitmap;
            if (combinationPressed)
                // Add the "virtual" neutral gear button
                rawInputBitmap.set_bit(neutralSwitch, true);
        }
    }
}

//-------------------------------------------------------------------

void internals::inputHub::onRawInput(DecouplingEvent &input)
{
    // Decode binary-coded switches when required
    inputHub_decode_bin_coded_switches(input.rawInputBitmap);

    // Step 1: Execute user commands if any
    if (inputHub_commands_filter(
            input.rawInputBitmap,
            input.rawInputBitmap ^ previousInputBitmap))
    {
        previousInputBitmap = input.rawInputBitmap;
        internals::hid::reset();
        return;
    }

    // Step 2: digital input <--> analog axes
    inputHub_AxisButton_filter(input);

    // Step 3: bite point calibration
    inputHub_bitePointCalibration_filter(
        input,
        input.rawInputBitmap ^ previousInputBitmap);
    previousInputBitmap = input.rawInputBitmap;

    // Step 4: check if ALT mode is requested
    bool isALTRequested = false;
    inputHub_AltRequest_filter(
        input.rawInputBitmap,
        input.leftAxisValue,
        input.rightAxisValue,
        isALTRequested);

    // Step 5: compute F1-style clutch position
    uint8_t clutchAxis;
    inputHub_combinedAxis_filter(
        input.leftAxisValue,
        input.rightAxisValue,
        clutchAxis);

    // Step 6: compute DPAD input
    uint8_t povInput;
    if (isALTRequested)
        povInput = DPAD_CENTERED;
    else
        inputHub_DPAD_filter(input.rawInputBitmap, povInput);

    // Step 7: compute neutral gear engagement
    inputHub_neutralGear_filter(input.rawInputBitmap);

    // Step 8: map raw input state into HID button state
    input.rawInputBitmap &= routedInputBitmapNeg;
    internals::inputMap::map(
        isALTRequested,
        input.rawInputBitmap);

    // Step 9: send HID report
    internals::hid::reportInput(
        input.rawInputBitmap,
        povInput,
        input.leftAxisValue,
        input.rightAxisValue,
        clutchAxis);
}

//-------------------------------------------------------------------
