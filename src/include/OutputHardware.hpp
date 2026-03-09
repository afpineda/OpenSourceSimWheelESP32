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

//------------------------------------------------------------------------------
// Monochrome OLED
//------------------------------------------------------------------------------

/**
 * @brief Monochrome OLED working parameters
 * @note For details, refer to the SSD1306 data sheet:
 *       https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
 */
struct OLEDParameters
{
    /// @brief True to flip horizontally
    bool flip_horizontal = false;
    /// @brief True to flip vertically
    bool flip_vertical = false;
    /// @brief True to swap black and white pixels
    bool inverted_display = false;
    /// @brief Screen width resolution in pixels
    uint8_t screen_width = 128;
    /// @brief Screen height resolution in pixels
    uint8_t screen_height = 64;
    /// @brief Row pixel index where display starts
    uint8_t start_row = 0;
    /// @brief Column pixel index where display starts
    uint8_t start_col = 0;
    /// @brief Row offset where the physical display meets the logical display
    uint8_t display_offset = 0;
    /// @brief Display contrast (higher value means higher contrast)
    uint8_t contrast = 127;
    /**
     * @brief COM pins value as required by the controller,
     *        bit[4] = COM pin configuration,
     *        bit[5] = COM left/right remap.
     */
    uint8_t COMpins = 0x12;
    /// @brief Display clock divide ratio/oscillator frequency
    uint8_t oscillator_frequency = 0x80;

public:
    /// @brief Base parameters for 128x64 displays having a 132x64 controller
    /// @return OLED parameters
    static OLEDParameters for132x64()
    {
        OLEDParameters result;
        result.start_col = 1;
        return result;
    }

    /**
     * @brief Force a specific graphics resolution
     *
     * @param width Screen width in pixels
     * @param height Screen height in pixels
     * @param params Other parameters
     * @return OLEDParameters Resulting parameters
     */
    static OLEDParameters withResolution(
        uint8_t width,
        uint8_t height,
        const OLEDParameters &params)
    {
        OLEDParameters result{params};
        result.screen_width = width;
        result.screen_height = height;
        return result;
    }
};

/**
 * @brief Base class for all displays compatible with SSD1306 (I2C interface)
 *
 * @note Protected methods return false if the device is not responding
 *       in the i2C bus
 */
struct OLEDBase
{
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
    Controller guess_controller() const noexcept;

    /**
     * @brief Check if the OLED was found in the I2C bus
     *
     * @return true If the device was responding
     * @return false If not
     */
    bool available() const noexcept { return (device); }

protected:
    /// @brief Create an uninitialized OLED base object
    constexpr OLEDBase() noexcept : device{nullptr} {}

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
    OLEDBase(
        ::std::initializer_list<uint8_t> &&try_addresses,
        I2CBus bus) noexcept;

    /// @brief Destructor
    virtual ~OLEDBase() noexcept;

    /// @brief Copy-constructor (default)
    /// @param other Instance to be copied
    OLEDBase(const OLEDBase &other) noexcept = default;

    /// @brief Move-constructor
    /// @param other Instance to be moved
    OLEDBase(OLEDBase &&other) noexcept;

    /// @brief Copy-Assignment (default)
    /// @param other Instance to be copied
    OLEDBase &operator=(const OLEDBase &other) noexcept = default;

    /// @brief Move-Assignment
    /// @param other Instance to be moved
    OLEDBase &operator=(OLEDBase &&other) noexcept;

    /**
     * @brief Raw write
     *
     * @param buffer Pointer to commands or data
     * @param size Size of @p buffer
     * @return true On success
     * @return false On failure
     */
    bool write(const uint8_t *buffer, ::std::size_t size) const noexcept;

    /**
     * @brief Write a command with no arguments
     *
     * @param command Command
     * @return true On success
     * @return false On failure
     */
    bool write_cmd(uint8_t command) const noexcept;

    /**
     * @brief Write a command with one argument
     *
     * @param command Command
     * @param arg First argument
     * @return true On success
     * @return false On failure
     */
    bool write_cmd(uint8_t command, uint8_t arg) const noexcept;

    /**
     * @brief Write a command with two argument
     *
     * @param command Command
     * @param arg1 First argument
     * @param arg2 Second argument
     * @return true On success
     * @return false On failure
     */
    bool write_cmd(uint8_t command, uint8_t arg1, uint8_t arg2) const noexcept;

    /**
     * @brief Write to GDD RAM
     *
     * @param buffer Pointer to graphics display data
     * @param size Size of the graphics display data
     * @return true On success
     * @return false On failure
     */
    bool write_gdd_ram(
        const uint8_t *buffer,
        ::std::size_t size) const noexcept;

    /**
     * @brief Read the status register
     *
     * @param[out] status Status register if the return value is true
     * @return true On success
     * @return false On failure
     */
    bool read_status(uint8_t &status) const noexcept;

private:
    /// @brief I2C device handler in the ESP-IDF API (must be type-casted)
    void *device = nullptr;
};

/**
 * @brief Monochrome OLED
 *
 */
struct OLED : public OLEDBase
{

    /// @brief Create an uninitialized OLED
    constexpr OLED() : OLEDBase(), _params{}, width_b{0}, height_b{0}
    {
        _params.screen_width = 0;
        _params.screen_height = 0;
    }

    /**
     * @brief Create an OLED using any of the default I2C addresses
     *
     * @param params OLED parameters
     * @param bus I2C Bus
     */
    OLED(
        const OLEDParameters &params,
        I2CBus bus);

    /**
     * @brief Create an OLED using a specific I2C address
     *
     * @param params OLED parameters
     * @param address7bits Full 7-bit I2C address
     * @param bus I2C Bus
     */
    OLED(
        const OLEDParameters &params,
        uint8_t address7bits,
        I2CBus bus);

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

    /// @brief Get the OLED parameters passed in the constructor
    /// @return OLED parameters
    OLEDParameters parameters() const noexcept { return _params; }

    /// @brief Get the frame size in bytes
    ::std::size_t frame_size() const noexcept
    {
        return width_b * height_b * 8;
    }

    /// @brief Set the display contrast
    /// @param value Contrast. Higher means more contrast.
    void contrast(uint8_t value);

    /// @brief Enable/Disable GDD RAM display
    /// @param yesOrNo True to enable, false to Disable
    void enable_display(bool yesOrNo);

    /// @brief Turn display on/off
    /// @param onOrOff True to turn on, false to turn off.
    void turn(bool onOrOff);

    /// @brief Switch pixel colors
    void inverse_display(bool yesOrNo);

    /// @brief Clear the display
    /// @param inverted True for white, false for black.
    void clear(bool inverted = false);

    /// @brief Display a frame at once
    /// @param frame Pointer to a frame buffer.
    void show(const uint8_t *frame);

protected:
    /// @brief OLED parameters given in the constructor
    OLEDParameters _params;
    /// @brief Screen width in bytes
    uint8_t width_b;
    /// @brief Screen height in bytes
    uint8_t height_b;

    /// @brief Initialize the display (called from the constructor)
    void init();

    /**
     * @brief Set the start page and start column before display
     * @note The controller is configured in "page" mode
     *
     * @param x Column (or segment) index
     * @param page Page index
     */
    void locate(uint8_t x, uint8_t page);

    /**
     * @brief Utility function to translate a row-major vector graphic
     *        to the column-major format used by OLED screens
     *
     * @param bit_index Column index in a single byte, range [0,7]
     * @param from Pointer to the frame buffer's first row and column
     * @param row_count Number of rows to translate, range [0,8]
     * @return uint8_t byte representing an 1x8 chunk
     *         (one segment in one page)
     */
    inline uint8_t row2col(
        uint8_t bit_index,
        const uint8_t *from,
        uint8_t row_count);
};