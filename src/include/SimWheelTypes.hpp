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
#include <set>
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
 * @note Valid input numbers are in the range [0,127]
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
              " is out of range [0,127]") {}
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
 * @note Unknown input numbers are used but not assigned
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
    unknown_input_number(const std::string &usage)
        : std::runtime_error(
              "There is an input number not assigned to a hardware input. Usage: " +
              usage) {}

    virtual ~unknown_input_number() noexcept {}
};

//-------------------------------------------------------------------
// Utility functions
//-------------------------------------------------------------------

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
// Input Bitmap
//-------------------------------------------------------------------

// Note: fucking Arduino macro
#undef bit

/**
 * @brief 128-bit integer
 *
 */
struct uint128_t
{
    /// @brief Least significant unsigned long long
    uint64_t low{0ULL};
    /// @brief Most significant unsigned long long
    uint64_t high{0ULL};

    /// @brief Non-zero check
    explicit constexpr operator bool()
    {
        return low || high;
    }

    /**
     * @brief Check bit
     *
     * @param n Bit index
     * @return true if the bit is set
     * @return false if the bit is not set
     */
    constexpr bool bit(uint8_t n) const noexcept
    {
        if (n < 64)
            return (1ULL << n) & low;
        else if (n < 128)
            return (1ULL << (n - 64) & high);
        else
            return false;
    }

    /**
     * @brief Set or clear a single bit
     *
     * @param n Bit index
     * @param value True to set, false to clear
     */
    constexpr void set_bit(uint8_t n, bool value = true) noexcept
    {
        if (n < 64)
        {
            if (value)
                low |= (1ULL << n);
            else
                low &= ~(1ULL << n);
        }
        else if (n < 128)
        {
            n -= 64;
            if (value)
                high |= (1ULL << n);
            else
                high &= ~(1ULL << n);
        }
    }

    /**
     * @brief Compound bitwise OR
     *
     * @param rhs Right operand
     * @return constexpr uint128_t& Reference to this instance
     */
    constexpr uint128_t &operator|=(const uint128_t &rhs) noexcept
    {
        low |= rhs.low;
        high |= rhs.high;
        return *this;
    }

    /**
     * @brief Compound bitwise AND
     *
     * @param rhs Right operand
     * @return constexpr uint128_t& Reference to this instance
     */
    constexpr uint128_t &operator&=(const uint128_t &rhs) noexcept
    {
        low &= rhs.low;
        high &= rhs.high;
        return *this;
    }

    /**
     * @brief Compound bitwise XOR
     *
     * @param rhs Right operand
     * @return constexpr uint128_t& Reference to this instance
     */
    constexpr uint128_t &operator^=(const uint128_t &rhs) noexcept
    {
        low ^= rhs.low;
        high ^= rhs.high;
        return *this;
    }

    /**
     * @brief Get the bitmap that represents a number
     *
     * @param n Number
     * @return constexpr uint128_t Bitmap
     */
    static constexpr uint128_t bitmap(uint8_t n) noexcept
    {
        if (n < 64)
            return {
                .low = (1ULL << n),
                .high = 0ULL,
            };
        else if (n < 128)
            return {
                .low = 0ULL,
                .high = (1ULL << (n - 64)),
            };
        else
            return {};
    }

    /**
     * @brief Get the bitwise negation of zero
     *
     * @return constexpr uint128_t Bitwise zero negation
     */
    static constexpr uint128_t neg() noexcept
    {
        return {
            .low = ~0ULL,
            .high = ~0ULL,
        };
    }
};

static_assert(
    sizeof(uint128_t) == 16,
    "Wrong size in the uint128_t data type. Check memory alignment.");

/**
 * @brief Shift left
 *
 * @param source Bitmap
 * @param n Count of bits to shift
 * @return constexpr uint128_t Shifted bitmap
 */
inline constexpr uint128_t operator<<(
    const uint128_t &source,
    ::std::size_t n) noexcept
{
    if (n == 0)
    {
        return source;
    }
    else
    {
        uint128_t result;
        if (n < 64)
        {
            result.low = (source.low << n);
            result.high = (source.high << n) | (source.low >> (64 - n));
        }
        else
        {
            result.low = 0ULL;
            result.high = source.low << (n - 64);
        }
        return result;
    }
}

/**
 * @brief Shift right
 *
 * @param source Bitmap
 * @param n Count of bits to shift
 * @return constexpr uint128_t Shifted bitmap
 */
inline constexpr uint128_t operator>>(
    const uint128_t &source,
    ::std::size_t n) noexcept
{
    if (n == 0)
    {
        return source;
    }
    else
    {
        uint128_t result;
        if (n < 64)
        {
            result.low = (source.low >> n) | (source.high << (64 - n));
            result.high = (source.high >> n);
        }
        else
        {
            result.high = 0ULL;
            result.low = source.high >> (n - 64);
        }
        return result;
    }
}

/**
 * @brief Bitwise OR
 *
 * @param a Left operand
 * @param b Right operand
 * @return constexpr uint128_t Resulting bitmap
 */
inline constexpr uint128_t operator|(uint128_t a, uint128_t b) noexcept
{
    return {
        .low = a.low | b.low,
        .high = a.high | b.high,
    };
}

/**
 * @brief Bitwise AND
 *
 * @param a Left operand
 * @param b Right operand
 * @return constexpr uint128_t Resulting bitmap
 */
inline constexpr uint128_t operator&(uint128_t a, uint128_t b) noexcept
{
    return {
        .low = a.low & b.low,
        .high = a.high & b.high,
    };
}

/**
 * @brief Bitwise XOR
 *
 * @param a Left operand
 * @param b Right operand
 * @return constexpr uint128_t Resulting bitmap
 */
inline constexpr uint128_t operator^(uint128_t a, uint128_t b) noexcept
{
    return {
        .low = a.low ^ b.low,
        .high = a.high ^ b.high,
    };
}

/**
 * @brief Bitwise negation
 *
 * @param a Operand
 * @return constexpr uint128_t Resulting bitmap
 */
inline constexpr uint128_t operator~(uint128_t a) noexcept
{
    return {
        .low = ~(a.low),
        .high = ~(a.high),
    };
}

/**
 * @brief Check equality
 *
 * @param a Left operand
 * @param b Right operand
 * @return true If equal
 * @return false If not equal
 */
inline constexpr bool operator==(uint128_t a, uint128_t b) noexcept
{
    return (a.low == b.low) && (a.high == b.high);
}

/**
 * @brief Check inequality
 *
 * @param a Left operand
 * @param b Right operand
 * @return true If not equal
 * @return false If equal
 */
inline constexpr bool operator!=(uint128_t a, uint128_t b)
{
    return (a.low != b.low) || (a.high != b.high);
}

//-------------------------------------------------------------------
// Input numbers
//-------------------------------------------------------------------

/**
 * @brief Firmware-defined input numbers in the range [0,127] or
 *        unspecified.
 *
 */
struct InputNumber
{
public:
    /// @brief Construct an unspecified input number
    constexpr InputNumber() noexcept
    {
        _value = 0xFF;
    }

    /**
     * @brief Cast an integer to a input number
     *
     * @param value Input number in the range [0,127]
     */
    constexpr InputNumber(uint8_t value)
    {
        if (value >= 128)
            throw invalid_input_number(value);
        _value = value;
    }

    /**
     * @brief Assign an unspecified value
     *
     * @param value The unspecified value
     */
    constexpr InputNumber(UNSPECIFIED value) noexcept
    {
        _value = 0xFF;
    }

    /**
     * @brief Copy constructor (default)
     *
     * @param number Input number to be copied
     */
    constexpr InputNumber(const InputNumber &number) = default;

    /**
     * @brief Move constructor (default)
     *
     * @param number Input number to be moved
     */
    constexpr InputNumber(InputNumber &&number) = default;

    /**
     * @brief Copy-assignment (default)
     *
     * @param number Input number to be copied
     * @return constexpr InputNumer&
     */
    constexpr InputNumber &operator=(const InputNumber &number) = default;

    /**
     * @brief Move-assignment (default)
     *
     * @param number Input number to be moved
     */
    constexpr InputNumber &operator=(InputNumber &&number) = default;

    /**
     * @brief Typecast to 128-bit bitmap
     *
     * @return uint128_t Bitmap
     */
    explicit constexpr operator uint128_t() const
    {
        return uint128_t::bitmap(_value);
    }

    /**
     * @brief Typecast this input number to an unsigned int
     *
     * @return uint8_t Input number
     */
    constexpr operator uint8_t() const
    {
        return _value;
    }

    /**
     * @brief Check if this input number is unspecified
     *
     * @param value Unspecified value
     * @return true If unspecified
     * @return false otherwise
     */
    bool constexpr operator==(const UNSPECIFIED value) const
    {
        return (_value > 127);
    }

    /**
     * @brief Check if this input number was specified
     *
     * @param value Unspecified value
     * @return true If specified
     * @return false otherwise
     */
    bool constexpr operator!=(const UNSPECIFIED value) const noexcept
    {
        return (_value < 128);
    }

    /// @cond

    inline constexpr bool operator==(const InputNumber value) const noexcept
    {
        return (_value == value._value);
    }
    inline constexpr bool operator!=(const InputNumber value) const noexcept
    {
        return (_value != value._value);
    }
    inline constexpr bool operator<(const InputNumber value) const noexcept
    {
        return (_value < value._value);
    }
    inline constexpr bool operator<=(const InputNumber value) const noexcept
    {
        return (_value <= value._value);
    }
    inline constexpr bool operator>(const InputNumber value) const noexcept
    {
        return (_value > value._value);
    }
    inline constexpr bool operator>=(const InputNumber value) const noexcept
    {
        return (_value >= value._value);
    }

    /// @endcond

    /**
     * @brief Book as in use
     *
     */
    void book() const
    {
        _registered.set_bit(_value, true);
    }

    /**
     * @brief Unbook
     *
     */
    void unbook() const
    {
        _registered.set_bit(_value, false);
    }

    /**
     * @brief Book all input numbers as in use (for testing)
     *
     */
    static void bookAll()
    {
        _registered = uint128_t::neg();
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
        return _registered.bit(inputNumber._value);
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
        return _registered.bit(inputNumber);
    }

    /**
     * @brief Get a bitmap of all booked input numbers
     *
     * @return uint128_t A bitmap
     */
    static uint128_t booked() { return _registered; }

    /**
     * @brief Book an input number as in use
     *
     * @param inputNumber Input Number
     */
    static void unbook(uint8_t inputNumber)
    {
        _registered.set_bit(inputNumber, false);
    }

    /**
     * @brief Unbook
     *
     * @param inputNumber Input number
     */
    static void book(uint8_t inputNumber)
    {
        _registered.set_bit(inputNumber, true);
    }

#if CD_CI
    static void clearBook()
    {
        _registered.low = 0ULL;
        _registered.high = 0ULL;
    };
#endif

private:
    uint8_t _value;
    inline static uint128_t _registered{};
};

/**
 * @brief Combination of input numbers
 *
 */
struct InputNumberCombination : public uint128_t
{
    /// @brief Create an empty combination of input numbers
    constexpr InputNumberCombination() : uint128_t{} {};

    /**
     * @brief Create a combination of input numbers
     *
     * @param items List of values for initialization
     */
    constexpr InputNumberCombination(
        std::initializer_list<InputNumber> items) noexcept : uint128_t{}
    {
        for (auto n : items)
            set_bit(n);
    }

    /**
     * @brief Create from a 128-bit bitmap
     *
     * @param from Bitmap
     */
    constexpr InputNumberCombination(const uint128_t &from)
        : uint128_t{from} {};

    /**
     * @brief Create from a 128-bit bitmap
     *
     * @param from Bitmap
     */
    constexpr InputNumberCombination(uint128_t &&from)
        : uint128_t{from} {};

    /**
     * @brief Get the count of input numbers in the combination
     *
     * @return constexpr uint8_t Count of input numbers
     */
    constexpr uint8_t size() const noexcept
    {
        uint8_t result = 0;
        for (uint8_t i = 0; i < 128; i++)
            if (bit(i))
                result++;
        return result;
    }

    /// @brief Copy constructor (default)
    constexpr InputNumberCombination(
        const InputNumberCombination &) noexcept = default;

    /// @brief Move constructor (default)
    constexpr InputNumberCombination(
        InputNumberCombination &&) noexcept = default;

    /// @brief Copy-assignment (default)
    constexpr InputNumberCombination &operator=(
        const InputNumberCombination &) noexcept = default;

    /// @brief Move-assignment (default)
    constexpr InputNumberCombination &operator=(
        InputNumberCombination &&) = default;

    // NEEDED???
    // /**
    //  * @brief Typecast to vector
    //  *
    //  * @return std::vector<uint8_t> Vector of input numbers
    //  */
    // operator std::vector<uint8_t>()
    // {
    //     std::vector<uint8_t> result = {};
    //     for (uint8_t n = 0; n < 128; n++)
    //     {
    //         if (bit(n))
    //             result.push_back(n);
    //     }
    //     return result;
    // }
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

        // reserved = reserved || (pin == GPIO_NUM_1) || (pin == GPIO_NUM_3);         // Serial port
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
// Not sure:
//        reserved = (pin >= GPIO_NUM_22) && (pin <= GPIO_NUM_32) && (pin != GPIO_NUM_26);
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
        reserved = ((pin >= GPIO_NUM_26) && (pin <= GPIO_NUM_32)); // Flash
        reserved = reserved ||
                   ((pin >= GPIO_NUM_35) && (pin <= GPIO_NUM_37) && psramFound()); // PSRAM

        // reserved = reserved || (pin == GPIO_NUM_43) || (pin == GPIO_NUM_44);       // Serial port
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
        reserved = (pin >= GPIO_NUM_11) && (pin <= GPIO_NUM_17);

        // reserved = reserved || (pin == GPIO_NUM_20) || (pin == GPIO_NUM_21); // Serial port
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
        if (reservedPins.find(_pin) != reservedPins.end())
            throw gpio_error(_pin, "Already in use");
        else
            reservedPins.insert(_pin);
    }

    /**
     * @brief Grant this GPIO for non-exclusive use
     *
     */
    void grant() const
    {
        abortIfUnspecified();
        reservedPins.insert(_pin);
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
    inline static ::std::set<int> reservedPins{};

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
        if ((pin >= 0) && !GPIO_IS_VALID_OUTPUT_GPIO(pin))
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
        if ((pin >= 0) && !GPIO_IS_VALID_RTC_GPIO(pin))
            throw gpio_error(pin, "Not RTC-capable");
    }
};

//-------------------------------------------------------------------
// GPIO pin collections
//-------------------------------------------------------------------

/// @brief Collection of GPIOs
typedef std::set<GPIO> GPIOCollection;
/// @brief Collection of input GPIOs
typedef std::set<InputGPIO> InputGPIOCollection;
/// @brief Collection of output GPIOs
typedef std::set<OutputGPIO> OutputGPIOCollection;

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
// Connectivity
//-------------------------------------------------------------------

/// @brief Connectivity choice
enum class Connectivity : uint8_t
{
    /// @brief Combined USB and BLE connectivity if available
    USB_BLE = 0,
    /// @brief Combined USB and BLE connectivity with forced connection drop
    USB_BLE_EXCLUSIVE = 1,
    /// @brief USB connectivity only, if available
    USB = 2,
    /// @brief BLE connectivity only, if available
    BLE = 3,
    /// @brief No connectivity at all (for troubleshooting)
    DUMMY = 4,
    /// @brief Default connectivity
    _DEFAULT = USB_BLE
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
    UCS1903,
    /// @brief APA106 driver
    APA106
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
// Pixel format
//-------------------------------------------------------------------

/**
 * @brief Byte order of pixel data starting with the least significant byte
 *
 */
enum class PixelFormat : unsigned char
{
    ///  @brief Red-green-blue
    RGB = 0,
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
// Pixels
//-------------------------------------------------------------------

/**
 * @brief Pixel in 3-byte packed RGB format
 *
 */
struct Pixel
{
    /// @brief Blue channel
    uint8_t blue;
    /// @brief Green channel
    uint8_t green;
    /// @brief Red channel
    uint8_t red;

    /**
     * @brief Convert to packed RGB format
     *
     * @return uint32_t Packed RGB value
     */
    operator uint32_t() const noexcept
    {
        return (blue) | (green << 8) | (red << 16);
    }

    /**
     * @brief Convert to packed RGB format
     *
     * @return int Packed RGB value
     */
    operator int() const noexcept
    {
        return (blue) | (green << 8) | (red << 16);
    }

    /**
     * @brief Create from a packed RGB value
     *
     * @param packedRGB Packed RGB value
     */
    Pixel(uint32_t packedRGB) noexcept
    {
        red = packedRGB >> 16;
        green = packedRGB >> 8;
        blue = packedRGB;
    }

    /**
     * @brief Create as a black pixel
     *
     */
    Pixel() noexcept
    {
        red = 0;
        green = 0;
        blue = 0;
    }

    /// @brief Copy-constructor
    /// @param source Instance to be copied
    Pixel(const Pixel &source) = default;

    /// @brief Move-constructor
    /// @param source Instance transferring ownership
    Pixel(Pixel &&source) = default;

    /**
     * @brief Assign a packed RGB color
     *
     * @param packedRGB Packed RGB value
     * @return int Non-zero
     */
    Pixel &operator=(uint32_t packedRGB) noexcept
    {
        red = packedRGB >> 16;
        green = packedRGB >> 8;
        blue = packedRGB;
        return *this;
    }

    /// @brief Copy-assignment
    /// @param source Instance to be copied
    /// @return This instance
    Pixel &operator=(const Pixel &source) = default;

    /// @brief Move-assignment
    /// @param source Instance transferring ownership
    /// @return This instance
    Pixel &operator=(Pixel &&source) = default;

    /**
     * @brief Compare to a packed RGB color
     *
     * @param packedRGB Packed RGB value
     * @return true If this pixel matches @p packedRGB
     * @return false Otherwise
     */
    bool operator==(uint32_t packedRGB) const noexcept
    {
        return (packedRGB == static_cast<uint32_t>(*this));
    }

    /**
     * @brief Compare to a packed RGB color
     *
     * @param packedRGB Packed RGB value
     * @return true If this pixel matches @p packedRGB
     * @return false Otherwise
     */
    bool operator==(int packedRGB) const noexcept
    {
        return (packedRGB == static_cast<int>(*this));
    }

    /**
     * @brief Compare to another pixel
     *
     * @param other Other pixel
     * @return true If this pixel matches @p other
     * @return false Otherwise
     */
    bool operator==(const Pixel &other) const noexcept
    {
        return (red == other.red) &&
               (blue == other.blue) &&
               (green == other.green);
    }

    /**
     * @brief Compare to a packed RGB color
     *
     * @param packedRGB Packed RGB value
     * @return true If this pixel does not match @p packedRGB
     * @return false Otherwise
     */
    bool operator!=(uint32_t packedRGB) const noexcept
    {
        return (packedRGB != static_cast<uint32_t>(*this));
    }

    /**
     * @brief Compare to a packed RGB color
     *
     * @param packedRGB Packed RGB value
     * @return true If this pixel does not match @p packedRGB
     * @return false Otherwise
     */
    bool operator!=(int packedRGB) const noexcept
    {
        return (packedRGB != static_cast<int>(*this));
    }

    /**
     * @brief Compare to another pixel
     *
     * @param other Other pixel
     * @return true If this pixel does not match @p other
     * @return false Otherwise
     */
    bool operator!=(const Pixel &other) const noexcept
    {
        return (red != other.red) ||
               (blue != other.blue) ||
               (green != other.green);
    }

    /**
     * @brief Get the first color channel in a certain pixel format
     *
     * @param format Pixel format
     * @return uint8_t Color channel
     */
    uint8_t byte0(PixelFormat format) const noexcept
    {
        switch (format)
        {
        case PixelFormat::BGR:
            [[fallthrough]];
        case PixelFormat::BRG:
            return blue;
        case PixelFormat::GBR:
            [[fallthrough]];
        case PixelFormat::GRB:
            return green;
        case PixelFormat::RBG:
            [[fallthrough]];
        case PixelFormat::RGB:
            return red;
        }
        return 0;
    }

    /**
     * @brief Get the second color channel in a certain pixel format
     *
     * @param format Pixel format
     * @return uint8_t Color channel
     */
    uint8_t byte1(PixelFormat format) const noexcept
    {
        switch (format)
        {
        case PixelFormat::RBG:
            [[fallthrough]];
        case PixelFormat::GBR:
            return blue;
        case PixelFormat::BGR:
            [[fallthrough]];
        case PixelFormat::RGB:
            return green;
        case PixelFormat::BRG:
            [[fallthrough]];
        case PixelFormat::GRB:
            return red;
        }
        return 0;
    }

    /**
     * @brief Get the third color channel in a certain pixel format
     *
     * @param format Pixel format
     * @return uint8_t Color channel
     */
    uint8_t byte2(PixelFormat format) const noexcept
    {
        switch (format)
        {
        case PixelFormat::GRB:
            [[fallthrough]];
        case PixelFormat::RGB:
            return blue;
        case PixelFormat::BRG:
            [[fallthrough]];
        case PixelFormat::RBG:
            return green;
        case PixelFormat::GBR:
            [[fallthrough]];
        case PixelFormat::BGR:
            return red;
        }
        return 0;
    }
}; // Pixel

static_assert(sizeof(Pixel) == 3);

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

    virtual ~AbstractUserInterface() {}

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
     * @brief Notify that user settings have been saved to flash memory
     *
     */
    virtual void onSaveSettings() {};

    /**
     * @brief Notify user input
     *
     * @param inputNumber Input number that was pressed
     */
    virtual void onUserInput(uint8_t inputNumber) {};

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
    virtual void onSaveSettings() override;
    virtual void serveSingleFrame(uint32_t elapsedMs) override {};
    virtual uint8_t getMaxFPS() override { return 0; }
    virtual void shutdown() override {};

protected:
    // Singleton pattern

    /// @brief Initialize
    PixelControlNotification() : AbstractUserInterface() {}

    // For descendant classes

    /// @brief Pixel vector for the telemetry group
    ::std::vector<Pixel> telemetry_pixel{};

    /// @brief Pixel vector for the backlit buttons group
    ::std::vector<Pixel> backlit_button_pixel{};

    /// @brief Pixel vector for the individual pixels group
    ::std::vector<Pixel> individual_pixel{};

    /**
     * @brief Flag to indicate host connection
     *
     * @note True on startup or connection loss.
     *       False after a connection is established.
     *       For read only. Do not overwrite.
     */
    bool notConnectedYet = true;

    /**
     * @brief Show pixels all at once in a specific group
     *
     * @param group Pixel group
     */
    void show(PixelGroup group) noexcept;

    /**
     * @brief Show pixels all at once in all groups
     *
     */
    void show() noexcept;

    /**
     * @brief Show all pixels in black in all groups all at once
     *
     * @note Does not overwrite pixel vector members
     */
    static void reset() noexcept;

    /**
     * @brief Get the count of pixels in a pixel group
     *
     * @param group Pixel group
     * @return uint8_t The count of pixels or zero
     *                 if the group was not configured.
     */
    static uint8_t getPixelCount(PixelGroup group) noexcept;

    /**
     * @brief Macro to render the current battery SoC
     *
     * @note Does nothing if there is no battery.
     *       Pixels are rendered, but not shown yet.
     *
     * @param barColor Packed RGB color to use in the percentage bar.
     *
     * @return True if there is a battery.
     * @return False if there is no battery.
     */
    bool renderBatteryLevel(uint32_t barColor = 0x00ACFA70);

public:
    /**
     * @brief  Get the singleton instance
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
     * @brief Notify that user settings have been saved to flash memory
     *
     */
    virtual void pixelControl_OnSaveSettings();

    /**
     * @brief Notify low battery.
     *
     * @note Called at timed intervals while the low battery
     *       condition persists.
     */
    virtual void pixelControl_OnLowBattery();
};
