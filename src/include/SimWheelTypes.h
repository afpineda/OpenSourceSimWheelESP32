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
#include "hal/gpio_types.h" // declares gpio_num_t
#include "esp32-hal.h"

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
    /**
     * @brief Abstract definition of buttons attached to a button matrix
     *
     * @param selectorPin Selector pin (GPIO number or alias)
     * @param inputPin Selector pin (GPIO number or alias)
     * @param number Input number assigned to the button
     * @return ButtonMatrixInputSpec& This object (to chain calls)
     */
    virtual ButtonMatrixInputSpec &inputNumber(
        gpio_num_t selectorPin,
        gpio_num_t inputPin,
        inputNumber_t number) = 0;
};

/**
 * @brief 74HC4051N pin tags for switches
 *        or any other 8-channel multiplexer
 *
 */
enum class mux8_pin_t
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
 *        (8 channels)
 *
 */
class Multiplexers8InputSpec
{
public:
    /**
     * @brief Abstract definition of buttons attached to a multiplexer
     *
     * @param inputPin Input pin (GPIO number or alias)
     * @param pin Pin name in the chip to which the button is connected
     * @param number Input number assigned to the button
     * @return Multiplexers8InputSpec& This object (to chain calls)
     */
    virtual Multiplexers8InputSpec &inputNumber(
        gpio_num_t inputPin,
        mux8_pin_t pin,
        inputNumber_t number) = 0;
};

/**
 * @brief CD74HCx4067 pin tags for switches
 *        or any other 16-channel multiplexer
 *
 */
enum class mux16_pin_t
{
    I0 = 0,
    I1,
    I2,
    I3,
    I4,
    I5,
    I6,
    I7,
    I8,
    I9,
    I10,
    I11,
    I12,
    I13,
    I14,
    I15,
};

/**
 * @brief Specification of input numbers for multiplexed switches
 *        (16 channels)
 *
 */
class Multiplexers16InputSpec
{
public:
    /**
     * @brief Abstract definition of buttons attached to a multiplexer
     *
     * @param inputPin Input pin (GPIO number or alias)
     * @param pin Pin name in the chip to which the button is connected
     * @param number Input number assigned to the button
     * @return Multiplexers16InputSpec& This object (to chain calls)
     */
    virtual Multiplexers16InputSpec &inputNumber(
        gpio_num_t inputPin,
        mux16_pin_t pin,
        inputNumber_t number) = 0;
};

/**
 * @brief ADG732 pin tags for switches
 *        or any other 32-channel multiplexer
 *
 */
enum class mux32_pin_t
{
    S1 = 0,
    S2,
    S3,
    S4,
    S5,
    S6,
    S7,
    S8,
    S9,
    S10,
    S11,
    S12,
    S13,
    S14,
    S15,
    S16,
    S17,
    S18,
    S19,
    S20,
    S21,
    S22,
    S23,
    S24,
    S25,
    S26,
    S27,
    S28,
    S29,
    S30,
    S31,
    S32
};

/**
 * @brief Specification of input numbers for multiplexed switches
 *        (32 channels)
 *
 */
class Multiplexers32InputSpec
{
public:
    /**
     * @brief Abstract definition of buttons attached to a multiplexer
     *
     * @param inputPin Input pin (GPIO number or alias)
     * @param pin Pin name in the chip to which the button is connected
     * @param number Input number assigned to the button
     * @return Multiplexers16InputSpec& This object (to chain calls)
     */
    virtual Multiplexers32InputSpec &inputNumber(
        gpio_num_t inputPin,
        mux32_pin_t pin,
        inputNumber_t number) = 0;
};

/**
 * @brief  74HC165N pin tags for switches
 *
 */
enum class sr8_pin_t
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
    /**
     * @brief Abstract definition of buttons attached to a shift registers chain
     *
     * @param indexInChain Position of a chip in the chain
     * @param pin Pin name in the chip to which the button is connected
     * @param number Input number assigned to the button
     * @return ShiftRegisters8InputSpec& This object (to chain calls)
     */
    virtual ShiftRegisters8InputSpec &inputNumber(
        uint8_t indexInChain,
        sr8_pin_t pin,
        inputNumber_t number) = 0;
};

/**
 * @brief MCP23017 pin tags for switches
 *
 */
enum class MCP23017_pin_t
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
    /**
     * @brief Abstract definition of buttons attached to MCP23017 chip
     *
     * @param pin Pin name in the chip to which the button is connected
     * @param number Input number assigned to the button
     * @return MCP23017InputSpec& This object (to chain calls)
     */
    virtual MCP23017InputSpec &inputNumber(MCP23017_pin_t pin, inputNumber_t number) = 0;
};

/**
 * @brief MCP23017 pin tags for switches
 *
 */
enum class PCF8574_pin_t
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
    /**
     * @brief Abstract definition of buttons attached to PCF8574 chip
     *
     * @param pin Pin name in the chip to which the button is connected
     * @param number Input number assigned to the button
     * @return PCF8574InputSpec& This object (to chain calls)
     */
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
 *
 */
typedef enum
{
    /// F1-Style clutch. Must be the first in the enum: do not change
    CF_CLUTCH = 0,
    /// Independent axes
    CF_AXIS,
    /// "ALT" mode
    CF_ALT,
    /// Regular buttons. Must be the last in the enum: do not change
    CF_BUTTON,
    /// Launch control. Left paddle is master
    CF_LAUNCH_CONTROL_MASTER_LEFT,
    /// Launch control. Right paddle is master
    CF_LAUNCH_CONTROL_MASTER_RIGHT
} clutchFunction_t;

/**
 * @brief Default autosave delay (in microseconds)
 *
 */
#define DEFAULT_AUTOSAVE_us 20 * 1000 * 1000

/**
 * @brief Time to wait for connection before power off (in seconds)
 *
 */
#define AUTO_POWER_OFF_DELAY_SECS 60

/**
 * @brief Supported power latch modes
 *
 */
typedef enum
{
    /// Power on when open drain, power off when low voltage
    POWER_OPEN_DRAIN,
    /// Power on when low voltage, power off when high voltage
    POWER_OFF_HIGH,
    /// Power on when high voltage, power off when low voltage
    POWER_OFF_LOW
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
    /// Has digital clutch paddles (switches)
    CAP_CLUTCH_BUTTON = 0,
    /// Has analog clutch paddles (potentiometers)
    CAP_CLUTCH_ANALOG = 1,
    /// Has "ALT" buttons
    CAP_ALT = 2,
    /// Has a directional pad
    CAP_DPAD = 3,
    /// Battery-operated
    CAP_BATTERY = 4,
    /// Has battery calibration data
    CAP_BATTERY_CALIBRATION_AVAILABLE = 5,
    /// Able to display powertrain telemetry data
    CAP_TELEMETRY_POWERTRAIN = 6,
    /// Able to display ECU telemetry data
    CAP_TELEMETRY_ECU = 7,
    /// Able to display race control telemetry data
    CAP_TELEMETRY_RACE_CONTROL = 8,
    /// Able to display telemetry data for gauges
    CAP_TELEMETRY_GAUGES = 9,
    /// Has one or more rotary encoders
    CAP_ROTARY_ENCODERS = 10
} deviceCapability_t;

/**
 * @brief Simple commands accepted from a feature HID report
 *
 */
typedef enum
{
    /// Not a command, reserved to avoid mistakes
    CMD_RESERVED = 0,
    /// Recalibrate analog axes (if any)
    CMD_AXIS_RECALIBRATE = 1,
    /// Restart battery auto-calibration
    CMD_BATT_RECALIBRATE = 2,
    /// Reset buttons map to factory defaults
    CMD_RESET_BUTTONS_MAP = 3,
    /// Save all user settings to flash memory immediately
    CMD_SAVE_NOW = 4,
    /// Reverse left axis (if any)
    CMD_REVERSE_LEFT_AXIS = 5,
    /// Reverse right axis (if any)
    CMD_REVERSE_RIGHT_AXIS = 6,
    /// Display all pixels in all pixel groups
    CMD_SHOW_PIXELS = 7,
    /// Turn off all pixels in all groups
    CMD_RESET_PIXELS = 8,
    /// Set the pulse width for rotary encoders to defaults
    CMD_ENCODER_PULSE_X1 = 9,
    /// Double the default pulse width of rotary encoders
    CMD_ENCODER_PULSE_X2 = 10,
    /// Triple the default pulse width of rotary encoders
    CMD_ENCODER_PULSE_X3 = 11
} simpleCommands_t;

/**
 * @brief Telemetry data
 *
 */
typedef struct
{
    /**
     * @brief Identifies a telemetry frame. For internal use. Do not overwrite.
     *
     */
    uint32_t frameID;
    /**
     * @brief Powertrain telemetry
     *
     */
    struct
    {
        /// Display character for current gear
        char gear = ' ';
        /// Revolutions per minute
        uint16_t rpm = 0;
        /// Percentage of RPM
        uint8_t rpmPercent = 0;
        /// Non-zero at maximum torque
        uint8_t shiftLight1 = 0;
        /// Non-zero at maximum power
        uint8_t shiftLight2 = 0;
        /// True at maximum RPM
        bool revLimiter = false;
        /// True if the engine is running
        bool engineStarted = false;
        /// Speed in user-defined units (Kph or Mph)
        uint16_t speed = 0;
    } powertrain;
    /**
     * @brief ECU telemetry
     *
     */
    struct
    {
        /// ABS is engaged
        bool absEngaged = false;
        /// TC is engaged
        bool tcEngaged = false;
        /// DRS is engaged
        bool drsEngaged = false;
        /// The pit limiter is engaged
        bool pitLimiter = false;
        /// True when fuel is low
        bool lowFuelAlert = false;
        /// Driver-selected ABS mode
        uint8_t absLevel = 0;
        /// Driver-selected TC mode
        uint8_t tcLevel = 0;
        /// Driver-selected TC Cut mode (NOTE: currently not available)
        uint8_t tcCut = 0;
        /// Percentage of brake bias towards front wheels
        uint8_t brakeBias = 0;
    } ecu;
    /**
     * @brief Race control telemetry
     *
     */
    struct
    {
        bool blackFlag = false;
        bool blueFlag = false;
        bool checkeredFlag = false;
        bool greenFlag = false;
        bool orangeFlag = false;
        bool whiteFlag = false;
        bool yellowFlag = false;
        /// Remaining laps to the end of the session. Zero if does not apply.
        uint16_t remainingLaps = 0;
        /// Remaining time to the end of the session, in minutes. Zero if does not apply.
        uint16_t remainingMinutes = 0;
    } raceControl;
    /**
     * @brief Gauges telemetry
     *
     */
    struct
    {
        /// Percentage of turbo pressure
        uint8_t relativeTurboPressure = 0;
        /// Turbo pressure in bars
        float absoluteTurboPressure = 0.0;
        /// Water temperature in user-defined units (Celsius or Fahrenheit)
        uint16_t waterTemperature = 0;
        /// Oil pressure in bars
        float oilPressure = 0.0;
        /// Oil temperature in user-defined units (Celsius or Fahrenheit)
        uint16_t oilTemperature = 0;
        /// Percentage of remaining fuel
        uint8_t relativeRemainingFuel = 0;
        /// Remaining fuel in user-defined units (litres or gallons)
        uint16_t absoluteRemainingFuel = 0;
    } gauges;
} telemetryData_t;

/**
 * @brief Abstract interface for notifications and telemetry display.
 *
 */
class AbstractUserInterface
{
protected:
    /**
     * @brief Simple timer.
     *
     * @note To be used in AbstractUserInterface::serveSingleFrame().
     *       Initialize the timer variable to zero.
     *
     * @param timerVariable Timer variable.
     * @param elapsedTimeMs Time elapsed since last call in miliseconds.
     * @param timeLimitMs Expiration time in miliseconds.
     * @return uint32_t The count of times the timer has expired in the elapsed time.
     */
    uint32_t frameTimer(
        uint32_t &timerVariable,
        uint32_t elapsedTimeMs,
        uint32_t timeLimitMs)
    {
        timerVariable += elapsedTimeMs;
        uint32_t result = timerVariable / timeLimitMs;
        timerVariable %= timeLimitMs;
        return result;
    };

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
     * @brief Called just once after initialization.
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
     * @param elapsedMs Elapsed milliseconds since last call.
     *
     * @note Called at timed intervals when no notifications are pending.
     *       Not called at all if frames per second is set to 0.
     *       See notify::begin(). Do not assume perfect timing.
     *
     * @note Must not enter an infinite loop. Must return as soon as possible.
     *
     * @note Called in a low priority thread.
     */
    virtual void serveSingleFrame(uint32_t elapsedMs) {};

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
     * @brief Cut power to the UI hardware.
     *
     * @note This is a command, not a notification. Not reversible.
     *
     */
    virtual void shutdown() {};
};

/**
 * @brief Array of user interface implementations.
 *
 */
typedef std::vector<AbstractUserInterface *> notificationImplementorsArray_t;

/**
 * @brief Available RGB LED groups for pixel control
 *
 */
typedef enum
{
    /// Telemetry leds group
    GRP_TELEMETRY = 0,
    /// Buttons lighting group
    GRP_BUTTONS,
    /// Individual leds group
    GRP_INDIVIDUAL
} pixelGroup_t;

/**
 * @brief Pixel driver
 *
 */
typedef enum
{
    /// WS2811 driver
    WS2811,
    /// WS2812 family
    WS2812,
    /// WS2815 family
    WS2815,
    /// SK6812 driver
    SK6812,
    /// UCS1903 driver
    UCS1903
} pixel_driver_t;

/**
 * @brief Byte order of pixel data
 *
 */
typedef enum
{
    /// Auto-detect based on pixel driver
    AUTO,
    /// Red-green-blue
    RGB,
    /// Red-blue-green
    RBG,
    /// Green-red-blue
    GRB,
    /// Green-blue-red
    GBR,
    /// Blue-red-green
    BRG,
    /// Blue-green-red
    BGR
} pixel_format_t;

/**
 * @brief Notifications using the pixel control interface
 *
 */
class PixelControlNotification : public AbstractUserInterface
{
private:
    virtual void onStart() override;
    virtual void onBitePoint() override;
    virtual void onConnected() override;
    virtual void onBLEdiscovering() override;
    virtual void onLowBattery() override;

public:
    /**
     * @brief Called just once after initialization.
     *
     * @note Called in a low priority thread.
     */
    virtual void pixelControl_OnStart();

    /**
     * @brief Notify a change in current bite point.
     *
     * @note Read userSetting::bitePoint to know the last value
     * @note Called in a low priority thread.
     */
    virtual void pixelControl_OnBitePoint();

    /**
     * @brief Notify device is connected.
     *
     * @note Called in a low priority thread.
     */
    virtual void pixelControl_OnConnected();

    /**
     * @brief Notify device is in discovery mode.
     *
     * @note Called in a low priority thread.
     *
     * @note Not called in the USB implementation.
     */
    virtual void pixelControl_OnBLEdiscovering();

    /**
     * @brief Notify low battery.
     *
     * @note Called in a low priority thread.
     * @note Called at timed intervals while the low battery
     *       condition persists.
     */
    virtual void pixelControl_OnLowBattery();
};

#endif