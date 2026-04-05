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

#if CD_CI
#error Arduino only
#endif

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "OutputHardware.hpp"
#include "HAL.hpp"
#include "freertos/FreeRTOS.h" // for vTaskDelay()
#include <cstring>             // For memset()
#include "esp32-hal-log.h"     // For log_e()
#include <utility>             // For std::swap()
#include <numeric>             // std::gcd()

//-------------------------------------------------------------------
// GLOBALS
//-------------------------------------------------------------------

#define I2C_TIMEOUT_MS 30

/// @brief Rounded division to ceiling
#define CEIL_DIV(dividend, divisor) (dividend + divisor - 1) / divisor
/// @brief Quick division modulus = a % (b+1)
#define QMOD(a, b) ((a) & (b))

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Pixel vector
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void PixelVectorHelper::shift(
    PixelVectorHelper::vector_type &data,
    PixelVectorHelper::size_type from_index,
    PixelVectorHelper::size_type to_index,
    PixelVectorHelper::size_type shift)
{
    if (from_index >= data.size())
        from_index = data.size() - 1;
    if (to_index >= data.size())
        to_index = data.size() - 1;
    if (from_index > to_index)
    {
        // -- Shift down --
        PixelVectorHelper::size_type n = from_index - to_index + 1;
        shift = shift % n;
        if (shift > 0)
        {
            PixelVectorHelper::size_type num_cycles = ::std::gcd(n, shift);
            for (
                PixelVectorHelper::size_type start = 0;
                start < num_cycles;
                ++start)
            {
                Pixel temp = ::std::move(data[start + to_index]);
                PixelVectorHelper::size_type current = start;
                while (true)
                {
                    PixelVectorHelper::size_type next_index = current + shift;
                    if (next_index >= n)
                        next_index -= n;
                    if (next_index == start)
                        break;
                    data[current + to_index] =
                        ::std::move(data[next_index + to_index]);
                    current = next_index;
                }
                data[current + to_index] = ::std::move(temp);
            }
        }
    }
    else if (from_index < to_index)
    {
        // -- Shift up --
        PixelVectorHelper::size_type n = to_index - from_index + 1;
        PixelVectorHelper::size_type equivalent = (n - shift) % n;
        PixelVectorHelper::shift(data, to_index, from_index, equivalent);
    }
}

void PixelVectorHelper::shift_left(
    PixelVectorHelper::vector_type &data,
    PixelVectorHelper::size_type count) noexcept
{
    if (data.size() > 1)
        PixelVectorHelper::shift(data, data.size() - 1, 0, count);
}

void PixelVectorHelper::shift_right(
    PixelVectorHelper::vector_type &data,
    PixelVectorHelper::size_type count) noexcept
{
    if (data.size() > 1)
        PixelVectorHelper::shift(data, 0, data.size() - 1, count);
}

void PixelVectorHelper::fill(
    PixelVectorHelper::vector_type &data,
    const Pixel &color) noexcept
{
    for (size_type i = 0; i < data.size(); i++)
        data[i] = color;
}

void PixelVectorHelper::fill(
    PixelVectorHelper::vector_type &data,
    PixelVectorHelper::size_type fromIndex,
    PixelVectorHelper::size_type toIndex,
    const Pixel &color) noexcept
{
    if (fromIndex <= toIndex)
        for (size_type i = fromIndex;
             (i < data.size()) && (i <= toIndex);
             i++)
        {
            data[i] = color;
        }
    else
        PixelVectorHelper::fill(data, toIndex, fromIndex, color);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// LED strips
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

static const rmt_transmit_config_t rmt_transmit_config = {
    .loop_count = 0,
    .flags = {
        .eot_level = 0,
        .queue_nonblocking = false}};

// ----------------------------------------------------------------------------

size_t LEDStrip::pixels_rmt_encoder(
    const void *data,
    size_t data_size,
    size_t symbols_written,
    size_t symbols_free,
    rmt_symbol_word_t *symbols,
    bool *done,
    void *arg)
{
    size_t total_symbol_count = (data_size * symbols_per_byte);
    if (symbols_written >= total_symbol_count)
    {
        // Transaction finished
        *done = true;
        return 0;
    }
    else
    {
        LEDStrip *instance = static_cast<LEDStrip *>(arg);
        const Pixel *pixel_ptr = static_cast<const Pixel *>(data);
        size_t previous_symbols_written = symbols_written;
        size_t pixelIndex = (symbols_written / symbols_per_pixel);
        if (instance->reversed)
            pixelIndex = (data_size / 3) - (pixelIndex + 1);
        while (
            (symbols_free >= symbols_per_pixel) &&
            (symbols_written < total_symbol_count))
        {
            uint8_t byte[3];
            byte[0] =
                (pixel_ptr[pixelIndex].byte0(instance->pixelFormat) *
                 instance->brightnessWeight) >>
                8;
            byte[1] =
                (pixel_ptr[pixelIndex].byte1(instance->pixelFormat) *
                 instance->brightnessWeight) >>
                8;
            byte[2] =
                (pixel_ptr[pixelIndex].byte2(instance->pixelFormat) *
                 instance->brightnessWeight) >>
                8;

            for (size_t byteIndex = 0; byteIndex < 3; byteIndex++)
            {
                // Note: we write 8 bytes to symbols[] for each byte
                int bitIndex = (symbols_per_byte - 1);
                while ((bitIndex >= 0) && (bitIndex < symbols_per_byte))
                {
                    if ((1 << bitIndex) & byte[byteIndex])
                    {
                        // Bit 1
                        symbols[0].duration0 =
                            instance->byte_enc_config.bit1.duration0;
                        symbols[0].duration1 =
                            instance->byte_enc_config.bit1.duration1;
                        symbols[0].level0 =
                            instance->byte_enc_config.bit1.level0;
                        symbols[0].level1 =
                            instance->byte_enc_config.bit1.level1;
                    }
                    else
                    {
                        // Bit 0
                        symbols[0].duration0 =
                            instance->byte_enc_config.bit0.duration0;
                        symbols[0].duration1 =
                            instance->byte_enc_config.bit0.duration1;
                        symbols[0].level0 =
                            instance->byte_enc_config.bit0.level0;
                        symbols[0].level1 =
                            instance->byte_enc_config.bit0.level1;
                    }
                    symbols++;
                    bitIndex -= 1;
                }
            }
            symbols_written += symbols_per_pixel;
            symbols_free -= symbols_per_pixel;
            if (instance->reversed)
                pixelIndex--;
            else
                pixelIndex++;
        }
        // Note: when the return value is 0,
        // we ask for the rmt tx channel to free more buffer space
        return symbols_written - previous_symbols_written;
    }
} // pixels_rmt_encoder()

// ----------------------------------------------------------------------------

LEDStrip::LEDStrip(
    OutputGPIO dataPin,
    uint8_t pixelCount,
    bool useLevelShift,
    PixelDriver driver,
    bool reverse_display)
{
    // Check parameters
    dataPin.reserve();
    if (pixelCount == 0)
        throw std::runtime_error("LEDStrip: pixel count can not be zero");

    // Compute pixel format
    switch (driver)
    {
    case PixelDriver::WS2811:
    case PixelDriver::UCS1903:
    case PixelDriver::APA106:
        pixelFormat = PixelFormat::RGB;
        break;
    default:
        pixelFormat = PixelFormat::GRB;
        break;
    }

    // Configure the RMT channel
    rmt_tx_channel_config_t tx_config = {
        .gpio_num = AS_GPIO(dataPin),
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = clockResolutionHz,
        .mem_block_symbols = 64, // Note: must be even
        .trans_queue_depth = 1,
        .intr_priority = 0,
        .flags{
            .invert_out = 0,
            .with_dma = 1,
            .io_loop_back = 0,
            .io_od_mode = (useLevelShift) ? 1u : 0u,
            .allow_pd = 0}};
    esp_err_t err = rmt_new_tx_channel(&tx_config, &rmtHandle);
    // Check if there is no DMA support and fall back to PIO
    if (err == ESP_ERR_NOT_SUPPORTED)
    {
        tx_config.flags.with_dma = 0;
        err = rmt_new_tx_channel(&tx_config, &rmtHandle);
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(rmt_enable(rmtHandle));

    // Configure the byte encoder

    // Note: all supported display drivers use the same bit alignment
    // and bit levels
    byte_enc_config.bit0.level0 = 1;
    byte_enc_config.bit0.level1 = 0;
    byte_enc_config.bit1.level0 = 1;
    byte_enc_config.bit1.level1 = 0;
    byte_enc_config.flags.msb_first = 1;

    switch (driver)
    {
        // Note: all values in nanoseconds
    case PixelDriver::WS2811:
        byte_enc_config.bit0.duration0 = 500;
        byte_enc_config.bit0.duration1 = 2000;
        byte_enc_config.bit1.duration0 = 1200;
        byte_enc_config.bit1.duration1 = 1300;
        restTimeNs = 50000;
        break;
    case PixelDriver::WS2812:
    case PixelDriver::WS2815:
        byte_enc_config.bit0.duration0 = 300;
        byte_enc_config.bit0.duration1 = 900;
        byte_enc_config.bit1.duration0 = 900;
        byte_enc_config.bit1.duration1 = 300;
        restTimeNs = 280000;
        break;
    case PixelDriver::SK6812:
        byte_enc_config.bit0.duration0 = 300;
        byte_enc_config.bit0.duration1 = 900;
        byte_enc_config.bit1.duration0 = 600;
        byte_enc_config.bit1.duration1 = 600;
        restTimeNs = 80000;
        break;
    case PixelDriver::UCS1903:
        byte_enc_config.bit0.duration0 = 500;
        byte_enc_config.bit0.duration1 = 800;
        byte_enc_config.bit1.duration0 = 800;
        byte_enc_config.bit1.duration1 = 400;
        restTimeNs = 24000;
        break;
    case PixelDriver::APA106:
        byte_enc_config.bit0.duration0 = 350;
        byte_enc_config.bit0.duration1 = 1360;
        byte_enc_config.bit1.duration0 = 1360;
        byte_enc_config.bit1.duration1 = 350;
        restTimeNs = 50000;
        break;
    default:
        // should not enter here
        throw std::runtime_error("Unknown pixel driver in LED strip");
        break;
    }
    // Adapt to clock frequency
    byte_enc_config.bit0.duration0 /= 100;
    byte_enc_config.bit0.duration1 /= 100;
    byte_enc_config.bit1.duration0 /= 100;
    byte_enc_config.bit1.duration1 /= 100;

    // Create encoder
    rmt_simple_encoder_config_t cfg{
        .callback = pixels_rmt_encoder,
        .arg = (void *)this,
        .min_chunk_size = symbols_per_pixel};
    ESP_ERROR_CHECK(
        rmt_new_simple_encoder(
            &cfg,
            &pixel_encoder_handle));

    // Initialize other instance members
    this->reversed = reversed;
    this->pixelCount = pixelCount;
    clear();
}

LEDStrip::~LEDStrip()
{
    if (pixel_encoder_handle)
        ESP_ERROR_CHECK(rmt_del_encoder(pixel_encoder_handle));
    if (rmtHandle)
    {
        ESP_ERROR_CHECK(rmt_disable(rmtHandle));
        ESP_ERROR_CHECK(rmt_del_channel(rmtHandle));
    }
}

LEDStrip::LEDStrip(LEDStrip &&other)
{
    *this = ::std::forward<LEDStrip>(other);
}

LEDStrip &LEDStrip::operator=(LEDStrip &&other)
{
    ::std::swap(rmtHandle, other.rmtHandle);
    ::std::swap(pixel_encoder_handle, other.pixel_encoder_handle);
    ::std::swap(byte_enc_config, other.byte_enc_config);
    ::std::swap(pixelCount, other.pixelCount);
    ::std::swap(pixelFormat, other.pixelFormat);
    ::std::swap(brightnessWeight, other.brightnessWeight);
    ::std::swap(restTimeNs, other.restTimeNs);
    ::std::swap(reversed, other.reversed);
    return *this;
}

//-----------------------------------------------------------------------------
// LED strip: display
//-----------------------------------------------------------------------------

void LEDStrip::show(const LEDStrip::pixel_vector_type &pixels)
{
    if (pixels.size())
    {
        ESP_ERROR_CHECK(
            rmt_transmit(
                rmtHandle,
                pixel_encoder_handle,
                pixels.data(),
                pixels.size() * sizeof(Pixel),
                &rmt_transmit_config));
        ESP_ERROR_CHECK(
            rmt_tx_wait_all_done(
                rmtHandle,
                -1));
        active_wait_ns(restTimeNs);
    }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// PCF8574 LED Driver
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

PCF8574LedDriver::PCF8574LedDriver(
    I2CBus bus,
    uint8_t address7bits)
{
    internals::hal::i2c::abortOnInvalidAddress(address7bits);
    device = static_cast<void *>(
        internals::hal::i2c::add_device(
            address7bits,
            1,
            bus));
    _state = 0;
    show();
}

//-----------------------------------------------------------------------------

void PCF8574LedDriver::setLed(uint8_t index, bool state)
{
    if (index >= 8)
        return;
    if (state)
        _state |= (1 << index);
    else
        _state &= ~(1 << index);
}

//-----------------------------------------------------------------------------

void PCF8574LedDriver::shiftRight()
{
    uint8_t shifted = (_state << 1);
    if (_state & 0b10000000)
        _state = shifted | 0b00000001;
    else
        _state = shifted;
}

//-----------------------------------------------------------------------------

void PCF8574LedDriver::shiftLeft()
{
    uint8_t shifted = (_state >> 1);
    if (_state & 0b00000001)
        _state = shifted | 0b10000000;
    else
        _state = shifted;
}

//-----------------------------------------------------------------------------

void PCF8574LedDriver::show() const
{
    uint8_t state = ~_state; // use negative logic
    i2c_master_transmit(I2C_SLAVE(device), &state, 1, I2C_TIMEOUT_MS);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Single LED
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

SingleLED::SingleLED(OutputGPIO pin)
{
    pin.reserve();
    internals::hal::gpio::forOutput(pin, true, true);
    _pin = pin;
}

void SingleLED::show()
{
    GPIO_SET_LEVEL(_pin, !_state); // negative logic
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// OLEDBase
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

OLEDBase::OLEDBase(
    ::std::initializer_list<uint8_t> &&try_addresses,
    I2CBus bus) noexcept
{
    internals::hal::i2c::require(bus);
    for (uint8_t address7bits : try_addresses)
        if (internals::hal::i2c::probe(address7bits, bus))
        {
            device = static_cast<void *>(
                internals::hal::i2c::add_device(address7bits, 4, bus));
            return;
        }
    if (device == nullptr)
        log_e("I2C OLED not available");
}

OLEDBase::~OLEDBase() noexcept
{
    internals::hal::i2c::remove_device(I2C_SLAVE(device));
}

OLEDBase::OLEDBase(OLEDBase &&other) noexcept
{
    ::std::swap(device, other.device);
}

OLEDBase &OLEDBase::operator=(OLEDBase &&other) noexcept
{
    ::std::swap(device, other.device);
    return *this;
}

bool OLEDBase::write(const uint8_t *buffer, ::std::size_t size) const noexcept
{
    if (device)
    {
        esp_err_t err =
            i2c_master_transmit(
                I2C_SLAVE(device),
                buffer,
                size,
                I2C_TIMEOUT_MS);
        return (err == ESP_OK);
    }
    return false;
}

bool OLEDBase::write_cmd(uint8_t command) const noexcept
{
    if (device)
    {
        uint8_t buffer[2];
        buffer[0] = 0x00; // CONTROL_COMMAND
        buffer[1] = command;
        esp_err_t err =
            i2c_master_transmit(I2C_SLAVE(device), buffer, 2, I2C_TIMEOUT_MS);
        return (err == ESP_OK);
    }
    return false;
}

bool OLEDBase::write_cmd(uint8_t command, uint8_t arg) const noexcept
{
    if (device)
    {
        uint8_t buffer[3];
        buffer[0] = 0x00; // CONTROL_COMMAND
        buffer[1] = command;
        buffer[2] = arg;
        esp_err_t err =
            i2c_master_transmit(I2C_SLAVE(device), buffer, 3, I2C_TIMEOUT_MS);
        return (err == ESP_OK);
    }
    return false;
}

bool OLEDBase::write_cmd(
    uint8_t command,
    uint8_t arg1,
    uint8_t arg2) const noexcept
{
    if (device)
    {
        uint8_t buffer[4];
        buffer[0] = 0x00; // CONTROL_COMMAND
        buffer[1] = command;
        buffer[2] = arg1;
        buffer[3] = arg2;
        esp_err_t err =
            i2c_master_transmit(I2C_SLAVE(device), buffer, 4, I2C_TIMEOUT_MS);
        return (err == ESP_OK);
    }
    return false;
}

bool OLEDBase::write_gdd_ram(
    const uint8_t *buffer,
    ::std::size_t size) const noexcept
{
    if (device && buffer && size)
    {
        uint8_t CONTROL_DATA = 0x40;
        i2c_master_transmit_multi_buffer_info_t info[2]{
            {
                .write_buffer = (uint8_t *)&CONTROL_DATA,
                .buffer_size = 1,
            },
            {
                .write_buffer = const_cast<uint8_t *>(buffer),
                .buffer_size = size,
            }};
        esp_err_t err =
            i2c_master_multi_buffer_transmit(
                I2C_SLAVE(device),
                info,
                2,
                I2C_TIMEOUT_MS);
        return (err == ESP_OK);
    }
    return false;
}

bool OLEDBase::read_status(uint8_t &status) const noexcept
{
    if (device)
    {
        uint8_t CONTROL_COMMAND = 0x00;
        esp_err_t err =
            i2c_master_transmit_receive(
                I2C_SLAVE(device),
                (uint8_t *)&CONTROL_COMMAND,
                1,
                (uint8_t *)&status,
                1,
                I2C_TIMEOUT_MS);
        return (err == ESP_OK);
    }
    return false;
}

OLEDBase::Controller OLEDBase::guess_controller() const noexcept
{
    // Characterize the display controller
    uint8_t status;
    if (read_status(status))
    {
        status &= 0x0F;
        if (status == 0x08)
            // Note: 132 segments
            return OLEDBase::Controller::SH1106;
        else if (status == 3 || status == 6)
            // Note: 128 segments
            return OLEDBase::Controller::SSD1306;
        else if (status == 0x07 || status == 0x0F)
            // Note: 128 segments
            return OLEDBase::Controller::SH1107;
    }
    return OLEDBase::Controller::UNKNOWN;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// OLED
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

OLED::OLED(
    const OLEDParameters &params,
    I2CBus bus)
    : OLEDBase({0b0111100, 0b0111101}, bus),
      _params{params}
{
    init();
}

OLED::OLED(
    const OLEDParameters &params,
    uint8_t address7bits,
    I2CBus bus)
    : OLEDBase({address7bits}, bus),
      _params{params}
{
    init();
}

void OLED::init()
{
    // -- Initialization sequence
    enable_display(false);
    // Set MUX ratio to screen height-1
    write_cmd(0xA8, _params.screen_height - 1);
    // Set display offset
    write_cmd(0xD3, _params.display_offset);
    // Set display start line
    write_cmd((_params.start_row & 0b111111) | 0x40);
    // Set segment remap
    uint8_t aux = (_params.flip_horizontal) ? 0xA1 : 0xA0;
    write_cmd(aux);
    // Set COM output direction
    aux = (_params.flip_vertical) ? 0xC8 : 0xC0;
    write_cmd(aux);
    // Set COM pins
    write_cmd(0xDA, (_params.COMpins & 0b00110000) | 0b10);
    // Set contrast
    write_cmd(0x81, _params.contrast);
    // Set pixel inversion
    aux = (_params.inverted_display) ? 0xA7 : 0xA6;
    write_cmd(aux);
    // Set oscillator freq
    write_cmd(0xD5, _params.oscillator_frequency);
    // Enable charge pump
    write_cmd(0x8D, 0x14);
    // Set addresing mode to "page"
    write_cmd(0x20, 0b10);
    // Turn on
    enable_display(true);
    turn(true);

    // Compute screen size in bytes
    width_b = CEIL_DIV(_params.screen_width, 8);
    height_b = CEIL_DIV(_params.screen_height, 8);
} // OLED::init()

void OLED::inverse_display(bool yesOrNo)
{
    uint8_t cmd = 0xA6;
    if (yesOrNo ^ _params.inverted_display)
        cmd++;
    write_cmd(cmd);
}

void OLED::contrast(uint8_t value)
{
    write_cmd(0x81, value);
}

void OLED::turn(bool onOrOff)
{
    if (onOrOff)
        // turn on
        write_cmd(0xAF);
    else
        // turn off
        write_cmd(0xAE);
}

void OLED::enable_display(bool yesOrNo)
{
    if (yesOrNo)
        // Display RAM contents
        write_cmd(0xA4);
    else
        // Do not display RAM contents
        write_cmd(0xA5);
}

void OLED::locate(uint8_t x, uint8_t page)
{
    uint8_t transmit_buffer[] =
        {
            // Control byte
            0x80,
            // set page
            static_cast<uint8_t>(0xB0 | (page & 0b111)),
            // Control byte
            0x80,
            // Set column : lower 4 bits
            static_cast<uint8_t>(x & 0b1111),
            // Control byte
            0x00,
            // Set column : higher 4 bits
            static_cast<uint8_t>((x >> 4) | 0x10),
        };
    write(transmit_buffer, sizeof(transmit_buffer));
}

void OLED::clear(bool inverted)
{
    uint8_t aux[_params.screen_width];
    memset(aux, (inverted) ? 0xFF : 0, sizeof(aux));
    uint8_t page_cnt = _params.screen_height >> 3;
    for (uint8_t pIndex = 0; pIndex < page_cnt; pIndex++)
    {
        locate(_params.start_col, pIndex);
        write_gdd_ram(aux, sizeof(aux));
    }
}

uint8_t OLED::row2col(uint8_t bit_index, const uint8_t *from, uint8_t row_count)
{
    uint8_t result = 0;
    uint8_t mask = (0x80 >> bit_index);
    for (uint8_t n = 0; n < row_count; n++)
    {
        uint8_t bit;
        if (from[(n * width_b)] & mask)
            bit = (1 << n);
        else
            bit = 0;
        result |= bit;
    }
    return result;
}

void OLED::show(const uint8_t *frame)
{
    if (frame)
    {
        uint8_t aux[_params.screen_width];
        const uint8_t *frame_pos = frame;
        const uint8_t last_page = height_b - 1;
        for (uint8_t page = 0; page <= last_page; page++)
        {
            frame_pos = frame + (page * 8 * width_b);
            uint8_t row_count;
            if (page == last_page)
                row_count = _params.screen_height - (last_page * 8);
            else
                row_count = 8;

            for (uint8_t segment = 0; segment < _params.screen_width; segment++)
            {
                uint8_t byte_index = segment >> 3;
                uint8_t bit_index = QMOD(segment, 7);
                aux[segment] =
                    row2col(bit_index, frame_pos + byte_index, row_count);
            }
            locate(_params.start_col, page);
            write_gdd_ram(aux, sizeof(aux));
        }
    }
}
