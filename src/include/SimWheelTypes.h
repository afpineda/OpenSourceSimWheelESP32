/**
 * @file SimWheelTypes.h
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Types and constants used everywhere
 *
 * @note Some types are renamed just for portability
 *
 * @copyright Licensed under the EUPL
 *
 */

#ifndef __SIM_WHEEL_TYPES_H__
#define __SIM_WHEEL_TYPES_H__

#include <stdint.h>
#include <vector>
#include <string>
#include "esp32-hal.h" // declares gpio_num_t

/**
 * @brief A bit array which assembles the state of every button, being the least significant bit
 *        the button numbered as 0, and the most significant bit the button numbered as 63.
 *        A bit set to 1 means the button is pressed, 0 means not pressed.
 *        May be used to identify a number of buttons, too.
 */
typedef uint64_t inputBitmap_t;

/**
 * @brief A button number, starting from zero.
 *
 */
typedef uint8_t inputNumber_t;

#define UNSPECIFIED_INPUT_NUMBER 0xFF // Input number is not required or is implicit or is unknown
#define MAX_INPUT_NUMBER 63           // Maximum allowed input number, including itself
#define MAX_USER_INPUT_NUMBER 127     // Maximum allowed input number, including itself, in user-defined button maps

// Well-known input numbers for PC game controllers
#define JOY_A 0
#define JOY_B 1
#define JOY_X 2
#define JOY_Y 3
#define JOY_LB 4
#define JOY_RB 5
#define JOY_LSHIFT_PADDLE 4
#define JOY_RSHIFT_PADDLE 5
#define JOY_BACK 6
#define JOY_START 7
#define JOY_LTHUMBSTICK_CLICK 8
#define JOY_RTHUMBSTICK_CLICK 9

/**
 * @brief Array of GPIO pins
 *
 */
typedef std::vector<gpio_num_t> gpio_num_array_t;

/**
 * @brief Specification of input numbers for a button matrix
 *
 */
class ButtonMatrixInputSpec
{
public:
    virtual ButtonMatrixInputSpec &inputNumber(
        gpio_num_t selectorPin,
        gpio_num_t inputPin,
        inputNumber_t number) = 0;
};

/**
 * @brief 74HC4051N pin tags for switches
 *
 */
typedef enum class mux8_pin_t
{
    A0 = 0,
    A1,
    A2,
    A3,
    A4,
    A5,
    A6,
    A7
};

/**
 * @brief Specification of input numbers for multiplexed switches
 *
 */
class Multiplexers8InputSpec
{
public:
    virtual Multiplexers8InputSpec &inputNumber(
        gpio_num_t inputPin,
        mux8_pin_t pin,
        inputNumber_t number) = 0;
};

/**
 * @brief  74HC165N pin tags for switches
 *
 */
typedef enum class sr8_pin_t
{
    H = 0,
    G,
    F,
    E,
    D,
    C,
    B,
    A,
    SER
};

/**
 * @brief Specification of input numbers for shift registers
 *
 */
class ShiftRegisters8InputSpec
{
public:
    virtual ShiftRegisters8InputSpec &inputNumber(
        uint8_t indexInChain,
        sr8_pin_t pin,
        inputNumber_t number) = 0;
};

/**
 * @brief MCP23017 pin tags for switches
 *
 */
typedef enum class MCP23017_pin_t
{
    GPA0 = 0,
    GPA1,
    GPA2,
    GPA3,
    GPA4,
    GPA5,
    GPA6,
    GPA7,
    GPB0,
    GPB1,
    GPB2,
    GPB3,
    GPB4,
    GPB5,
    GPB6,
    GPB7
};

/**
 * @brief Specification of input numbers for MCP23017 GPIO expanders
 *
 */
class MCP23017InputSpec
{
public:
    virtual MCP23017InputSpec &inputNumber(MCP23017_pin_t pin, inputNumber_t number) = 0;
};

/**
 * @brief MCP23017 pin tags for switches
 *
 */
typedef enum class PCF8574_pin_t
{
    P0 = 0,
    P1,
    P2,
    P3,
    P4,
    P5,
    P6,
    P7
};

/**
 * @brief Specification of input numbers for PCF8574 GPIO expanders
 *
 */
class PCF8574InputSpec
{
public:
    virtual PCF8574InputSpec &inputNumber(PCF8574_pin_t pin, inputNumber_t number) = 0;
};

/**
 * @brief The value of a joystick's axis
 *
 * @note DO NOT CHANGE. hidImplementation and userSettings depends on it.
 */
typedef uint8_t clutchValue_t;

#define CLUTCH_NONE_VALUE 0
#define CLUTCH_FULL_VALUE 254
#define CLUTCH_DEFAULT_VALUE 127
#define CLUTCH_1_4_VALUE 64
#define CLUTCH_3_4_VALUE 192
#define CLUTCH_INVALID_VALUE 255

/**
 * @brief Value read from an ADC pin
 *
 */
typedef uint16_t analogReading_t;

/**
 * @brief A combination of input numbers
 *
 */
typedef std::vector<inputNumber_t> inputNumberCombination_t;

/**
 * @brief Transform a button number to an input bitmap.
 *        For example, BITMAP(2) returns 0b00000100
 *
 */
#define BITMAP(n) (1ULL << static_cast<inputBitmap_t>(n))

/**
 * @brief Return a mask for a number of consecutive buttons (`count`) starting from `first`.
 *        A mask is a bit array where each bit determines if a button is to be used or not.
 *        1 means **not** used. 0 means in use.
 *        Masks are required to combine the state from multiple input bitmaps.
 *        For example, BITMASK(2,2) returns 0b(...)11110011 which means that
 *        buttons numbered 2 and 3 are in use.
 */
#define BITMASK(count, first) ~(((1ULL << static_cast<inputBitmap_t>(count)) - 1ULL) << static_cast<inputBitmap_t>(first))

/**
 * @brief Return the logical negation of a bit mask
 *
 */
#define NBITMASK(count, first) (((1ULL << static_cast<inputBitmap_t>(count)) - 1ULL) << static_cast<inputBitmap_t>(first))

/**
 * @brief Debounce time for buttons, in system ticks
 *
 */
#define DEBOUNCE_TICKS pdMS_TO_TICKS(30)

/**
 * @brief Priority of background tasks
 *
 */
#define INPUT_TASK_PRIORITY (tskIDLE_PRIORITY + 2)

/**
 * @brief User-selected function of the clutch paddles
 *        CF_CLUTCH = F1-style clutch
 *        CF_ALT = Alternate button numbers (like ALT key in PC keyboards)
 *        CF_BUTTON = a button like any other
 */
typedef enum
{
    CF_CLUTCH = 0, // F1-Style clutch. Must be the first in the enum: do not change
    CF_AXIS,       // Independent axes
    CF_ALT,        // "ALT" mode
    CF_BUTTON      // Regular buttons. Must be the last in the enum: do not change
} clutchFunction_t;

/**
 * @brief Default autosave delay (in microseconds)
 *
 */
#define DEFAULT_AUTOSAVE_us 20 * 1000 * 1000

// Time to wait for connection before power off (in seconds)
#define AUTO_POWER_OFF_DELAY_SECS 60

/**
 * @brief Supported power latch modes
 *
 */
typedef enum
{
    POWER_OPEN_DRAIN, // Power on when open drain, power off when low voltage
    POWER_OFF_HIGH,   // Power on when low voltage, power off when high voltage
    POWER_OFF_LOW,    // Power on when high voltage, power off when low voltage
} powerLatchMode_t;

/**
 * @brief Battery level to report when unknown (percentage)
 *
 */
#define UNKNOWN_BATTERY_LEVEL 66

/**
 * @brief Enumeration of device capabilities
 *
 */
typedef enum
{
    CAP_CLUTCH_BUTTON = 0,                 // has digital clutch paddles (switches)
    CAP_CLUTCH_ANALOG = 1,                 // has analog clutch paddles (potentiometers)
    CAP_ALT = 2,                           // has "ALT" buttons
    CAP_DPAD = 3,                          // has a directional pad
    CAP_BATTERY = 4,                       // battery-operated
    CAP_BATTERY_CALIBRATION_AVAILABLE = 5, // has battery calibration data
    CAP_USER_INTERFACE = 6,                // has an user interface
    CAP_TELEMETRY_POWERTRAIN = 7,          /// Able to display powertrain telemetry data
    CAP_TELEMETRY_ECU = 8,                 /// Able to display ECU telemetry data
    CAP_TELEMETRY_RACE_CONTROL = 9,        /// Able to display race control telemetry data
    CAP_TELEMETRY_GAUGES = 10              /// Able to display telemetry data for gauges
} deviceCapability_t;

/**
 * @brief Simple commands accepted from a feature HID report
 *
 */
typedef enum
{
    CMD_RESERVED = 0,          // Not a command, reserved to avoid mistakes
    CMD_AXIS_RECALIBRATE = 1,  // Recalibrate analog axes (if any)
    CMD_BATT_RECALIBRATE = 2,  // Restart battery auto-calibration
    CMD_RESET_BUTTONS_MAP = 3, // Reset buttons map to factory defaults
    CMD_SAVE_NOW = 4,          // Save all user settings to flash memory immediately
    CMD_REVERSE_LEFT_AXIS = 5, // Reverse left axis (if any)
    CMD_REVERSE_RIGHT_AXIS = 6 // Reverse right axis (if any)
} simpleCommands_t;

/**
 * @brief Telemetry data
 *
 */
typedef struct
{
    uint64_t frameID; /// For internal use. Do not overwrite.
    struct
    {
        char gear = ' ';          /// Display character for current gear
        uint16_t rpm = 0;         /// Revolutions per minute
        uint8_t rpmPercent = 0;   /// Percentage of RPM
        bool shiftLight1 = false; /// True at maximum torque
        bool shiftLight2 = false; /// True at maximum power
        uint16_t speed = 0;       /// Speed in user-defined units (Kph or Mph)
    } powertrain;
    struct
    {
        bool absEngaged = false;   /// ABS is engaged
        bool tcEngaged = false;    /// TC is engaged
        bool drsEngaged = false;   /// DRS is engaged
        bool pitLimiter = false;   /// The pit limiter is engaged
        bool lowFuelAlert = false; /// True when fuel is low
        uint8_t absLevel = 0;      /// Driver-selected ABS mode
        uint8_t tcLevel = 0;       /// Driver-selected TC mode
        uint8_t tcCut = 0;         /// Driver-selected TC Cut mode
        uint8_t brakeBias = 0;     /// Percentage of brake bias towards front wheels
    } ecu;
    struct
    {
        bool blackFlag = false;
        bool blueFlag = false;
        bool checkeredFlag = false;
        bool greenFlag = false;
        bool orangeFlag = false;
        bool whiteFlag = false;
        bool yellowFlag = false;
        uint16_t remainingLaps = 0;
        char remainingTime[9] = "--:--:--"; /// Remaining session time in HH:MM:SS
    } raceControl;
    struct
    {
        uint8_t relativeTurboPressure = 0;      /// Percentage of turbo pressure
        float absoluteTurboPressure = 0.0;      /// Turbo pressure in bars
        float waterTemperature = 0.0;           /// Water temperature in user-defined units (Celsius or Kelvin)
        float oilPressure = 0.0;                /// Oil pressure in bars
        float oilTemperature = 0.0;             /// Oil temperature in user-defined units (Celsius or Kelvin)
        uint8_t relativeRemainingFuel = 0;      /// Percentage of remaining fuel
        float absoluteRemainingFuel = 0;        /// Remaining fuel in user-defined units (litres or gallons)
        uint16_t remainingFuelLaps = 0;         /// Laps to run out of fuel
        char remainingFuelTime[9] = "--:--:--"; /// Time to run out of fuel in HH:MM:SS
    } gauges;
} telemetryData_t;

/**
 * @brief Abstract interface for notifications and telemetry display.
 *
 */
class AbstractUserInterface
{
public:
    /// Index of this implementor. Do not overwrite.
    uint8_t index;
    /// Set to true to receive and use powertrain telemetry data
    bool requiresPowertrainTelemetry = false;
    /// Set to true to receive and use ECU telemetry data
    bool requiresECUTelemetry = false;
    /// Set to true to receive and use race control telemetry data
    bool requiresRaceControlTelemetry = false;
    /// Set to true to receive and use telemetry data for gauges
    bool requiresGaugeTelemetry = false;

public:
    /**
     * @brief Called just once at initialization.
     *
     * @note Called in a low priority thread.
     */
    virtual void onStart() {};

    /**
     * @brief Notify new telemetry data
     *
     * @param pTelemetryData Pointer to telemetry data. Can be null.
     *                       Safe to store for later use.
     *
     * @note For this method to get called,
     *       user code must meet the following requirements:
     *       - Frames-per-second must be set to non-zero. See notify::begin().
     *       - At instance creation, one of the requiresXXXTelemetry attributes
     *         must be set to true.
     *
     * @note Always called just before serveSingleFrame().
     *       This method may draw a display frame into a buffer,
     *       while serveSingleFrame() just displays that frame buffer.
     *
     * @note @p pTelemetryData is null when no telemetry data has been
     *       received in the previous two seconds. May be used
     *       to turn the display off.
     *
     * @note Must not enter an infinite loop. Must return as soon as possible.
     *
     * @note Called in a low priority thread.
     */
    virtual void onTelemetryData(const telemetryData_t *pTelemetryData) {};

    /**
     * @brief Draw a single frame.
     *
     * @note Called at timed intervals when no notifications are pending.
     *       Not called at all if frames per second is set to 0.
     *       See notify::begin().
     *
     * @note Must not enter an infinite loop. Must return as soon as possible.
     *
     * @note Called in a low priority thread.
     */
    virtual void serveSingleFrame() {};

    /**
     * @brief Notify a change in current bite point.
     *
     * @note Read userSetting::bitePoint to know the last value
     * @note Called in a low priority thread.
     */
    virtual void onBitePoint() {};

    /**
     * @brief Notify device is connected.
     *
     * @note Called in a low priority thread.
     */
    virtual void onConnected() {};

    /**
     * @brief Notify device is in discovery mode.
     *
     * @note Called in a low priority thread.
     *
     * @note Not called in the USB implementation.
     */
    virtual void onBLEdiscovering() {};

    /**
     * @brief Notify low battery.
     *
     * @note Called in a low priority thread.
     * @note Called at timed intervals while the low battery
     *       condition persists.
     */
    virtual void onLowBattery() {};

    /**
     * @brief Select the next page in the user interface when available.
     *
     * @note Not thread-safe. May be called while events are being processed
     *       or serveSingleFrame() is running.
     */
    virtual void selectNextPage() {};

    /**
     * @brief Select the previous page in the user interface when available.
     *
     * @note Not thread-safe. May be called while events are being processed
     *       or serveSingleFrame() is running.
     */
    virtual void selectPreviousPage() {};
};

/**
 * @brief Array of user interface implementations.
 *
 */
typedef std::vector<AbstractUserInterface *> notificationImplementorsArray_t;

#endif