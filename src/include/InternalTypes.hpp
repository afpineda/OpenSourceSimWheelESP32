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
#include <vector>
#include <optional>
#include "SimWheelTypes.hpp" // For uint128_t

#if !CD_CI
/// @brief For testing
#define PRIVATE private
/// @brief For testing
#define PROTECTED protected
#else
/// @brief For testing
#define PRIVATE public
/// @brief For testing
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
    uint128_t state{};
    /// @brief Input mask
    uint128_t mask{};
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
        state.set_bit(n, true);
    }

    /**
     * @brief Simulate a button release
     *
     * @param n Input number
     */
    void release(uint8_t n)
    {
        state.set_bit(n, false);
    }

    /**
     * @brief Simulate no input
     *
     */
    void clear()
    {
        state = {};
        leftAxis = 0;
        rightAxis = 0;
    }
};

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
    uint128_t rawInputBitmap{};
    /// @brief Position of the left axis
    uint8_t leftAxisValue{CLUTCH_NONE_VALUE};
    /// @brief Position of the right axis
    uint8_t rightAxisValue{CLUTCH_NONE_VALUE};
};

/// @brief Queue size for decoupling events
#define MAX_DECOUPLING_EVENT_COUNT 64

//-------------------------------------------------------------------
// Battery Management
//-------------------------------------------------------------------

/// @brief Battery status
struct BatteryStatus
{
public:
    /// @brief Measured battery level in the range from 0% to 100%
    std::optional<uint8_t> stateOfCharge{};
    /// @brief True if the battery is being charged
    std::optional<bool> isCharging{};
    /// @brief False if the battery is not connected
    std::optional<bool> isBatteryPresent{};
    /// @brief True if there is wired power
    std::optional<bool> usingExternalPower{};

    /**
     * @brief Compare for equality
     *
     * @param other Another battery status
     * @return true If equal
     * @return false If non-equal
     */
    constexpr bool operator==(const BatteryStatus &other) const noexcept
    {
        return (stateOfCharge == other.stateOfCharge) &&
               (isCharging == other.isCharging) &&
               (isBatteryPresent == other.isBatteryPresent) &&
               (usingExternalPower == other.usingExternalPower);
    }

    /**
     * @brief Reset to unknown
     *
     */
    inline void reset() noexcept
    {
        stateOfCharge.reset();
        isCharging.reset();
        isBatteryPresent.reset();
        usingExternalPower.reset();
    }
};

#pragma pack(push, 1)

/// @brief Data format for the Battery Level Status characteristic (packed)
/// @note Initialized for non-battery-operated firmwares
struct BatteryStatusChrData
{
    /// @brief Flags: id field present
    unsigned int flag_id_present : 1 = 1;
    /// @brief Flags: battery level field present
    unsigned int flag_battery_level_present : 1 = 1;
    /// @brief Flags: additional status field present
    unsigned int flag_additional_status_present : 1 = 1;
    /// @brief Flags: reserved for future use
    unsigned int flag_reserved : 5 = 0;

    // End of byte 0

    /// @brief Power state: is battery present
    unsigned int ps_battery_present : 1 = 0;
    /// @brief Power state: is wired external power present
    unsigned int ps_wired_ext_power : 2 = 1;
    /// @brief Power state: is wireless external power present
    unsigned int ps_wireless_ext_power : 2 = 0;
    /// @brief Power state: battery charging status
    unsigned int ps_battery_charge_state : 2 = 0;
    /// @brief Power state: summarized state of charge
    unsigned int ps_battery_charge_level : 2 = 1;
    /// @brief Power state: charging type
    unsigned int ps_charging_type : 3 = 0;
    /// @brief Power state: charging fault reason
    unsigned int ps_fault_reason : 3 = 0;
    /// @brief Power state: reserved for future use
    unsigned int ps_reserved : 1 = 0;

    // End of bytes 1-2

    /// @brief Field: Battery identifier
    uint16_t id = 0x106;

    // End of bytes 3-4

    /// @brief Field: Battery level
    uint8_t battery_level = 100;

    // End of byte 5

    /// @brief Additional status: service required
    unsigned int as_service_required : 1 = 0;
    /// @brief Additional status: battery fault status
    unsigned int as_battery_fault : 2 = 0;
    /// @brief Additional status: reserved for future use
    unsigned int as_reserved : 5 = 0;

    // End of byte 6
};

// For unknown reasons, the x86 compiler ignores the pragma pack directive
#if !CD_CI
static_assert(
    sizeof(BatteryStatusChrData) == 7,
    "Wrong size of BatteryStatusChrData (check struct packaging)");
#endif

#pragma pack(pop)

//-------------------------------------------------------------------
// Internal events
//-------------------------------------------------------------------

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

//------------------------------------------------------------------------------

/**
 * @brief Publish-subscribe event (forever subscribed)
 *
 * @warning Not thread safe. All subscribers must subscribe in the same thread.
 *          Subscription must not take place while the event is dispatching
 *          in another thread.
 *
 * @author Ángel Fernández Pineda. Madrid. Spain. 2026.
 *         https://github.com/afpineda/cpp-publish-subscribe-observable
 *
 * @copyright EUPL 1.2 License
 *
 * @tparam Args Callback argument types
 */
template <class... Args>
class static_event
{
public:
    /// @brief This type
    using type = static_event<Args...>;
    /// @brief Callback type for subscribers
    using callback_type = typename ::std::add_pointer<void(Args...)>::type;

    /**
     * @brief Subscribe forever
     *
     * @param callback Callback function to be called on event dispatch
     */
    void subscribe(callback_type callback) noexcept
    {
        if (!callback)
            return;
        _subscriptions.push_back(callback);
    }

    /**
     * @brief Clear all subscriptions
     *
     * @warning To be used exclusively in test units.
     */
    void clear() noexcept
    {
        _subscriptions.clear();
    }

    /**
     * @brief Dispatch event to all subscribed callbacks
     *
     * @param args Event data
     */
    void operator()(const Args &...args)
    {
        for (const auto &entry : _subscriptions)
        {
            entry(args...);
        }
    }

    /**
     * @brief Dispatch event to all subscribed callbacks (const)
     *
     * @param args Event data
     */
    void operator()(const Args &...args) const
    {
        for (const auto &entry : _subscriptions)
        {
            entry(args...);
        }
    }

    /**
     * @brief Get the number of subscribed callbacks
     *
     * @return ::std::size_t Count of subscribed callbacks
     */
    ::std::size_t subscribed()
    {
        return _subscriptions.size();
    }

    /**
     * @brief Move-assignment
     *
     * @param source Rvalue
     * @return type& Reference to this instance
     */
    type &operator=(type &&source) noexcept
    {
        _subscriptions.swap(source._subscriptions);
        return *this;
    }

    /**
     * @brief Copy-assignment
     *
     * @param source Instance to be copied
     * @return type& Reference to this instance
     */
    type &operator=(const type &source) noexcept
    {
        _subscriptions = source._subscriptions;
        return *this;
    }

    /**
     * @brief Default constructor
     *
     */
    constexpr static_event() noexcept = default;

    /**
     * @brief Copy constructor
     *
     * @param source Instance to be copied
     */
    static_event(const type &source)
    {
        _subscriptions = source._subscriptions;
    }

    /**
     * @brief Move constructor
     *
     * @param source Rvalue
     */
    static_event(type &&source)
    {
        _subscriptions.swap(source._subscriptions);
    }

    /**
     * @brief Equality operator
     *
     * @note Mostly for testing purposes.
     *
     * @param other Instance to compare to
     * @return true If equal
     * @return false If not equal
     */
    constexpr bool operator==(const type &other) const noexcept
    {
        return (_subscriptions == other._subscriptions);
    }

private:
    /// @brief List of subscription entries
    ::std::vector<callback_type> _subscriptions{};
};

/**
 * @brief System startup
 * @note Notified only once.
 */
inline static_event<> OnStart;

/**
 * @brief The system is about to shutdown.
 * @note Notified only once.
 */
inline static_event<> OnShutdown;

/**
 * @brief No host connection
 * @note Notified when this condition is detected, but not
 *       while it persists.
 */
inline static_event<> OnDisconnected;

/**
 * @brief Host connection
 * @note Notified when this condition is detected.
 */
inline static_event<> OnConnected;

/**
 * @brief New bite point
 * @note Notified when the clutch's bite point changes
 */
inline static_event<uint8_t> OnBitePoint;

/**
 * @brief New clutch working mode
 * @note Notified when that working mode changes
 */
inline static_event<ClutchWorkingMode> OnClutchWorkingMode;

/**
 * @brief New ALT buttons working mode
 * @note Notified when that working mode changes
 */
inline static_event<AltButtonsWorkingMode> OnAltButtonsWorkingMode;

/**
 * @brief New DPAD working mode
 * @note Notified when that working mode changes
 */
inline static_event<> OnDPadWorkingMode;

/**
 * @brief Save event
 * @note Notified when a user setting is saved to persistent storage
 */
inline static_event<> OnSettingsSaved;

/**
 * @brief New pulse width multiplier
 * @note Notified when changed
 */
inline static_event<PulseWidthMultiplier> OnPulseWidthMultiplier;

/**
 * @brief Notified when a low battery condition is detected
 * @note This event is notified at timed intervals while the condition persists.
 *
 */
inline static_event<> OnLowBattery;

/**
 * @brief New battery level (state of charge) or battery status
 * @note Notified when the state of charge changes by 1% or more.
 */
inline static_event<const BatteryStatus &> OnBatteryStatus;

/**
 * @brief Request to load a user setting
 * @note Notified when a user setting must be loaded
 */
inline static_event<UserSetting> LoadSetting;

/**
 * @brief Request to save a user setting
 * @note Notified when a user setting must be saved to persistent storage
 */
inline static_event<UserSetting> SaveSetting;

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
