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
#include <vector>
#include <initializer_list>

//---------------------------------------------------------------
// Pixel vector
//---------------------------------------------------------------

/**
 * @brief Helper class to wrap around pixel vectors
 *
 */
struct PixelVectorHelper
{
public:
    /// @brief Size type of this vector
    using size_type = typename ::std::vector<Pixel>::size_type;

    /// @brief Type of the backing variable
    using vector_type = typename ::std::vector<Pixel>;

    /**
     * @brief Shift the contents of a vector
     *        up (right) or down (left)
     *
     * @note if @p from_index is greater than @p to_index,
     *       contents are shifted down, otherwise,
     *       contents are shifted up.
     *
     * @note The pixel that overflows in one end is reintroduced
     *       in the other end
     *
     * @param[in,out] data Pixel vector
     * @param from_index Index where shifting starts (inclusive)
     * @param to_index Index where shifting ends (inclusive)
     * @param shift Count of pixels to be shifted
     */
    static void shift(
        vector_type &data,
        size_type from_index,
        size_type to_index,
        size_type shift = 1);

    /**
     * @brief Shift left (or down)
     *
     * @param[in,out] data Pixel vector
     * @param count Number of pixels to shift
     */
    static void shift_left(vector_type &data, size_type count) noexcept;

    /**
     * @brief Shift right (or up)
     *
     * @param[in,out] data Pixel vector
     * @param count Number of pixels to shift
     */
    static void shift_right(vector_type &data, size_type count) noexcept;

    /**
     * @brief Fill the entire vector with a pixel color
     *
     * @param[in,out] data Pixel vector
     * @param color Pixel color
     */
    static void fill(vector_type &data, const Pixel &color) noexcept;

    /**
     * @brief Fill a segment with a pixel color
     *
     * @param[in,out] data Pixel vector
     * @param color Pixel color
     * @param fromIndex Segment start index (inclusive)
     * @param toIndex Segment end index (inclusive)
     */
    static void fill(
        vector_type &data,
        size_type fromIndex,
        size_type toIndex,
        const Pixel &color) noexcept;

}; // PixelVectorHelper

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
    /// @brief Pixel vector type
    using pixel_vector_type = typename ::std::vector<Pixel>;

    /**
     * @brief Create an LED strip
     *
     * @param dataPin GPIO number attached to `Din` (data input)
     * @param pixelCount Total count of pixels in the strip
     * @param useLevelShift Set to `false` when using 3.3V logic.
     *                      Set to `true` when using the level
     *                      shifter in open-drain mode.
     * @param driver Pixel driver
     * @param reverse_display True if the logical arrangement of the pixels
     *                        is the inverse of their physical arrangement
     */
    LEDStrip(
        OutputGPIO dataPin,
        uint8_t pixelCount,
        bool useLevelShift,
        PixelDriver driver = PixelDriver::WS2812,
        bool reverse_display = false);

    /// @brief Destructor
    virtual ~LEDStrip();

    /**
     * @brief Move constructor
     *
     * @param other Instance to be moved
     */
    LEDStrip(LEDStrip &&other);

    /**
     * @brief Move-assignment
     *
     * @param other Instance to be moved
     */
    LEDStrip &operator=(LEDStrip &&other);

    /// @brief Copy constructor (deleted)
    LEDStrip(const LEDStrip &other) = delete;

    /// @brief Copy-assignment (deleted)
    LEDStrip &operator=(const LEDStrip &other) = delete;

    /**
     * @brief Retrieve the pixel count in the strip
     *
     * @return uint8_t Pixel count
     */
    uint8_t getPixelCount() { return pixelCount; }

    /**
     * @brief Set the global brightness
     *
     * @param value Brightness.
     *              255 is the highest and
     *              0 will turn all LEDs off.
     *
     * @note LEDs are very bright.
     *       Keep this value low for a comfortable experience.
     *       Defaults to 127 (decimal).
     */
    void brightness(uint8_t value) { brightnessWeight = value + 1; }

    /**
     * @brief Create a vector of pixels suitable for this LED strip
     *
     * @param color Initial color for all pixels (defaults to black)
     * @return PixelVector Pixel vector (ownership transferred to the caller)
     */
    pixel_vector_type pixelVector(const Pixel &color = 0)
    {
        pixel_vector_type result(pixelCount, color);
        return result;
    }

    /**
     * @brief Turn off all LEDs immediately
     *
     */
    void clear()
    {
        show(pixelVector(0));
    }

    /**
     * @brief Show pixels all at once
     *
     * @param pixels Pixel vector
     */
    void show(const pixel_vector_type &pixels);

private:
    /// @brief Clock resolution in hertz (1 tick=0.1 us=100ns)
    static constexpr uint32_t clockResolutionHz = 10000000;
    /// @brief RMT symbol count per encoded byte
    static constexpr size_t symbols_per_byte = 8;
    /// @brief RMT symbol count per pixel
    static constexpr size_t symbols_per_pixel =
        sizeof(Pixel) * symbols_per_byte;

    /// @brief Transmission handle
    rmt_channel_handle_t rmtHandle = nullptr;
    /// @brief Pixel encoder handle
    rmt_encoder_handle_t pixel_encoder_handle = nullptr;
    /// @brief Byte encoder configuration for pixel display
    rmt_bytes_encoder_config_t byte_enc_config{};
    /// @brief Number of pixels in the LED strip
    uint8_t pixelCount;
    /// @brief Pixel format required by the pixel driver
    PixelFormat pixelFormat;
    /// @brief Global brightness correction factor in the range [1,256]
    uint16_t brightnessWeight = 128;
    /// @brief Rest time in the data pin for all pixels to show up
    uint32_t restTimeNs = 280000;
    /// @brief Reverse display or not
    bool reversed = false;

    /**
     * @brief Encode pixel data and apply the brightness reduction factor
     *
     * @param data Pixel data in BGR format as stored in PixelVector
     * @param data_size Pixel data size in bytes
     * @param symbols_written Count of symbols previously written
     * @param symbols_free Count of symbols available in the transmit buffer
     * @param symbols Pointer to the transmit buffer
     * @param done Pointer to end of transaction flag
     * @param arg Pointer to the LEDStrip::Implementation instance
     * @return size_t Symbols written
     */
    static size_t pixels_rmt_encoder(
        const void *data,
        size_t data_size,
        size_t symbols_written,
        size_t symbols_free,
        rmt_symbol_word_t *symbols,
        bool *done,
        void *arg);
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
        result.start_col = 2;
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

    /// @brief Copy-constructor (deleted)
    /// @param other Instance to be copied
    OLEDBase(const OLEDBase &other) noexcept = delete;

    /// @brief Move-constructor
    /// @param other Instance to be moved
    OLEDBase(OLEDBase &&other) noexcept;

    /// @brief Copy-Assignment (deleted)
    /// @param other Instance to be copied
    OLEDBase &operator=(const OLEDBase &other) noexcept = delete;

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

    /// @brief Copy-constructor (deleted)
    /// @param other Instance to be copied
    OLED(const OLED &other) noexcept = delete;

    /// @brief Move-constructor (default)
    /// @param other Instance to be moved
    OLED(OLED &&other) noexcept = default;

    /// @brief Copy-Assignment (deleted)
    /// @param other Instance to be copied
    OLED &operator=(const OLED &other) noexcept = delete;

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