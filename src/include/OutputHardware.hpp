/**
 * @file OutputHardware.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-02
 * @brief Output hardware classes
 *
 * @copyright Licensed under the EUPL
 *
 */

#pragma once

//---------------------------------------------------------------
// Imports
//---------------------------------------------------------------

#include "SimWheelTypes.hpp"
#include "driver/rmt_tx.h" // For rmt_channel_handle_t & rmt_encoder_handle_t
#include <cstring>         // For memset()

#include <initializer_list>

//---------------------------------------------------------------
// Led strip encoder
//---------------------------------------------------------------

/**
 * @brief Low-level interface to LED strips
 *
 */
class LEDStrip
{
public:
    /**
     * @brief Create an LED strip object.
     *
     * @param dataPin GPIO number attached to `Din` (data input).
     * @param pixelCount Total count of pixels in the strip.
     * @param useLevelShift Set to `false` when using 3.3V logic.
     *                      Set to `true` when using the level
     *                      shifter in open-drain mode.
     * @param pixelType Pixel driver.
     * @param pixelFormat Format of color data (byte order).
     *                    Set to `AUTO` for auto-detection.
     */
    LEDStrip(
        OutputGPIO dataPin,
        uint8_t pixelCount,
        bool useLevelShift,
        PixelDriver pixelType = PixelDriver::WS2812,
        PixelFormat pixelFormat = PixelFormat::AUTO);
    ~LEDStrip();

    /**
     * @brief Retrieve the pixel count in the strip.
     *
     * @return uint8_t Pixel count.
     */
    uint8_t getPixelCount() { return pixelCount; }

    /**
     * @brief Set global LED brightness
     *
     * @param value Brightness.
     *              255 is the highest and
     *              0 will turn all LEDs off.
     *
     * @note LEDs are very bright.
     *       Keep this value low for a comfortable experience.
     *       Defaults to 15 (decimal).
     */
    void brightness(uint8_t value) { brightnessWeight = value + 1; }

    // protected:
    /**
     * @brief Turn off all LEDs
     * @note Effective after show() is called.
     *
     */
    void clear();

    /**
     * @brief Set pixel color in RGB format
     *
     * @param pixelIndex Index of the pixel in the strip.
     * @param redChannel Red component of the color.
     * @param greenChannel Green component of the color.
     * @param blueChannel Blue component of the color.
     * @note Effective after show() is called.
     */
    void pixelRGB(
        uint8_t pixelIndex,
        uint8_t redChannel,
        uint8_t greenChannel,
        uint8_t blueChannel);

    /**
     * @brief Set color (in RGB format) to a range of pixels
     *
     * @param fromPixelIndex Index of the first pixel.
     * @param toPixelIndex Index of the last pixel.
     * @param redChannel Red component of the color.
     * @param greenChannel Green component of the color.
     * @param blueChannel Blue component of the color.
     * @note Effective after show() is called.
     */
    void pixelRangeRGB(
        uint8_t fromPixelIndex,
        uint8_t toPixelIndex,
        uint8_t redChannel,
        uint8_t greenChannel,
        uint8_t blueChannel);

    /**
     * @brief Set pixel color in RGB format
     *
     * @param pixelIndex Index of the pixel.
     * @param packedRGB Pixel color in packet RGB format
     */
    void pixelRGB(
        uint8_t pixelIndex,
        uint32_t packedRGB)
    {
        pixelRGB(pixelIndex,
                 (uint8_t)(packedRGB >> 16),
                 (uint8_t)(packedRGB >> 8),
                 (uint8_t)(packedRGB));
    }

    /**
     * @brief Set color (in RGB format) to a range of pixels
     *
     * @param fromPixelIndex Index of the first pixel.
     * @param toPixelIndex Index of the last pixel.
     * @param packedRGB Pixel color in packet RGB format
     */
    void pixelRangeRGB(
        uint8_t fromPixelIndex,
        uint8_t toPixelIndex,
        uint32_t packedRGB)
    {
        pixelRangeRGB(fromPixelIndex,
                      toPixelIndex,
                      (uint8_t)(packedRGB >> 16),
                      (uint8_t)(packedRGB >> 8),
                      (uint8_t)(packedRGB));
    }

    /**
     * @brief Shift all pixel colors to the next pixel index
     *
     */
    void shiftToNext();

    /**
     * @brief Shift all pixel colors to the previous pixel index
     *
     */
    void shiftToPrevious();

    /**
     * @brief Show pixel colors.
     *
     */
    void show();

private:
    uint8_t pixelCount;
    uint8_t *pixelData;
    PixelFormat pixelFormat;
    rmt_channel_handle_t rmtHandle = nullptr;
    rmt_encoder_handle_t encHandle = nullptr;
    bool changed = false;
    uint8_t brightnessWeight = 16;
    uint32_t resetTimeNs = 280000;

    void normalizeColor(uint8_t &r, uint8_t &g, uint8_t &b);
    void rawPixelRGB(
        uint8_t pixelIndex,
        uint8_t redChannel,
        uint8_t greenChannel,
        uint8_t blueChannel);
};

//---------------------------------------------------------------
// PCF8574 LED Driver
//---------------------------------------------------------------

/**
 * @brief 8-LED driver based on the PCF8574 GPIO expander
 *
 * @note For single-color LEDs.
 *       P0 is the left-most LED.
 *       P7 is the right-most LED.
 */
class PCF8574LedDriver
{
public:
    /**
     * @brief Construct a LED driver
     *
     * @param bus I2C bus where the chip is attached to.
     * @param address7bits Full I2C address in 7-bit format.
     *
     */
    PCF8574LedDriver(
        I2CBus bus,
        uint8_t address7bits);

    /**
     * @brief Set the state of a single LED
     *
     * @note Not displayed immediately
     *
     * @param index LED index in the range [0,7].
     *              P0 has index 0.
     * @param state True to turn on, false to turn off.
     */
    void setLed(uint8_t index, bool state);

    /**
     * @brief Shift all lights to the left
     *
     * @note Not displayed immediately.
     *       P0 is the left-most LED.
     */
    void shiftLeft();

    /**
     * @brief Shift all lights to the right
     *
     * @note Not displayed immediately.
     *       P0 is the left-most LED.
     */
    void shiftRight();

    /**
     * @brief Invert the state of all LEDs
     *
     * @note Not displayed immediately
     */
    void swap() { _state = ~_state; }

    /**
     * @brief Show the required LEDs all at once
     *
     */
    void show() const;

    /**
     * @brief Get the state of each LED
     *
     * @return uint8_t A bitmap. 1 means turn on. 0 means turn off.
     */
    uint8_t getState() const { return _state; };

    /**
     * @brief Set the state of each LED all at once
     *
     * @note Not displayed immediately
     *
     * @param state A bitmap. 1 means turn on. 0 means turn off.
     */
    void setState(uint8_t state) { _state = state; }

    ~PCF8574LedDriver()
    {
        setState(0);
        show();
    }

private:
    /// @brief Current state of the LEDs
    uint8_t _state = 0;

    /// @brief Slave device in the I2C API (must be type-casted)
    void *device = nullptr;
};

//---------------------------------------------------------------
// Single LED
//---------------------------------------------------------------

/**
 * @brief A simple LED driver for a single LED
 *
 * @note You may attach two or more LEDs to the output pin,
 *       but all of them will behave as a single LED.
 *
 */
class SingleLED
{
public:
    /**
     * @brief Create a new single-LED driver
     *
     * @param pin
     */
    SingleLED(OutputGPIO pin);

    /**
     * @brief Set the state of the LED
     *
     * @param state true to turn on, false to turn off.
     */
    void set(bool state) { _state = state; }

    /**
     * @brief Get the state of the LED
     *
     * @return true If on
     * @return false If off
     */
    bool get() { return _state; }

    /**
     * @brief Invert the state of the LED
     *
     * @note Not displayed immediately
     */
    void swap() { _state = !_state; }

    /**
     * @brief Show the state of the LED
     *
     */
    void show();

    ~SingleLED() { _pin.grant(); }

private:
    OutputGPIO _pin;
    bool _state = false;
};

//---------------------------------------------------------------
// OLED
//---------------------------------------------------------------

/**
 * @brief OLED resolution (width x heigh)
 *
 */
enum class OLED_resolution : uint8_t
{
    /// @brief 132 x 64 pixels
    // _132x64 = 0,
    /// @brief 128 x 128 pixels
    _128x128 = 0,
    /// @brief 128 x 64 pixels
    _128x64,
    /// @brief 128 x 32 pixels
    _128x32,
    /// @brief 64 x 32 pixels
    _64x32,
    /// @brief 96 x 16 pixels
    _96x16,
    /// @brief 72 x 40 pixels
    _72x40,
    /// @brief Default resolution
    _DEFAULT = _128x64
};

// /// @brief Physical orientation of the OLED display
// enum class OLEDOrientation : uint8_t
// {
//     /// @brief Normal orientation
//     degrees0 = 0,
//     /// @brief Rotate view 90 degrees
//     degrees90,
//     /// @brief Rotate view 180 degrees
//     degrees180,
//     /// @brief Rotate view 2700 degrees
//     degrees270,
//     /// @brief Default orientation
//     DEFAULT = degrees0
// };

/**
 * @brief Base class for all displays compatible with SSD1306 (I2C interface)
 *
 */
struct OLEDBase
{

    // protected:
    /**
     * @brief Create an OLED base object
     *
     * @param address7bits Full address in 7 bit format
     * @param bus I2C bus
     */
    OLEDBase(uint8_t address7bits, I2CBus bus);

    /**
     * @brief Create an OLED base object
     *
     * @details This constructor will probe each I2C address in @p try_address
     *          and use the first available in the I2C @p bus .
     *
     * @param try_addresses List of ordered I2C full addresses to try
     *                      (7 bit format).
     * @param bus I2C bus
     */
    OLEDBase(::std::initializer_list<uint8_t> &&try_addresses, I2CBus bus);

    /// @brief Destructor
    virtual ~OLEDBase();

    /// @brief Copy-constructor (default)
    /// @param other Instance to be copied
    OLEDBase(const OLEDBase &other) noexcept = default;

    /// @brief Move-constructor (default)
    /// @param other Instance to be moved
    OLEDBase(OLEDBase &&other) noexcept = default;

    /// @brief Copy-Assignment (default)
    /// @param other Instance to be copied
    OLEDBase &operator=(const OLEDBase &other) noexcept = default;

    /// @brief Move-Assignment (default)
    /// @param other Instance to be moved
    OLEDBase &operator=(OLEDBase &&other) noexcept = default;

    /**
     * @brief Check if there was a device response
     *        to the last I2C command issued
     *
     * @return true If the device was responding
     * @return false If not
     */
    bool available() { return last_i2c_result; }

    /**
     * @brief Write a command with no arguments
     *
     * @param command Command
     * @return true On success
     * @return false On failure
     */
    bool write_cmd(uint8_t command);

    /**
     * @brief Write a command with one argument
     *
     * @param command Command
     * @param arg First argument
     * @return true On success
     * @return false On failure
     */
    bool write_cmd(uint8_t command, uint8_t arg);

    /**
     * @brief Write a command with two argument
     *
     * @param command Command
     * @param arg1 First argument
     * @param arg2 Second argument
     * @return true On success
     * @return false On failure
     */
    bool write_cmd(uint8_t command, uint8_t arg1, uint8_t arg2);

    /**
     * @brief Write any sequence of commands and arguments
     *
     * @param buffer Pointer to command sequence
     * @param size Size of the command sequence
     * @return true On success
     * @return false On failure
     */
    bool write_cmd(const uint8_t *buffer, ::std::size_t size);

    /**
     * @brief Write to GDD RAM
     *
     * @param buffer Pointer to graphics display data
     * @param size Size of the graphics display data
     * @return true On success
     * @return false On failure
     */
    bool write_gdd_ram(const uint8_t *buffer, ::std::size_t size);

    /**
     * @brief Read the status register
     *
     * @param[out] status Status register if the return value is true
     * @return true On success
     * @return false On failure
     */
    bool read_status(uint8_t &status);

    /// @brief OLED controllers that can be automatically detected
    enum class Controller : uint8_t
    {
        /// @brief SSD1306 OLED controller
        SSD1306,
        /// @brief SH1107 OLED controller
        SH1107,
        /// @brief SH1106 OLED controller
        SH1106,
        /// @brief Unknown OLED controller
        UNKNOWN
    };

    /**
     * @brief Guess the OLED controller
     *
     * @return Controller Automatically-detected controller
     */
    Controller guess_controller();

private:
    /// @brief I2C device handler in the ESP-IDF API (must be type-casted)
    void *device = nullptr;
    /// @brief Result of last I2C operation
    bool last_i2c_result = false;
};

struct OLED : public OLEDBase
{
    OLED(
        I2CBus bus,
        OLED_resolution res = OLED_resolution::_DEFAULT);
    OLED(
        uint8_t address7bits,
        I2CBus bus,
        OLED_resolution res = OLED_resolution::_DEFAULT);

    /// @brief Copy-constructor (default)
    /// @param other Instance to be copied
    OLED(const OLED &other) noexcept = default;

    /// @brief Move-constructor (default)
    /// @param other Instance to be moved
    OLED(OLED &&other) noexcept = default;

    /// @brief Copy-Assignment (default)
    /// @param other Instance to be copied
    OLED &operator=(const OLED &other) noexcept = default;

    /// @brief Move-Assignment (default)
    /// @param other Instance to be moved
    OLED &operator=(OLED &&other) noexcept = default;

    /**
     * @brief Get the auto-detected display controller
     *
     * @return Controller Display controller
     */
    Controller controller() { return _controller; }

    /**
     * @brief Get the configured display resolution
     *
     * @return OLED_resolution Display resolution
     */
    OLED_resolution resolution() { return _resolution; }

    /**
     * @brief Get the display width
     *
     * @return uint8_t Width in pixels
     */
    uint8_t width() { return _width; }

    /**
     * @brief Get the display height
     *
     * @return uint8_t Height in pixels
     */
    uint8_t height() { return _height; }

    // Fundamental commands

    void contrast(uint8_t value);
    void enable_display(bool yesOrNo);
    void turn(bool onOrOff);

    // Frame buffer

    void setPixel(uint8_t x, uint8_t y, bool color);
    void clear();
    void show();

    // protected:
    OLED_resolution _resolution;
    Controller _controller = OLED::Controller::UNKNOWN;
    uint8_t _frame[2048] = {0}; // Size for the worst case: 128x128
    unsigned int _screen_offset = 0;
    uint8_t _width;
    uint8_t _height;

    void init();
    void inverse_display(bool yesOrNo);

    void locate(uint8_t x, uint8_t y);
};