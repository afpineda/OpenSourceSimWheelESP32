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

// #include <iostream> // For debug

//-------------------------------------------------------------------
// Globals
//-------------------------------------------------------------------

// Related to ALT buttons

static uint64_t altBitmap = 0ULL;

// Related to clutch

#define CALIBRATION_INCREMENT 3
static uint64_t calibrateUpBitmap = 0ULL;
static uint64_t calibrateDownBitmap = 0ULL;
static uint64_t leftClutchBitmap = 0ULL;
static uint64_t rightClutchBitmap = 0ULL;
static uint64_t clutchInputMask = ~0ULL;

// Related to wheel functions

static uint64_t cycleALTWorkingModeBitmap = 0ULL;
static uint64_t cycleClutchWorkingModeBitmap = 0ULL;
static uint64_t cmdAxisAutocalibrationBitmap = 0ULL;
// static uint64_t cmdBatteryRecalibrationBitmap = 0ULL;
static uint64_t cycleDPADWorkingModeBitmap = 0ULL;
static uint64_t cycleSecurityLockBitmap = 0ULL;

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
static uint64_t dpadBitmap[9] = {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL};
static uint64_t dpadNegMask = 0ULL;
static uint64_t dpadMask = ~0ULL;

// Related to the neutral gear

static bool neutralWasEngaged = false;
static uint64_t neutralSwitchBitmap = 0ULL;
static uint64_t neutralCombinationBitmap = 0ULL;

// Related to coded switches

struct CodedSwitch
{
    const InputNumber *decodedIN = nullptr;
    size_t size;
    InputNumber bit1 = UNSPECIFIED::VALUE;
    InputNumber bit2 = UNSPECIFIED::VALUE;
    InputNumber bit4 = UNSPECIFIED::VALUE;
    InputNumber bit8 = UNSPECIFIED::VALUE;
    InputNumber bit16 = UNSPECIFIED::VALUE;
    uint64_t mask = ~0ULL;
    uint64_t decodedMask = ~0ULL;
};

static std::vector<CodedSwitch> _codedSwitches;

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
        throw std::runtime_error("You can not assign the same input number for the left and right clutch paddles");

    leftClutchBitmap = (uint64_t)leftInputNumber;
    rightClutchBitmap = (uint64_t)rightInputNumber;
    clutchInputMask = ~(leftClutchBitmap | rightClutchBitmap);
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
            "You can not assign the same input number for increase and decrease bite point");

    calibrateUpBitmap = (uint64_t)increase;
    calibrateDownBitmap = (uint64_t)decrease;
}

void inputHub::clutch::cycleWorkingModeInputs(InputNumberCombination inputNumbers)
{
    cycleClutchWorkingModeBitmap = (uint64_t)inputNumbers;
}

void inputHub::clutch::cmdRecalibrateAxisInputs(InputNumberCombination inputNumbers)
{
    cmdAxisAutocalibrationBitmap = (uint64_t)inputNumbers;
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

    dpadBitmap[DPAD_UP] = (uint64_t)(padUpNumber);
    dpadBitmap[DPAD_DOWN] = (uint64_t)(padDownNumber);
    dpadBitmap[DPAD_LEFT] = (uint64_t)(padLeftNumber);
    dpadBitmap[DPAD_RIGHT] = (uint64_t)(padRightNumber);
    dpadBitmap[DPAD_UP_LEFT] = dpadBitmap[DPAD_UP] | dpadBitmap[DPAD_LEFT];
    dpadBitmap[DPAD_UP_RIGHT] = dpadBitmap[DPAD_UP] | dpadBitmap[DPAD_RIGHT];
    dpadBitmap[DPAD_DOWN_LEFT] = dpadBitmap[DPAD_DOWN] | dpadBitmap[DPAD_LEFT];
    dpadBitmap[DPAD_DOWN_RIGHT] = dpadBitmap[DPAD_DOWN] | dpadBitmap[DPAD_RIGHT];

    dpadNegMask = 0ULL;
    for (int n = 1; n < 9; n++)
        dpadNegMask |= dpadBitmap[n];
    dpadMask = ~dpadNegMask;
    DeviceCapabilities::setFlag(DeviceCapability::DPAD, (dpadNegMask != 0ULL));
}

void inputHub::dpad::cycleWorkingModeInputs(InputNumberCombination inputNumbers)
{
    cycleDPADWorkingModeBitmap = (uint64_t)(inputNumbers);
}

//-------------------------------------------------------------------

void inputHub::altButtons::inputs(InputNumberCombination inputNumbers)
{
    altBitmap = (uint64_t)(inputNumbers);
    DeviceCapabilities::setFlag(DeviceCapability::ALT, (altBitmap != 0ULL));
}

void inputHub::altButtons::cycleWorkingModeInputs(InputNumberCombination inputNumbers)
{
    cycleALTWorkingModeBitmap = (uint64_t)(inputNumbers);
}

//-------------------------------------------------------------------

void inputHub::securityLock::cycleWorkingModeInputs(InputNumberCombination inputNumbers)
{
    cycleSecurityLockBitmap = (uint64_t)(inputNumbers);
}

//-------------------------------------------------------------------

void inputHub::neutralGear::set(
    InputNumber neutral,
    InputNumberCombination combination)
{
    if (combination.size() < 2)
        throw std::runtime_error(
            "For neutral gear, a combination of two or more hardware inputs is required");
    neutral.book();
    neutralSwitchBitmap = (uint64_t)neutral;
    neutralCombinationBitmap = (uint64_t)combination;
}

//-------------------------------------------------------------------

InputNumber *copyCodedSwitchSpec(InputNumber *source, size_t count)
{
    size_t size = sizeof(InputNumber) * count;
    InputNumber *dest = static_cast<InputNumber *>(malloc(size));
    if (dest == nullptr)
        std::runtime_error("Not enough memory to create a coded switch");
    memcpy(dest, source, size);
    return dest;
}

void throw_repeated_input_number()
{
    throw std::runtime_error("input numbers used in all coded switches must be unique");
}

void inputHub::codedSwitch::add(
    InputNumber bit1,
    InputNumber bit2,
    InputNumber bit4,
    CodedSwitch8 spec)
{
    if ((bit1 == UNSPECIFIED::VALUE) ||
        (bit2 == UNSPECIFIED::VALUE) ||
        (bit4 == UNSPECIFIED::VALUE))
        throw invalid_input_number();

    if ((bit1 == bit2) || (bit1 == bit4) || (bit2 == bit4))
        throw_repeated_input_number();

    for (auto sw : _codedSwitches)
    {
        if ((bit1 == sw.bit1) || (bit1 == sw.bit2) || (bit1 == sw.bit4) || (bit1 == sw.bit8) || (bit1 == sw.bit16) ||
            (bit2 == sw.bit1) || (bit2 == sw.bit2) || (bit2 == sw.bit4) || (bit2 == sw.bit8) || (bit2 == sw.bit16) ||
            (bit4 == sw.bit1) || (bit4 == sw.bit2) || (bit4 == sw.bit4) || (bit4 == sw.bit8) || (bit4 == sw.bit16))
            throw_repeated_input_number();
    }

    CodedSwitch another;
    another.bit1 = bit1;
    another.bit2 = bit2;
    another.bit4 = bit4;
    another.size = spec.size();
    another.decodedIN = copyCodedSwitchSpec(spec.data(), spec.size());
    _codedSwitches.push_back(another);
}

void inputHub::codedSwitch::add(
    InputNumber bit1,
    InputNumber bit2,
    InputNumber bit4,
    InputNumber bit8,
    CodedSwitch16 spec)
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
        if ((bit1 == sw.bit1) || (bit1 == sw.bit2) || (bit1 == sw.bit4) || (bit1 == sw.bit8) || (bit1 == sw.bit16) ||
            (bit2 == sw.bit1) || (bit2 == sw.bit2) || (bit2 == sw.bit4) || (bit2 == sw.bit8) || (bit2 == sw.bit16) ||
            (bit4 == sw.bit1) || (bit4 == sw.bit2) || (bit4 == sw.bit4) || (bit4 == sw.bit8) || (bit4 == sw.bit16) ||
            (bit8 == sw.bit1) || (bit8 == sw.bit2) || (bit8 == sw.bit4) || (bit8 == sw.bit8) || (bit8 == sw.bit16))
            throw_repeated_input_number();
    }

    CodedSwitch another;
    another.bit1 = bit1;
    another.bit2 = bit2;
    another.bit4 = bit4;
    another.bit8 = bit8;
    another.size = spec.size();
    another.decodedIN = copyCodedSwitchSpec(spec.data(), spec.size());
    _codedSwitches.push_back(another);
}

void inputHub::codedSwitch::add(
    InputNumber bit1,
    InputNumber bit2,
    InputNumber bit4,
    InputNumber bit8,
    InputNumber bit16,
    CodedSwitch32 spec)
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
        if ((bit1 == sw.bit1) || (bit1 == sw.bit2) || (bit1 == sw.bit4) || (bit1 == sw.bit8) || (bit1 == sw.bit16) ||
            (bit2 == sw.bit1) || (bit2 == sw.bit2) || (bit2 == sw.bit4) || (bit2 == sw.bit8) || (bit2 == sw.bit16) ||
            (bit4 == sw.bit1) || (bit4 == sw.bit2) || (bit4 == sw.bit4) || (bit4 == sw.bit8) || (bit4 == sw.bit16) ||
            (bit8 == sw.bit1) || (bit8 == sw.bit2) || (bit8 == sw.bit4) || (bit8 == sw.bit8) || (bit8 == sw.bit16) ||
            (bit16 == sw.bit1) || (bit16 == sw.bit2) || (bit16 == sw.bit4) || (bit16 == sw.bit8) || (bit8 == sw.bit16))
            throw_repeated_input_number();
    }

    CodedSwitch another;
    another.bit1 = bit1;
    another.bit2 = bit2;
    another.bit4 = bit4;
    another.bit8 = bit8;
    another.bit16 = bit16;
    another.size = spec.size();
    another.decodedIN = copyCodedSwitchSpec(spec.data(), spec.size());
    _codedSwitches.push_back(another);
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
    inline static ClutchWorkingMode clutchWorkingMode = ClutchWorkingMode::CLUTCH;
    inline static AltButtonsWorkingMode altButtonsWorkingMode = AltButtonsWorkingMode::ALT;
    inline static DPadWorkingMode dpadWorkingMode = DPadWorkingMode::Navigation;

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
            OnBitePoint::notify(value);
            if (save)
                SaveSetting::notify(UserSetting::BITE_POINT);
        }
    }

    virtual void setClutchWorkingMode(ClutchWorkingMode mode, bool save) override
    {
        if (mode != clutchWorkingMode)
        {
            clutchWorkingMode = mode;
            if (save)
                SaveSetting::notify(UserSetting::CLUTCH_WORKING_MODE);
        }
    }

    virtual void setAltButtonsWorkingMode(AltButtonsWorkingMode mode, bool save) override
    {
        if (mode != altButtonsWorkingMode)
        {
            altButtonsWorkingMode = mode;
            if (save)
                SaveSetting::notify(UserSetting::ALT_WORKING_MODE);
        }
    }

    virtual void setDPadWorkingMode(DPadWorkingMode mode, bool save) override
    {
        if (mode != dpadWorkingMode)
        {
            dpadWorkingMode = mode;
            if (save)
                SaveSetting::notify(UserSetting::DPAD_WORKING_MODE);
        }
    }

    virtual void setSecurityLock(bool value, bool save) override
    {
        if (securityLock != value)
        {
            securityLock = value;
            if (save)
                SaveSetting::notify(UserSetting::SECURITY_LOCK);
        }
    }

    static void cycleClutchWorkingMode()
    {
        uint8_t next = (uint8_t)clutchWorkingMode + 1;
        if (next > (uint8_t)ClutchWorkingMode::_MAX_VALUE)
            next = 0;
        clutchWorkingMode = (ClutchWorkingMode)next;
        SaveSetting::notify(UserSetting::CLUTCH_WORKING_MODE);
    }

    static void cycleAltButtonsWorkingMode()
    {
        uint8_t next = (uint8_t)altButtonsWorkingMode + 1;
        if (next > (uint8_t)AltButtonsWorkingMode::_MAX_VALUE)
            next = 0;
        altButtonsWorkingMode = (AltButtonsWorkingMode)next;
        SaveSetting::notify(UserSetting::ALT_WORKING_MODE);
    }

    static void cycleDPadWorkingMode()
    {
        uint8_t next = (uint8_t)dpadWorkingMode + 1;
        if (next > (uint8_t)DPadWorkingMode::_MAX_VALUE)
            next = 0;
        dpadWorkingMode = (DPadWorkingMode)next;
        SaveSetting::notify(UserSetting::DPAD_WORKING_MODE);
    }

    static void cycleSecurityLock()
    {
        securityLock = !securityLock;
        SaveSetting::notify(UserSetting::SECURITY_LOCK);
    }

    static void increaseBitePoint()
    {
        if (bitePoint >= CLUTCH_FULL_VALUE)
            return;
        unsigned int next = bitePoint + CALIBRATION_INCREMENT;
        if (next > CLUTCH_FULL_VALUE)
            next = CLUTCH_FULL_VALUE;
        bitePoint = (uint8_t)next;
        OnBitePoint::notify(bitePoint);
        SaveSetting::notify(UserSetting::BITE_POINT);
    }

    static void decreaseBitePoint()
    {
        if (bitePoint == CLUTCH_NONE_VALUE)
            return;
        int prev = bitePoint - CALIBRATION_INCREMENT;
        if (prev < CLUTCH_NONE_VALUE)
            prev = CLUTCH_NONE_VALUE;
        bitePoint = (uint8_t)prev;
        OnBitePoint::notify(bitePoint);
        SaveSetting::notify(UserSetting::BITE_POINT);
    }
};

//-------------------------------------------------------------------
// Get ready
//-------------------------------------------------------------------

void abortOnUnknownIN(uint64_t bitmap, std::string reason)
{
    if (bitmap == 0ULL)
        return;
    uint64_t booked = InputNumber::booked();
    for (uint8_t i = 0; i < 64; i++)
    {
        uint64_t toCheck = (1ULL << i);
        if ((bitmap & toCheck) && ((booked & toCheck) == 0ULL))
            throw unknown_input_number(reason);
    }
}

void inputHubStart()
{
    LoadSetting::notify(UserSetting::CLUTCH_WORKING_MODE);
    LoadSetting::notify(UserSetting::BITE_POINT);
    LoadSetting::notify(UserSetting::ALT_WORKING_MODE);
    LoadSetting::notify(UserSetting::DPAD_WORKING_MODE);
    LoadSetting::notify(UserSetting::SECURITY_LOCK);
}

void internals::inputHub::getReady()
{
    for (CodedSwitch &csw : _codedSwitches)
    {
        abortOnUnknownIN((uint64_t)csw.bit1, "Coded switch");
        abortOnUnknownIN((uint64_t)csw.bit2, "Coded switch");
        abortOnUnknownIN((uint64_t)csw.bit4, "Coded switch");
        abortOnUnknownIN((uint64_t)csw.bit8, "Coded switch");
        abortOnUnknownIN((uint64_t)csw.bit16, "Coded switch");
        csw.bit1.unbook();
        csw.bit2.unbook();
        csw.bit4.unbook();
        csw.bit8.unbook();
        csw.bit16.unbook();
        csw.mask = (uint64_t)csw.bit1 | (uint64_t)csw.bit2 | (uint64_t)csw.bit4 |
                   (uint64_t)csw.bit8 | (uint64_t)csw.bit16;
        csw.mask = ~csw.mask;
    }
    for (CodedSwitch &csw : _codedSwitches)
    {
        csw.decodedMask = ~0ULL;
        for (uint8_t i = 0; i < csw.size; i++)
        {
            csw.decodedIN[i].book();
            csw.decodedMask &= ~(uint64_t)csw.decodedIN[i];
        }
    }

    abortOnUnknownIN(calibrateUpBitmap, "bite point (+) calibration");
    abortOnUnknownIN(calibrateDownBitmap, "bite point (-) calibration");
    abortOnUnknownIN(cycleClutchWorkingModeBitmap, "cycle clutch working mode");
    abortOnUnknownIN(cmdAxisAutocalibrationBitmap, "recalibrate axis");
    abortOnUnknownIN(neutralCombinationBitmap, "neutral gear");
    for (int n = 1; n < 9; n++)
        abortOnUnknownIN(dpadBitmap[n], "dpad input numbers");
    abortOnUnknownIN(cycleDPADWorkingModeBitmap, "cycle DPAD working mode");
    abortOnUnknownIN(altBitmap, "ALT buttons");
    abortOnUnknownIN(cycleALTWorkingModeBitmap, "cycle ALT buttons working mode");
    abortOnUnknownIN(cycleSecurityLockBitmap, "cycle security lock working mode");
    if (DeviceCapabilities::hasFlag(DeviceCapability::CLUTCH_ANALOG))
    {
        if (clutchInputMask == ~0ULL)
            throw std::runtime_error(
                "You have analog clutch paddles, but you forgot to call inputHub::clutch::inputs()");
    }
    else
    {
        abortOnUnknownIN(leftClutchBitmap, "left clutch paddle");
        abortOnUnknownIN(rightClutchBitmap, "right clutch paddle");
        if (cmdAxisAutocalibrationBitmap)
            throw std::runtime_error(
                "There are no analog clutch paddles, but you called cmdRecalibrateAxisInputs()");
    }
    if (!DeviceCapabilities::hasFlag(DeviceCapability::CLUTCH_ANALOG) &&
        !DeviceCapabilities::hasFlag(DeviceCapability::CLUTCH_BUTTON))
    {
        if (calibrateDownBitmap || calibrateUpBitmap)
            throw std::runtime_error(
                "There are no clutch paddles, but you called inputHub::clutch::bitePointInputs()");
        if (cycleClutchWorkingModeBitmap)
            throw std::runtime_error(
                "There are no clutch paddles, but you called inputHub::clutch::cycleWorkingModeInputs()");
    }
    if (!DeviceCapabilities::hasFlag(DeviceCapability::DPAD) && (cycleDPADWorkingModeBitmap != 0ULL))
        throw std::runtime_error(
            "There is no DPAD, but you called inputHub::dpad::cycleWorkingModeInputs()");
    if (!DeviceCapabilities::hasFlag(DeviceCapability::ALT) && (cycleALTWorkingModeBitmap != 0ULL))
        throw std::runtime_error(
            "There are no ALT buttons, but you called inputHub::altButtons::cycleWorkingModeInputs()");
    InputHubService::inject(new InputHubServiceProvider());
    OnStart::subscribe(inputHubStart);
}

//-------------------------------------------------------------------
// Input processing
//-------------------------------------------------------------------

/**
 * @brief Decode binary-coded switches
 *
 * @example 0b111 -> 0b01000000
 */
void inputHub_decode_bin_coded_switches(
    uint64_t &globalState,
    uint64_t &changes)
{
    if (_codedSwitches.size() == 0)
        return;
    for (auto sw : _codedSwitches)
    {
        uint8_t positionIndex = 0;
        if ((uint64_t)sw.bit1 & globalState)
            positionIndex = 1;
        if ((uint64_t)sw.bit2 & globalState)
            positionIndex += 2;
        if ((uint64_t)sw.bit4 & globalState)
            positionIndex += 4;
        if ((sw.size > 15) && ((uint64_t)sw.bit8 & globalState))
            positionIndex += 8;
        if ((sw.size > 31) && ((uint64_t)sw.bit16 & globalState))
            positionIndex += 16;

        // std::cout << "SW index: " << (int)positionIndex << std::endl;

        uint64_t bitmap = (uint64_t)sw.decodedIN[positionIndex];
        globalState &= sw.mask & sw.decodedMask;
        globalState |= bitmap;
        bool changed = (changes & ~sw.mask);
        changes &= sw.mask & sw.decodedMask;
        if (changed)
            changed |= bitmap;
    }
}

//-------------------------------------------------------------------

/**
 * @brief Executes user-requested commands in response to button press combinations
 *
 * @return true If a command has been issued
 * @return false Otherwise
 */
bool inputHub_commands_filter(
    uint64_t globalState,
    uint64_t changes)
{
    // Look for input events requesting a change in functionality
    // These input events never translate into a HID report
    if ((changes & cycleALTWorkingModeBitmap) && (globalState == cycleALTWorkingModeBitmap))
    {
        InputHubServiceProvider::cycleAltButtonsWorkingMode();
        return true;
    }
    if ((changes & cycleClutchWorkingModeBitmap) && (globalState == cycleClutchWorkingModeBitmap))
    {
        InputHubServiceProvider::cycleClutchWorkingMode();
        return true;
    }
    if ((changes & cycleDPADWorkingModeBitmap) && (globalState == cycleDPADWorkingModeBitmap))
    {
        InputHubServiceProvider::cycleDPadWorkingMode();
        return true;
    }
    if ((changes & cmdAxisAutocalibrationBitmap) && (globalState == cmdAxisAutocalibrationBitmap))
    {
        InputService::call::recalibrateAxes();
        return true;
    }
    // Removed in version 7
    // if ((changes & cmdBatteryRecalibrationBitmap) && (globalState == cmdBatteryRecalibrationBitmap))
    // {
    //     BatteryCalibrationService::call::restartAutoCalibration();
    //     return true;
    // }
    if ((changes & cycleSecurityLockBitmap) && (globalState == cycleSecurityLockBitmap))
    {
        InputHubServiceProvider::cycleSecurityLock();
        return true;
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
void inputHub_bitePointCalibration_filter(DecouplingEvent &input)
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
        if ((calibrateUpBitmap & input.rawInputChanges) &&
            (calibrateUpBitmap & input.rawInputBitmap))
            InputHubServiceProvider::increaseBitePoint();
        else if ((calibrateDownBitmap & input.rawInputChanges) &&
                 (calibrateDownBitmap & input.rawInputBitmap))
            InputHubServiceProvider::decreaseBitePoint();
        input.rawInputBitmap &= (~(calibrateDownBitmap | calibrateUpBitmap));
        input.rawInputChanges &= (~(calibrateDownBitmap | calibrateUpBitmap));
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
        (InputHubServiceProvider::clutchWorkingMode == ClutchWorkingMode::BUTTON))
    {
        // Transform analog axis position into an input state
        if (input.leftAxisValue >= CLUTCH_3_4_VALUE)
        {
            input.rawInputBitmap |= leftClutchBitmap;
            input.rawInputChanges |= leftClutchBitmap;
        }
        else if (input.leftAxisValue <= CLUTCH_1_4_VALUE)
        {
            input.rawInputBitmap &= (~leftClutchBitmap);
            input.rawInputChanges |= leftClutchBitmap;
        }
        if (input.rightAxisValue >= CLUTCH_3_4_VALUE)
        {
            input.rawInputBitmap |= rightClutchBitmap;
            input.rawInputChanges |= rightClutchBitmap;
        }
        else if (input.rightAxisValue <= CLUTCH_1_4_VALUE)
        {
            input.rawInputBitmap &= (~rightClutchBitmap);
            input.rawInputChanges |= rightClutchBitmap;
        }
        input.leftAxisValue = CLUTCH_NONE_VALUE;
        input.rightAxisValue = CLUTCH_NONE_VALUE;
        return;
    }

    if (DeviceCapabilities::hasFlag(DeviceCapability::CLUTCH_BUTTON))
    {
        bool isAxisMode =
            (InputHubServiceProvider::clutchWorkingMode == ClutchWorkingMode::AXIS) ||
            (InputHubServiceProvider::clutchWorkingMode == ClutchWorkingMode::CLUTCH) ||
            (InputHubServiceProvider::clutchWorkingMode == ClutchWorkingMode::LAUNCH_CONTROL_MASTER_LEFT) ||
            (InputHubServiceProvider::clutchWorkingMode == ClutchWorkingMode::LAUNCH_CONTROL_MASTER_RIGHT);
        if (isAxisMode)
        {
            // Transform input state into an axis position
            if (input.rawInputBitmap & leftClutchBitmap)
                input.leftAxisValue = CLUTCH_FULL_VALUE;
            else
                input.leftAxisValue = CLUTCH_NONE_VALUE;
            if (input.rawInputBitmap & rightClutchBitmap)
                input.rightAxisValue = CLUTCH_FULL_VALUE;
            else
                input.rightAxisValue = CLUTCH_NONE_VALUE;
            input.rawInputChanges = (input.rawInputChanges & clutchInputMask);
            input.rawInputBitmap = (input.rawInputBitmap & clutchInputMask);
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
        if (rightAxis > CLUTCH_3_4_VALUE)
            clutchAxis = InputHubServiceProvider::bitePoint;
        else
            clutchAxis = CLUTCH_NONE_VALUE;
        if (leftAxis > clutchAxis)
            clutchAxis = leftAxis;
        leftAxis = CLUTCH_NONE_VALUE;
        rightAxis = CLUTCH_NONE_VALUE;
        break;

    case ClutchWorkingMode::LAUNCH_CONTROL_MASTER_RIGHT:
        if (leftAxis > CLUTCH_3_4_VALUE)
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
    uint64_t &rawInputBitmap,
    uint8_t &leftAxis,
    uint8_t &rightAxis,
    bool &isAltRequested)
{
    if (InputHubServiceProvider::altButtonsWorkingMode == AltButtonsWorkingMode::ALT)
    {
        isAltRequested = (rawInputBitmap & altBitmap);
        rawInputBitmap &= ~altBitmap;
    }
    if (InputHubServiceProvider::clutchWorkingMode == ClutchWorkingMode::ALT)
    {
        isAltRequested =
            isAltRequested ||
            (leftAxis >= CLUTCH_DEFAULT_VALUE) ||
            (rightAxis >= CLUTCH_DEFAULT_VALUE) ||
            (rawInputBitmap & leftClutchBitmap) ||
            (rawInputBitmap & rightClutchBitmap);
        leftAxis = CLUTCH_NONE_VALUE;
        rightAxis = CLUTCH_NONE_VALUE;
        rawInputBitmap &= clutchInputMask;
    }
}

//-------------------------------------------------------------------

/**
 * @brief Transform DPAD input into navigational input depending on user preferences
 *
 */
void inputHub_DPAD_filter(
    uint64_t &rawInputBitmap,
    uint8_t &povInput)
{
    povInput = DPAD_CENTERED;
    if (InputHubServiceProvider::dpadWorkingMode == DPadWorkingMode::Navigation)
    {
        // Map directional buttons to POV input as needed
        uint64_t povState = rawInputBitmap & dpadNegMask;

        if (povState)
        {
            uint8_t n = 1;
            while ((povInput == DPAD_CENTERED) && (n < 9))
            {
                if (povState == dpadBitmap[n])
                    povInput = n;
                n++;
            }
        }
        rawInputBitmap = rawInputBitmap & dpadMask;
    }
}

/**
 * @brief Engage or disengage neutral gear
 *
 */
void inputHub_neutralGear_filter(uint64_t &rawInputBitmap)
{
    if (neutralSwitchBitmap)
    {
        bool combinationPressed = ((~rawInputBitmap & neutralCombinationBitmap) == 0ULL);
        if (neutralWasEngaged && ((rawInputBitmap & neutralCombinationBitmap) == 0ULL))
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
                rawInputBitmap |= neutralSwitchBitmap;
        }
    }
}

//-------------------------------------------------------------------

void internals::inputHub::onRawInput(DecouplingEvent &input)
{

    // Decode binary-coded switches when required
    inputHub_decode_bin_coded_switches(input.rawInputBitmap, input.rawInputChanges);

    // Step 1: Execute user commands if any
    if (inputHub_commands_filter(input.rawInputBitmap, input.rawInputChanges))
    {
        internals::hid::reset();
        return;
    }

    // Step 2: digital input <--> analog axes
    inputHub_AxisButton_filter(input);

    // Step 3: bite point calibration
    inputHub_bitePointCalibration_filter(input);

    // Step 4: check if ALT mode is requested
    bool isALTRequested = false;
    inputHub_AltRequest_filter(
        input.rawInputBitmap,
        input.leftAxisValue,
        input.rightAxisValue,
        isALTRequested);

    // Step 5: compute F1-style clutch position
    uint8_t clutchAxis;
    inputHub_combinedAxis_filter(input.leftAxisValue, input.rightAxisValue, clutchAxis);

    // Step 6: compute DPAD input
    uint8_t povInput = DPAD_CENTERED;
    if (!isALTRequested)
        inputHub_DPAD_filter(input.rawInputBitmap, povInput);

    // Step 7: compute neutral gear engagement
    inputHub_neutralGear_filter(input.rawInputBitmap);

    // Step 8: map raw input state into HID button state
    uint64_t inputsLow, inputsHigh;
    internals::inputMap::map(isALTRequested, input.rawInputBitmap, inputsLow, inputsHigh);

    // Step 9: send HID report
    internals::hid::reportInput(
        inputsLow,
        inputsHigh,
        povInput,
        input.leftAxisValue,
        input.rightAxisValue, clutchAxis);
}

//-------------------------------------------------------------------
