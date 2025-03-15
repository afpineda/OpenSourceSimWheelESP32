/**
 * @file SimWheelTypes.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-02
 * @brief Types and constants required for custom firmware setup
 *
 * @copyright Licensed under the EUPL
 *
 */

#pragma once

//-------------------------------------------------------------------
// Global macros
//-------------------------------------------------------------------

#if CD_CI

/// @cond

#define GPIO_IS_VALID_GPIO(pin) (pin < 100)
#define TEST_NO_OUTPUT_GPIO 80
#define GPIO_IS_VALID_OUTPUT_GPIO(pin) (pin < TEST_NO_OUTPUT_GPIO)
#define TEST_RTC_GPIO1 40
#define TEST_RTC_GPIO2 49
#define GPIO_IS_VALID_RTC_GPIO(pin) ((pin >= TEST_RTC_GPIO1) && (pin <= TEST_RTC_GPIO2))
#define TEST_RESERVED_GPIO 50

/// @endcond

#else

/// @brief Validation of RTC GPIO pins
#define GPIO_IS_VALID_RTC_GPIO(pin) rtc_gpio_is_valid_gpio(static_cast<gpio_num_t>(pin))

#endif

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include <cstdint>
#include <string>
#include <stdexcept>
#include <vector>
#include <initializer_list>
#include <map>
#include <algorithm>

#if !CD_CI
#include "driver/rtc_io.h"   // For rtc_gpio_is_valid_gpio()
#include "hal/gpio_types.h"  // For gpio_num_t
#include "driver/gpio.h"     // For GPIO_IS_VALID... and others
#include "esp32-hal-psram.h" // For psramFound()
#endif

//-------------------------------------------------------------------
// Exceptions
//-------------------------------------------------------------------

/**
 * @brief Exception for invalid input numbers
 *
 * @note Valid input numbers are in the range [0,63]
 */
class invalid_input_number : public std::runtime_error
{
public:
    /**
     * @brief Construct a new invalid_input_number exception
     *
     * @param value Offending input number
     */
    invalid_input_number(uint8_t value)
        : std::runtime_error(
              "The input number " +
              std::to_string(value) +
              " is out of range [0,63]") {}
    /**
     * @brief Construct a new invalid input number object
     *        for unspecified input numbers
     *
     */
    invalid_input_number()
        : std::runtime_error(
              "Trying to use an unspecified input number.") {}

    virtual ~invalid_input_number() noexcept {}
};

/**
 * @brief Exception for invalid GPIO pin numbers
 *
 */
class gpio_error : public std::runtime_error
{
public:
    /**
     * @brief Construct a new gpio_error exception
     *
     *
     * @param value GPIO pin number
     * @param reason Message describing the error
     */
    gpio_error(uint8_t value, const std::string &reason)
        : std::runtime_error(
              "Invalid GPIO number " +
              std::to_string(value) +
              ". Reason: " +
              reason) {}
};

/**
 * @brief Exception for empty input number specifications
 *
 */
class empty_input_number_set : public std::runtime_error
{
public:
    /**
     * @brief Construct a new empty_input_number_set exception
     *
     * @param hardware Hardware type
     */
    empty_input_number_set(const std::string &hardware)
        : std::runtime_error(
              "No input numbers were given to: " + hardware) {}

    virtual ~empty_input_number_set() noexcept {}
};

/**
 * @brief Exception for unknown input numbers
 *
 * @note Unkown input numbers are used but no assigned
 *       to a hardware input
 */
class unknown_input_number : public std::runtime_error
{
public:
    /**
     * @brief Construct a new unknown_input_number exception
     *
     * @param usage A message about how the input number was used
     */
    unknown_input_number(std::string usage)
        : std::runtime_error(
              "There is an input number not assigned to a hardware input. Usage: " +
              usage) {}

    virtual ~unknown_input_number() noexcept {}
};

/**
 * @brief Exception for invalid user-defined input numbers
 *
 * @note Valid input numbers are in the range [0,127]
 */
class invalid_user_input_number : public std::runtime_error
{
public:
    /**
     * @brief Construct a new invalid_input_number exception
     *
     * @param value Offending input number
     */
    invalid_user_input_number(uint8_t value)
        : std::runtime_error(
              "The user-defined input number " +
              std::to_string(value) +
              " is out of range [0,127]") {}
    /**
     * @brief Construct a new invalid input number object
     *        for unspecified input numbers
     *
     */
    invalid_user_input_number()
        : std::runtime_error(
              "Trying to use an unspecified input number.") {}

    virtual ~invalid_user_input_number() noexcept {}
};

//-------------------------------------------------------------------
// Utility functions
//-------------------------------------------------------------------

/**
 * @brief Add an item to a collection without duplicates
 *
 * @tparam T Item class
 * @param item Item to be added
 * @param vector Collection
 * @return true If added
 * @return false If already exists
 */
template <typename T>
bool addIfNotExists(T item, std::vector<T> &vector)
{
    for (size_t i = 0; i < vector.size(); i++)
        if (vector.at(i) == item)
            return false;
    vector.push_back(item);
    return true;
}

/**
 * @brief Equivalent to Arduino's map()
 *
 * @param x Input value
 * @param in_min Input min value
 * @param in_max Input max value
 * @param out_min Output min value
 * @param out_max Output max value
 * @return int Output value
 */
inline int map_value(int x, int in_min, int in_max, int out_min, int out_max)
{
    const int run = in_max - in_min;
    if (run == 0)
        return 0;
    const int rise = out_max - out_min;
    const int delta = x - in_min;
    return (delta * rise) / run + out_min;
}

//-------------------------------------------------------------------
// Unspecified values
//-------------------------------------------------------------------

/**
 * @brief Unspecified value type
 *
 */
enum class UNSPECIFIED
{
    /// @brief Unspecified value
    VALUE = 0xFF
};

//-------------------------------------------------------------------
// Input numbers
//-------------------------------------------------------------------

/**
 * @brief Firmware-defined input numbers in the range [0,63] or
 *        unspecified.
 *
 */
struct InputNumber
{
public:
    /// @brief Construct an unspecified input number
    InputNumber()
    {
        _value = 0xFF;
    }

    /**
     * @brief Cast an integer to a input number
     *
     * @param value Input number in the range [0,63]
     */
    InputNumber(uint8_t value)
    {
        if (value >= 64)
            throw invalid_input_number(value);
        _value = value;
    }

    /**
     * @brief Assign an unspecified value
     *
     * @param value The unspecified value
     */
    InputNumber(UNSPECIFIED value)
    {
        _value = 0xFF;
    }

    /**
     * @brief Copy an input number
     *
     * @param number Input number to be copied
     */
    InputNumber(const InputNumber &number)
    {
        _value = number._value;
    }

    /**
     * @brief Typecast this input number to a 64-bit bitmap
     *
     * @return A bitmap
     */
    explicit operator uint64_t() const
    {
        if (_value < 64)
            return (1ULL << _value);
        else
            return 0ULL;
    }

    /**
     * @brief Typecast this input number to an unsigned int
     *
     * @return uint8_t Input number
     */
    operator uint8_t() const
    {
        if (_value > 63)
            throw std::bad_cast();
        else
            return _value;
    }

    /**
     * @brief Check if this input number is unspecified
     *
     * @param value Unspecified value
     * @return true If unspecified
     * @return false otherwise
     */
    bool operator==(const UNSPECIFIED value) const { return (_value > 63); }

    /**
     * @brief Check if this input number was specified
     *
     * @param value Unspecified value
     * @return true If specified
     * @return false otherwise
     */
    bool operator!=(const UNSPECIFIED value) const { return (_value < 64); }

    /// @cond

    inline bool operator==(const InputNumber value) const { return (_value == value._value); }
    inline bool operator!=(const InputNumber value) const { return (_value != value._value); }
    inline bool operator<(const InputNumber value) const { return (_value < value._value); }

    constexpr InputNumber &operator=(const InputNumber &other)
    {
        _value = other._value;
        return *this;
    }

    constexpr InputNumber &operator=(uint8_t value)
    {
        if (value >= 64)
            throw invalid_input_number(value);
        _value = value;
        return *this;
    }

    /// @endcond

    /**
     * @brief Book as in use
     *
     */
    void book() const
    {
        if (_value < 64)
            _registered |= (1ULL << _value);
    }

    /**
     * @brief Book all input numbers as in use (for testing)
     *
     */
    static void bookAll()
    {
        _registered = ~(0ULL);
    }

    /**
     * @brief Check if an input number is booked
     *
     * @param inputNumber Input number to check
     * @return true If booked
     * @return false If not booked
     */
    static bool booked(InputNumber inputNumber)
    {
        return _registered & (uint64_t)inputNumber;
    }

    /**
     * @brief Check if an input number is booked
     *
     * @param inputNumber Input number to check
     * @return true If booked
     * @return false If not booked
     */
    static bool booked(uint8_t inputNumber)
    {
        // Avoid two implicit typecasts
        if (inputNumber < 64)
            return _registered & (1ULL << inputNumber);
        else
            return false;
    }

    /**
     * @brief Get a bitmap of all booked input numbers
     *
     * @return uint64_t A bitmap
     */
    static uint64_t booked() { return _registered; }

#if CD_CI
    static void clearBook()
    {
        _registered = 0ULL;
    };
#endif

private:
    uint8_t _value;
    inline static uint64_t _registered = 0ULL;
};

/**
 * @brief Combination of input numbers
 *
 */
class InputNumberCombination : public std::vector<InputNumber>
{
public:
    /**
     * @brief Typecast this InputNumber vector to a uint8_t vector
     *
     * @return std::vector<uint8_t>
     */
    operator std::vector<uint8_t>()
    {
        std::vector<uint8_t> result = {};
        for (InputNumber inputNumber : *this)
        {
            if (inputNumber == UNSPECIFIED::VALUE)
                throw invalid_input_number();
            else
                result.push_back((uint8_t)inputNumber);
        }
        return result;
    }

    /**
     * @brief Create a vector of input numbers from integer constants/variables
     *
     * @param items List of values for initialization
     */
    InputNumberCombination(std::initializer_list<uint8_t> items)
    {
        this->clear();
        for (auto item = items.begin(); item != items.end(); ++item)
        {
            if (*item > 63)
                throw invalid_input_number(*item);
            else
                this->push_back(static_cast<InputNumber>(*item));
        }
    }

    /**
     * @brief Convert this vector to a single 64-bit bitmap
     *
     * @return uint64_t A bitmap
     */
    explicit operator uint64_t()
    {
        uint64_t bitmap = 0ULL;
        for (size_t i = 0; i < this->size(); i++)
            bitmap = bitmap | (uint64_t)this->at(i);
        return bitmap;
    }
};

// Well-known input numbers for PC game controllers
#define JOY_A 0                 ///< Game pad button "A"
#define JOY_B 1                 ///< Game pad button "B"
#define JOY_X 2                 ///< Game pad button "X"
#define JOY_Y 3                 ///< Game pad button "Y"
#define JOY_LB 4                ///< Game pad left shoulder button
#define JOY_RB 5                ///< Game pad right shoulder button
#define JOY_LSHIFT_PADDLE 4     ///< Left shift paddle
#define JOY_RSHIFT_PADDLE 5     ///< Right shift paddle
#define JOY_BACK 6              ///< Game pad back button
#define JOY_START 7             ///< Game pad start button
#define JOY_LTHUMBSTICK_CLICK 8 ///< Game pad left thumb stick click button
#define JOY_RTHUMBSTICK_CLICK 9 ///< Game pad right thumb stick click button

//-------------------------------------------------------------------
// User-defined input numbers
//-------------------------------------------------------------------

/**
 * @brief User-defined input numbers in the range [0,127]
 *
 */
struct UserInputNumber
{
public:
    /// @brief Construct an user-defined input number
    UserInputNumber()
    {
        _value = 0;
    }

    /**
     * @brief Cast an integer to a user-defined input number
     *
     * @param value Input number in the range [0,127]
     */
    UserInputNumber(uint8_t value)
    {
        if (value >= 128)
            throw invalid_user_input_number(value);
        _value = value;
    }

    /**
     * @brief Copy a user-defined input number
     *
     * @param number Input number to be copied
     */
    UserInputNumber(const UserInputNumber &number)
    {
        _value = number._value;
    }

    /**
     * @brief Get the most significant 64-bit bitmap
     *
     * @return A bitmask
     */
    uint64_t getHigh() const
    {
        if (_value < 64)
            return (1ULL << _value);
        else
            return 0ULL;
    }

    /**
     * @brief Get the least significant 64-bit bitmap
     *
     * @return A bitmask
     */
    uint64_t getLow() const
    {
        if (_value >= 64)
            return 0ULL;
        else
            return (1ULL << _value);
    }

    /**
     * @brief Typecast this input number to an unsigend int
     *
     * @return uint8_t Input number
     */
    operator uint8_t() const
    {
        if (_value > 127)
            throw std::bad_cast();
        else
            return _value;
    }

    /// @cond

    inline bool operator==(const UserInputNumber value) const { return (_value == value._value); }
    inline bool operator!=(const UserInputNumber value) const { return (_value != value._value); }
    inline bool operator<(const UserInputNumber value) const { return (_value < value._value); }

    /// @endcond

private:
    uint8_t _value;
};

//-------------------------------------------------------------------
// GPIO PINS
//-------------------------------------------------------------------
// Implemented in GPIO.cpp
//-------------------------------------------------------------------

/**
 * @brief GPIO pin number
 *
 * @note Checks for **unusable** pins
 */
struct GPIO
{
public:
    /**
     * @brief Assign a pin number
     *
     * @throw gpio_error If not usable or not capable
     * @param pin Pin number
     */
    GPIO(int pin)
    {
        if (pin < 0)
        {
            // Unspecified
            _pin = -1;
            return;
        }

        if (!GPIO_IS_VALID_GPIO(pin))
            throw gpio_error(pin, "Does not exist");
        bool reserved = false;
#if defined(CONFIG_IDF_TARGET_ESP32)
        reserved = (pin >= GPIO_NUM_6) && (pin <= GPIO_NUM_11); // Flash
        reserved = reserved ||
                   ((pin >= GPIO_NUM_16) && (pin <= GPIO_NUM_17) && psramFound()); // PSRAM
        reserved = reserved || (pin == GPIO_NUM_1) || (pin == GPIO_NUM_3);         // Serial port
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
// Not sure:
//        reserved = (pin >= GPIO_NUM_22) && (pin <= GPIO_NUM_32) && (pin != GPIO_NUM_26);
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
        reserved = ((pin >= GPIO_NUM_26) && (pin <= GPIO_NUM_32)); // Flash
        reserved = reserved ||
                   ((pin >= GPIO_NUM_35) && (pin <= GPIO_NUM_37) && psramFound()); // PSRAM
        reserved = reserved || (pin == GPIO_NUM_43) || (pin == GPIO_NUM_44);       // Serial port
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
        reserved = (pin >= GPIO_NUM_11) && (pin <= GPIO_NUM_17);
        reserved = reserved || (pin == GPIO_NUM_20) || (pin == GPIO_NUM_21); // Serial port
#elif CD_CI
        reserved = (pin == TEST_RESERVED_GPIO);
#endif
        if (reserved)
            throw gpio_error(pin, "Reserved for SPI FLASH, PSRAM or otherwise NOT USABLE");
        _pin = pin;
    }

    /**
     * @brief Create a GPIO as "not connected"
     *
     */
    GPIO() { _pin = -1; }

    /**
     * @brief Assign a "not connected" value
     *
     * @param value Unspecified value
     */
    GPIO(UNSPECIFIED value) : GPIO() {}

    /**
     * @brief Get pin number
     *
     * @return int Pin number or -1 if "not connected"
     */
    operator int() const { return _pin; }

    /**
     * @brief Copy another GPIO instance
     *
     * @param instance Instance to be copied
     */
    GPIO(const GPIO &instance) { _pin = instance._pin; }

    /**
     * @brief Check if "not connected"
     *
     * @param value Unspecified value
     * @return true If a GPIO pin was assigned
     * @return false If "not connected"
     */
    inline bool operator==(const UNSPECIFIED value) const { return _pin < 0; }

    /**
     * @brief Check if a pin number was assigned
     *
     * @param value  Unspecified value
     * @return true If "not connected"
     * @return false If a GPIO pin was assigned
     */
    inline bool operator!=(const UNSPECIFIED value) const { return _pin >= 0; }

    /// @cond

    inline bool operator!=(const GPIO value) const { return (_pin != value._pin); }
    inline bool operator==(const GPIO value) const { return (_pin == value._pin); }
    inline bool operator<(const GPIO value) const { return (_pin < value._pin); }

    /// @endcond

    /**
     * @brief Reserve this GPIO for exclusive use
     *
     * @throw gpio_error If already reserved
     */
    void reserve() const
    {
        abortIfUnspecified();
        if ((_pin >= 0) && !addIfNotExists<int>(_pin, reservedPins))
            throw gpio_error(_pin, "Already in use");
    }

    /**
     * @brief Grant this GPIO for non-exclusive use
     *
     */
    void grant() const
    {
        abortIfUnspecified();
        if (_pin >= 0)
            reservedPins.erase(
                std::find(
                    reservedPins.begin(),
                    reservedPins.end(),
                    _pin));
    }

    /**
     * @brief Throw an exception if unspecified
     *
     * @throw gpio_error If "not connected"
     */
    void abortIfUnspecified() const
    {
        if (_pin < 0)
            throw gpio_error(_pin, "Is unspecified but required");
    }

#if CD_CI
    static void clearReservations()
    {
        reservedPins.clear();
    }
#endif

private:
    inline static std::vector<int> reservedPins = {};

protected:
    /// @brief Assigned pin number
    int _pin;
};

/**
 * @brief Output-capable GPIO pin number
 *
 */
struct OutputGPIO : public GPIO
{
public:
    /**
     * @brief Create a GPIO
     *
     * @param pin Pin number
     */
    OutputGPIO(int pin) : GPIO(pin)
    {
        if (!GPIO_IS_VALID_OUTPUT_GPIO(pin))
            throw gpio_error(pin, "Not output-capable");
    }
    /// @brief Create a "not connected" GPIO
    OutputGPIO() : GPIO() {};
    /// @brief Create a "not connected" GPIO
    OutputGPIO(UNSPECIFIED value) : GPIO(value) {}
};

/**
 * @brief Input-capable GPIO pin number
 *
 */
struct InputGPIO : public GPIO
{
public:
    /// @brief Create a "not connected" input GPIO
    InputGPIO() : GPIO() {};
    /**
     * @brief Create an input GPIO
     *
     * @param pin Pin number
     */
    InputGPIO(int pin) : GPIO(pin) {}
    /// @brief Create a "not connected" input GPIO
    InputGPIO(UNSPECIFIED value) : GPIO(value) {}
};

/**
 * @brief ADC-capable GPIO pin number
 *
 */
struct ADC_GPIO : public InputGPIO
{
public:
    /// @brief Create a "not connected" ADC GPIO
    ADC_GPIO() : InputGPIO() {}

    /**
     * @brief Create an ADC GPIO connected to a pin number
     *
     * @param pin Pin number
     */
    ADC_GPIO(int pin) : InputGPIO(pin) {}

    /**
     * @brief Create a "not connected" ADC GPIO
     *
     * @param value Unspecified value
     */
    ADC_GPIO(UNSPECIFIED value) : InputGPIO(value) {}
};

/**
 * @brief RTC-capable GPIO pin number
 *
 */
struct RTC_GPIO : public InputGPIO
{
public:
    /**
     * @brief Create a RTC GPIO connected to a pin number
     *
     * @param pin Pin number
     */
    RTC_GPIO(int pin) : InputGPIO(pin)
    {
        if (!GPIO_IS_VALID_RTC_GPIO(pin))
            throw gpio_error(pin, "Not RTC-capable");
    }
};

//-------------------------------------------------------------------
// GPIO pin collections
//-------------------------------------------------------------------

/// @brief Collection of GPIOs
typedef std::vector<GPIO> GPIOCollection;
/// @brief Collection of input GPIOs
typedef std::vector<InputGPIO> InputGPIOCollection;
/// @brief Collection of output GPIOs
typedef std::vector<OutputGPIO> OutputGPIOCollection;

//-------------------------------------------------------------------
// I2C
//-------------------------------------------------------------------

/**
 * @brief I2C bus controller
 *
 */
#if CONFIG_IDF_TARGET_ESP32C3
// The ESP32-C3 does not have a secondary bus
enum class I2CBus
{
    PRIMARY = 0,
    SECONDARY = 0
};
#else
enum class I2CBus
{
    PRIMARY = 0,
    SECONDARY
};
#endif

//-------------------------------------------------------------------
// Power latch
//-------------------------------------------------------------------

/**
 * @brief Supported power latch modes
 *
 */
enum class PowerLatchMode : uint8_t
{
    /// @brief Power on when low voltage, power off when open drain
    POWER_OPEN_DRAIN,
    /// @brief Power on when low voltage, power off when high voltage
    POWER_OFF_HIGH,
    /// @brief Power on when high voltage, power off when low voltage
    POWER_OFF_LOW
};

//-------------------------------------------------------------------
// LED strips
//-------------------------------------------------------------------

/**
 * @brief Available RGB LED groups for pixel control
 *
 */
enum class PixelGroup
{
    /// @brief Telemetry leds group
    GRP_TELEMETRY = 0,
    /// @brief Buttons lighting group
    GRP_BUTTONS,
    /// @brief Individual leds group
    GRP_INDIVIDUAL
};

/**
 * @brief Pixel driver
 *
 */
enum class PixelDriver
{
    /// @brief WS2811 driver
    WS2811 = 0,
    /// @brief WS2812 family
    WS2812,
    /// @brief WS2815 family
    WS2815,
    /// @brief SK6812 driver
    SK6812,
    /// @brief UCS1903 driver
    UCS1903
};

/**
 * @brief Byte order of pixel data
 *
 */
enum class PixelFormat
{
    ///  @brief Auto-detect based on pixel driver
    AUTO = 0,
    ///  @brief Red-green-blue
    RGB,
    ///  @brief Red-blue-green
    RBG,
    ///  @brief Green-red-blue
    GRB,
    ///  @brief Green-blue-red
    GBR,
    ///  @brief Blue-red-green
    BRG,
    ///  @brief Blue-green-red
    BGR
};

//-------------------------------------------------------------------
// Telemetry data
//-------------------------------------------------------------------

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
        /// @brief Display character for current gear
        char gear = ' ';
        /// @brief Revolutions per minute
        uint16_t rpm = 0;
        /// @brief Percentage of RPM
        uint8_t rpmPercent = 0;
        /// @brief Non-zero at maximum torque
        uint8_t shiftLight1 = 0;
        /// @brief Non-zero at maximum power
        uint8_t shiftLight2 = 0;
        /// @brief True at maximum RPM
        bool revLimiter = false;
        /// @brief True if the engine is running
        bool engineStarted = false;
        /// @brief Speed in user-defined units (Kph or Mph)
        uint16_t speed = 0;
    } powertrain;
    /**
     * @brief ECU telemetry
     *
     */
    struct
    {
        /// @brief ABS is engaged
        bool absEngaged = false;
        /// @brief TC is engaged
        bool tcEngaged = false;
        /// @brief DRS is engaged
        bool drsEngaged = false;
        /// @brief The pit limiter is engaged
        bool pitLimiter = false;
        /// @brief True when fuel is low
        bool lowFuelAlert = false;
        /// @brief Driver-selected ABS mode
        uint8_t absLevel = 0;
        /// @brief Driver-selected TC mode
        uint8_t tcLevel = 0;
        /// @brief Driver-selected TC Cut mode (NOTE: currently not available)
        uint8_t tcCut = 0;
        /// @brief Percentage of brake bias towards front wheels
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
        /// @brief Remaining laps to the end of the session. Zero if does not apply.
        uint16_t remainingLaps = 0;
        /// @brief Remaining time to the end of the session, in minutes. Zero if does not apply.
        uint16_t remainingMinutes = 0;
    } raceControl;
    /**
     * @brief Gauges telemetry
     *
     */
    struct
    {
        /// @brief Percentage of turbo pressure
        uint8_t relativeTurboPressure = 0;
        /// @brief Turbo pressure in bars
        float absoluteTurboPressure = 0.0;
        /// @brief Water temperature in user-defined units (Celsius or Fahrenheit)
        uint16_t waterTemperature = 0;
        /// @brief Oil pressure in bars
        float oilPressure = 0.0;
        /// @brief Oil temperature in user-defined units (Celsius or Fahrenheit)
        uint16_t oilTemperature = 0;
        /// @brief Percentage of remaining fuel
        uint8_t relativeRemainingFuel = 0;
        /// @brief Remaining fuel in user-defined units (litres or gallons)
        uint16_t absoluteRemainingFuel = 0;
    } gauges;
} TelemetryData;

//-------------------------------------------------------------------
// User interface
//-------------------------------------------------------------------

/**
 * @brief Abstract interface for notifications and telemetry display.
 *
 * @note All methods are called in a separate low-priority thread
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
    /// @brief Set to true to receive and use powertrain telemetry data
    bool requiresPowertrainTelemetry = false;
    /// @brief Set to true to receive and use ECU telemetry data
    bool requiresECUTelemetry = false;
    /// @brief Set to true to receive and use race control telemetry data
    bool requiresRaceControlTelemetry = false;
    /// @brief Set to true to receive and use telemetry data for gauges
    bool requiresGaugeTelemetry = false;

    // Non copyable

    AbstractUserInterface() = default;
    AbstractUserInterface(const AbstractUserInterface &) = delete;
    AbstractUserInterface &operator=(const AbstractUserInterface &) = delete;

public:
    /**
     * @brief Get the maximum FPS supported by the underlying hardware
     *
     * @note You must override this method to receive telemetry data
     *
     * @return uint8_t Frames per second.
     */
    virtual uint8_t getMaxFPS() { return 0; }

    /**
     * @brief Get the stack size required by this user interface
     *
     * @warning You should override this method if you get
     *          "stack canary" or "dual exception" errors.
     *
     * @return uin16_t Stack size in bytes for the notification/frameserver daemon.
     *                 Return zero to use a default value.
     *                 Increase the stack size if you get "stack canary"
     *                 or "dual exception" errors and reboots.
     */
    virtual uint16_t getStackSize() { return 0; }

    /**
     * @brief Called just once after initialization.
     *
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
     *       - getMaxFPS must return non-zero.
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
     */
    virtual void onTelemetryData(const TelemetryData *pTelemetryData) {};

    /**
     * @brief Draw a single frame.
     *
     * @param elapsedMs Elapsed milliseconds since last call.
     *
     * @note Called at timed intervals when no notifications are pending.
     *       Do not assume perfect timing.
     *
     * @note Must not enter an infinite loop. Must return as soon as possible.
     *
     */
    virtual void serveSingleFrame(uint32_t elapsedMs) {};

    /**
     * @brief Notify a change in the current bite point.
     *
     * @param bitePoint Last known bite point.
     *
     */
    virtual void onBitePoint(uint8_t bitePoint) {};

    /**
     * @brief Notify device is connected.
     *
     */
    virtual void onConnected() {};

    /**
     * @brief Notify device is in discovery mode.
     *
     */
    virtual void onBLEdiscovering() {};

    /**
     * @brief Notify low battery.
     *
     * @note Called at timed intervals while the low battery
     *       condition persists.
     */
    virtual void onLowBattery() {};

    /**
     * @brief Cut power to the UI hardware.
     *
     * @note This is a command, not a notification.
     *       Not reversible.
     *
     */
    virtual void shutdown() {};
};

//-------------------------------------------------------------------

/**
 * @brief Notifications using pixel control
 *
 * @note All methods are called in a separate low-priority thread
 */
class PixelControlNotification : public AbstractUserInterface
{
private:
    virtual void onStart() override;
    virtual void onBitePoint(uint8_t bitePoint) override;
    virtual void onConnected() override;
    virtual void onBLEdiscovering() override;
    virtual void onLowBattery() override;
    virtual void serveSingleFrame(uint32_t elapsedMs) override {};
    virtual uint8_t getMaxFPS() override { return 0; }
    virtual void shutdown() override {};

protected:
    // Singleton pattern
    PixelControlNotification() {}
    virtual ~PixelControlNotification() {}

public:
    /**
     * @brief Get the singleton instance
     *
     * @return PixelControlNotification* Singleton instance
     */
    static PixelControlNotification *getInstance()
    {
        static PixelControlNotification *_instance = nullptr;
        if (!_instance)
            _instance = new PixelControlNotification();
        return _instance;
    }

public:
    PixelControlNotification(PixelControlNotification &other) = delete;
    void operator=(const PixelControlNotification &) = delete;

    /**
     * @brief Called just once after initialization.
     *
     */
    virtual void pixelControl_OnStart();

    /**
     * @brief Notify a change in current bite point.
     *
     * @param bitePoint Last known bite point.
     */
    virtual void pixelControl_OnBitePoint(uint8_t bitePoint);

    /**
     * @brief Notify device is connected.
     *
     * @note Called in a low priority thread.
     */
    virtual void pixelControl_OnConnected();

    /**
     * @brief Notify device is in discovery mode.
     *
     * @note Not called in the USB implementation.
     */
    virtual void pixelControl_OnBLEdiscovering();

    /**
     * @brief Notify low battery.
     *
     * @note Called at timed intervals while the low battery
     *       condition persists.
     */
    virtual void pixelControl_OnLowBattery();
};

//-------------------------------------------------------------------