/**
 * @file InternalTypes.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-04
 * @brief Types and constants used everywhere for firmware implementation
 *
 * @copyright Licensed under the EUPL
 *
 */

#pragma once

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include <cstdint>
#include <cassert>
#include <array>
#include <vector>
#include <stdexcept>
#include <semaphore>
#include <chrono>
#include <optional>

#if !CD_CI
/// @brief For tesing
#define PRIVATE private
/// @brief For tesing
#define PROTECTED protected
#else
/// @brief For tesing
#define PRIVATE public
/// @brief For tesing
#define PROTECTED public
#endif

//-------------------------------------------------------------------
// Testing
//-------------------------------------------------------------------

/**
 * @brief Fake input specification used for testing.
 *
 */
struct FakeInput
{
    /// @brief Input bitmap
    uint64_t state = 0ULL;
    /// @brief Input bitmask
    uint64_t mask = ~(0ULL);
    /// @brief Left axis position
    uint8_t leftAxis = 0;
    /// @brief Right axis position
    uint8_t rightAxis = 0;
    /// @brief Count of times axis recalibration was asked
    size_t recalibrationRequestCount = 0;

    /**
     * @brief Simulate a button press
     *
     * @param n Input number
     */
    void press(uint8_t n)
    {
        uint64_t bmp = (1ULL << n);
        state = state | bmp;
    }

    /**
     * @brief Simulate a button release
     *
     * @param n Input number
     */
    void release(uint8_t n)
    {
        uint64_t bmp = (1ULL << n);
        state = state & ~bmp;
    }

    /**
     * @brief Simulate no input
     *
     */
    void clear()
    {
        state = 0ULL;
        leftAxis = 0;
        rightAxis = 0;
    }

    FakeInput() {}
    ~FakeInput() {}
};

//-------------------------------------------------------------------
// Input bitmaps
//-------------------------------------------------------------------

// /**
//  * @brief A bit array which assembles the state of every button, being the least significant bit
//  *        the button numbered as 0, and the most significant bit the button numbered as 63.
//  *        A bit set to 1 means the button is pressed, 0 means not pressed.
//  *        May be used to identify a number of buttons, too.
//  */
// typedef uint64_t InputBitmap;

/**
 * @brief Return a mask for a number of consecutive buttons (`count`) starting from `first`.
 *        A mask is a bit array where each bit determines if a button is to be used or not.
 *        1 means **not** used. 0 means in use.
 *        Masks are required to combine the state from multiple input bitmaps.
 *        For example, BITMASK(2,2) returns 0b(...)11110011 which means that
 *        buttons numbered 2 and 3 are in use.
 */
#define BITMASK(count, first) ~(((1ULL << static_cast<uint64_t>(count)) - 1ULL) << static_cast<uint64_t>(first))

/**
 * @brief Return the logical negation of a bit mask
 *
 */
#define NBITMASK(count, first) (((1ULL << static_cast<uint64_t>(count)) - 1ULL) << static_cast<uint64_t>(first))

//-------------------------------------------------------------------
// Device capabilities
//-------------------------------------------------------------------

/**
 * @brief Enumeration of device capabilities
 *
 */
enum class DeviceCapability : uint8_t
{
    /// @brief Has digital clutch paddles (switches)
    CLUTCH_BUTTON = 0,
    /// @brief Has analog clutch paddles (potentiometers)
    CLUTCH_ANALOG = 1,
    /// @brief Has "ALT" buttons
    ALT = 2,
    /// @brief Has a directional pad
    DPAD = 3,
    /// @brief Battery-operated
    BATTERY = 4,
    /// @brief Has battery calibration data
    BATTERY_CALIBRATION_AVAILABLE = 5,
    /// @brief Able to display powertrain telemetry data
    TELEMETRY_POWERTRAIN = 6,
    /// @brief Able to display ECU telemetry data
    TELEMETRY_ECU = 7,
    /// @brief Able to display race control telemetry data
    TELEMETRY_RACE_CONTROL = 8,
    /// @brief Able to display telemetry data for gauges
    TELEMETRY_GAUGES = 9,
    /// @brief Has one or more rotary encoders
    ROTARY_ENCODERS = 10,
    _MAX_VALUE = ROTARY_ENCODERS
};

/**
 * @brief Set of device capabilities
 *
 */
struct DeviceCapabilities
{
public:
    /**
     * @brief Clear or set a device capability
     *
     * @param capability A device capability
     * @param setOrClear True to set, false to clear
     */
    static void setFlag(DeviceCapability capability, bool setOrClear = true)
    {
        if (setOrClear)
            _flags |= (1 << static_cast<uint8_t>(capability));
        else
            _flags &= ~(1 << static_cast<uint8_t>(capability));
    }

    /**
     * @brief Check if a capability is available
     *
     * @param capability  A device capability
     * @return true If available
     * @return false If not available
     */
    static bool hasFlag(DeviceCapability capability)
    {
        return _flags & (1 << static_cast<uint8_t>(capability));
    }

    /**
     * @brief Get all flags as a bitmap
     *
     * @return uint32_t Flags
     */
    static uint16_t getFlags() { return _flags; }

private:
    inline static uint16_t _flags = 0;
};

//-------------------------------------------------------------------
// Clutch
//-------------------------------------------------------------------

/**
 * @brief User-selected working mode of the clutch paddles
 *
 */
enum class ClutchWorkingMode : uint8_t
{
    /// @brief F1-Style clutch. Must be the first in the enum: do not change
    CLUTCH = 0,
    /// @brief Independent axes
    AXIS,
    /// @brief Alternate mode
    ALT,
    /// @brief Regular buttons
    BUTTON,
    /// @brief Launch control (left paddle is master)
    LAUNCH_CONTROL_MASTER_LEFT,
    /// @brief Launch control (right paddle is master)
    LAUNCH_CONTROL_MASTER_RIGHT,
    _MAX_VALUE = LAUNCH_CONTROL_MASTER_RIGHT,
    _DEFAULT_VALUE = CLUTCH
};

/// @brief Value for a fully released clutch
#define CLUTCH_NONE_VALUE 0
/// @brief Value for a fully engaged clutch
#define CLUTCH_FULL_VALUE 254
/// @brief Default bite point value
#define CLUTCH_DEFAULT_VALUE 127
/// @brief Value for 1/4 engaged clutch
#define CLUTCH_1_4_VALUE 64
/// @brief Value for 3/4 engaged clutch
#define CLUTCH_3_4_VALUE 192
/// @brief Invalid clutch value used for masking
#define CLUTCH_INVALID_VALUE 255

//-------------------------------------------------------------------
// ALT BUTTONS
//-------------------------------------------------------------------

/**
 * @brief User-selected working mode of "ALT" buttons
 *
 */
enum class AltButtonsWorkingMode : uint8_t
{
    /// @brief Regular button
    Regular = 0,
    /// @brief Engage or disengage alternate mode
    ALT,
    _MAX_VALUE = ALT,
    _DEFAULT_VALUE = ALT
};

//-------------------------------------------------------------------
// Rotary encoder
//-------------------------------------------------------------------

/**
 * @brief User-selected pulse width multiplier
 *
 */
enum class PulseWidthMultiplier : uint8_t
{
    X1 = 1,
    X2,
    X3,
    X4,
    X5,
    X6,
    _MAX_VALUE = X6,
    _DEFAULT_VALUE = X2
};

//-------------------------------------------------------------------
// DPAD
//-------------------------------------------------------------------

/**
 * @brief User-selected working mode of directional pads
 *
 */
enum class DPadWorkingMode : uint8_t
{
    /// @brief Regular button
    Regular = 0,
    /// @brief Navigation control
    Navigation,
    _MAX_VALUE = Navigation,
    _DEFAULT_VALUE = Navigation
};

//-------------------------------------------------------------------
// Global firmware parameters
//-------------------------------------------------------------------

/**
 * @brief Priority of background tasks
 *
 */
#define INPUT_TASK_PRIORITY (tskIDLE_PRIORITY + 2)

//-------------------------------------------------------------------
// Queues
//-------------------------------------------------------------------

/**
 * @brief Queue for 61 bits
 *
 * @note This is thread-safe as long as there is just one
 *       thread calling enqueue() and just one thread calling
 *       dequeue()
 */
class BitQueue
{
public:
    /**
     * @brief Push a bit into the queue
     *
     * @note In case of overflow, @p value is discarded.
     * @param value True to push 1, false to push 0.
     */
    void enqueue(bool value)
    {
        uint8_t bqTailNext = bqTail;
        incDataPointer(bqTailNext);
        if (bqTailNext != bqHead)
        {
            // Queue not full
            uint64_t aux = (1ULL << bqTail);
            bitsQueue &= (~aux);
            if (value)
                bitsQueue |= aux;
            bqTail = bqTailNext;
        } // else queue full, overflow
    }

    /**
     * @brief Extract a bit from the queue
     *
     * @param[out] value The extracted bit, if any
     * @return true if the queue was not empty, so @p value contains valid data.
     * @return false if the queue was empty, so @p value was not written.
     */
    bool dequeue(bool &value)
    {
        bool isNotEmpty = (bqHead != bqTail);
        if (isNotEmpty)
        {
            uint64_t bitState = (1ULL << bqHead) & bitsQueue;
            value = (bitState != 0ULL);
            incDataPointer(bqHead);
        }
        return isNotEmpty;
    }

    /// @cond

    PRIVATE : uint64_t bitsQueue = 0ULL;
    uint8_t bqHead = 0; // "pointer" (short of) to head
    uint8_t bqTail = 0; // "pointer" to tail

    inline static void incDataPointer(uint8_t &pointer)
    {
        pointer = (pointer + 1) % 64;
    };

    /// @endcond
};

//-------------------------------------------------------------------
// Inputs-InputHub decoupling
//-------------------------------------------------------------------

/**
 * @brief Decoupling event
 *
 */
struct DecouplingEvent
{
    /// @brief Input bitmap
    uint64_t rawInputBitmap;
    /// @brief Bitmap of changes from the previous event
    uint64_t rawInputChanges;
    /// @brief Position of the left axis
    uint8_t leftAxisValue;
    /// @brief Position of the right axis
    uint8_t rightAxisValue;
};

/// @brief Queue size for decoupling events
#define MAX_DECOUPLING_EVENT_COUNT 64

//-------------------------------------------------------------------
// Internal events
//-------------------------------------------------------------------

/**
 * @brief Available internal event types
 *
 */
enum class InternalEventType : uint8_t
{
    /// @brief System startup
    Start,
    /// @brief Sytem shutdown
    Shutdown,
    /// @brief Discovery mode started
    Disconnected,
    /// @brief Device connected to a host computer
    Connected,
    /// @brief The bite point has changed
    BitePoint,
    /// @brief The clutch working mode has changed
    ClutchWorkingMode,
    /// @brief The ALT buttons working mode has changed
    AltButtonsWorkingMode,
    /// @brief The DPAD working mode has changed
    DPadWorkingMode,
    /// @brief User settings were saved
    SettingsSaved,
    /// @brief The pulse width multiplier has changed
    PulseWidthMultiplier,
    /// @brief The system is in a low battery state (repeated at timed intervals)
    LowBattery,
    /// @brief Request to save a user setting
    SaveSetting,
    /// @brief Request to load a user setting
    LoadSetting
};

/**
 * @brief User setting to be stored in flash memory
 *
 * @warning Must be in the range [0,63]
 *
 */
enum class UserSetting : uint8_t
{
    ALL = 0,
    AXIS_CALIBRATION,
    AXIS_POLARITY,
    PULSE_WIDTH,
    SECURITY_LOCK,
    BITE_POINT,
    CLUTCH_WORKING_MODE,
    ALT_WORKING_MODE,
    DPAD_WORKING_MODE,
    INPUT_MAP,
    CUSTOM_HARDWARE_ID,
    BATTERY_AUTO_CALIBRATION,
    BATTERY_CALIBRATION_DATA,
    _MAX_VALUE = BATTERY_CALIBRATION_DATA
};

/**
 * @brief Subscribable events
 *
 * @note Callbacks may be called from any thread
 *
 * @tparam eventType Type of internal event
 * @tparam _Args Callback argument types
 */
template <InternalEventType eventType, typename... _Args>
struct InternalEvent
{
    /// @brief Callback prototype
    typedef void (*Callback)(_Args...);

    /**
     * @brief Subscribe to this event
     *
     * @param callback Event callback
     */
    static void subscribe(Callback callback)
    {
        if (callback != nullptr)
            _callbacks.push_back(callback);
    }

    /**
     * @brief Invoke all subscribed callbacks
     *
     * @param __args Callback arguments
     */
    static void notify(_Args... __args)
    {
        for (auto callback : _callbacks)
            callback(std::forward<_Args>(__args)...);
    }

private:
    inline static std::vector<Callback> _callbacks = {};
};

/**
 * @brief Subscribable events requiring void handlers
 *
 * @tparam eventType Event type
 */
template <InternalEventType eventType>
struct InternalEvent<eventType, void>
{
    /// @brief Callback prototype
    typedef void (*Callback)();

    /**
     * @brief Subscribe to this event
     *
     * @param callback Event callback
     */
    static void subscribe(Callback callback)
    {
        if (callback != nullptr)
            _callbacks.push_back(callback);
    }

    /**
     * @brief Invoke all subscribed callbacks
     *
     */
    static void notify()
    {
        for (auto callback : _callbacks)
            callback();
    }

    /**
     * @brief Clear subscriptions (for testing)
     *
     */
    static void clearSubscriptions()
    {
        _callbacks.clear();
    }

private:
    inline static std::vector<Callback> _callbacks = {};
};

/**
 * @brief System startup
 * @note Notified only once.
 */
typedef InternalEvent<InternalEventType::Start, void> OnStart;

/**
 * @brief The system is about to shutdown.
 * @note Notified only once.
 */
typedef InternalEvent<InternalEventType::Shutdown, void> OnShutdown;

/**
 * @brief No host connection
 * @note Notified when this condition is detected, but not
 *       while it persists.
 */
typedef InternalEvent<InternalEventType::Disconnected, void> OnDisconnected;

/**
 * @brief Host connection
 * @note Notified when this condition is detected.
 */
typedef InternalEvent<InternalEventType::Connected, void> OnConnected;

/**
 * @brief New bite point
 * @note Notified when the clutch's bite point changes
 */
typedef InternalEvent<InternalEventType::BitePoint, uint8_t> OnBitePoint;

/**
 * @brief New clutch working mode
 * @note Notified when that working mode changes
 */
typedef InternalEvent<InternalEventType::ClutchWorkingMode, ClutchWorkingMode> OnClutchWorkingMode;

/**
 * @brief New ALT buttons working mode
 * @note Notified when that working mode changes
 */
typedef InternalEvent<InternalEventType::AltButtonsWorkingMode, AltButtonsWorkingMode> OnAltButtonsWorkingMode;

/**
 * @brief New DPAD working mode
 * @note Notified when that working mode changes
 */
typedef InternalEvent<InternalEventType::DPadWorkingMode, void> OnDPadWorkingMode;

/**
 * @brief Save event
 * @note Notified when a user setting is saved to persistent storage
 */
typedef InternalEvent<InternalEventType::SettingsSaved, void> OnSettingsSaved;

/**
 * @brief New pulse width multiplier
 * @note Notified when changed
 */
typedef InternalEvent<InternalEventType::PulseWidthMultiplier, PulseWidthMultiplier> OnPulseWidthMultiplier;

/**
 * @brief Notified when a low battery condition is detected
 * @note This event is notified at timed intervals while the condition persists.
 *
 */
typedef InternalEvent<InternalEventType::LowBattery, void> OnLowBattery;

/**
 * @brief New battery level (state of charge)
 * @note Notified when the state of charge changes by 1% or more.
 */
typedef InternalEvent<InternalEventType::LowBattery, int> OnBatteryLevel;

/**
 * @brief Request to load a user setting
 * @note Notified when a user setting must be loaded
 */
typedef InternalEvent<InternalEventType::LoadSetting, UserSetting> LoadSetting;

/**
 * @brief Request to save a user setting
 * @note Notified when a user setting must be saved to persistent storage
 */
typedef InternalEvent<InternalEventType::SaveSetting, UserSetting> SaveSetting;

//-------------------------------------------------------------------
// Battery
//-------------------------------------------------------------------

/// @brief Battery level to report when unknown
#define UNKNOWN_BATTERY_LEVEL 66

//-------------------------------------------------------------------
// Automatic shutdown
//-------------------------------------------------------------------

/**
 * @brief Time to wait for connection before power off (in seconds)
 *
 */
#define AUTO_POWER_OFF_DELAY_SECS 60

//-------------------------------------------------------------------
// Simple commands
//-------------------------------------------------------------------

/**
 * @brief Simple commands accepted from a feature HID report
 *
 */
typedef enum
{
    /// @brief Not a command, reserved to avoid mistakes
    CMD_RESERVED = 0,
    /// @brief Recalibrate analog axes (if any)
    CMD_AXIS_RECALIBRATE = 1,
    /// @brief Restart battery auto-calibration
    CMD_BATT_RECALIBRATE = 2,
    /// @brief Reset buttons map to factory defaults
    CMD_RESET_BUTTONS_MAP = 3,
    /// @brief Save all user settings to flash memory immediately
    CMD_SAVE_NOW = 4,
    /// @brief Reverse left axis (if any)
    CMD_REVERSE_LEFT_AXIS = 5,
    /// @brief Reverse right axis (if any)
    CMD_REVERSE_RIGHT_AXIS = 6,
    /// @brief Display all pixels in all pixel groups
    CMD_SHOW_PIXELS = 7,
    /// @brief Turn off all pixels in all groups
    CMD_RESET_PIXELS = 8,
    _MAX_VALUE = 8
} SimpleCommand;

//-------------------------------------------------------------------
// Battery Management
//-------------------------------------------------------------------

struct BatteryStatus
{
public:
    /// @brief Measured battery level in the range from 0% to 100%
    std::optional<uint8_t> stateOfCharge;
    /// @brief  True if the battery is being charged
    std::optional<bool> isCharging;
    /// @brief False if the battery is not connected but there is external power
    std::optional<bool> isBatteryPresent;
    /// @brief True if there is wired power
    std::optional<bool> usingExternalPower;
};
