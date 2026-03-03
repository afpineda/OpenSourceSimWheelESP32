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

//-------------------------------------------------------------------
// GLOBALS
//-------------------------------------------------------------------

#define I2C_TIMEOUT_MS 30

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// LED strips
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

// LED Strips
static const rmt_transmit_config_t rmt_transmit_config = {
    .loop_count = 0,
    .flags = {
        .eot_level = 0,
        .queue_nonblocking = false}};

// ----------------------------------------------------------------------------

LEDStrip::LEDStrip(
    OutputGPIO dataPin,
    uint8_t pixelCount,
    bool useLevelShift,
    PixelDriver pixelType,
    PixelFormat pixelFormat)
{
    // Check parameters
    dataPin.reserve();
    if (pixelCount == 0)
        throw std::runtime_error("LEDStrip: pixel count can not be zero");

    // Compute pixel format when required
    if (pixelFormat == PixelFormat::AUTO)
    {
        switch (pixelType)
        {
        case PixelDriver::WS2811:
        case PixelDriver::UCS1903:
            pixelFormat = PixelFormat::RGB;
            break;
        default:
            pixelFormat = PixelFormat::GRB;
            break;
        }
    }

    // Configure RMT channel
    rmt_tx_channel_config_t tx_config = {
        .gpio_num = AS_GPIO(dataPin),
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10000000, // 10MHz resolution, 1 tick = 0.1us
        .mem_block_symbols = 128,
        .trans_queue_depth = 1,
        .intr_priority = 0,
        .flags = {
            .invert_out = 0,
            .with_dma = 1,
            .io_loop_back = 0,
            .io_od_mode = 0,
            .allow_pd = 0,
            .init_level = 0,
        }};
    if (useLevelShift)
        tx_config.flags.io_od_mode = 1;
    rmtHandle = nullptr;
    esp_err_t err = rmt_new_tx_channel(&tx_config, &rmtHandle);
    if (err == ESP_ERR_NOT_SUPPORTED)
    {
        tx_config.flags.with_dma = 0;
        err = rmt_new_tx_channel(&tx_config, &rmtHandle);
    }
    ESP_ERROR_CHECK_WITHOUT_ABORT(err);
    if (!rmtHandle)
        throw std::runtime_error("LEDStrip: rmt_new_tx_channel() failed");
    ESP_ERROR_CHECK(rmt_enable(rmtHandle));

    // Configure byte encoder
    rmt_bytes_encoder_config_t byte_enc_config = {};
    byte_enc_config.bit0.level0 = 1;
    byte_enc_config.bit0.level1 = 0;
    byte_enc_config.bit1.level0 = 1;
    byte_enc_config.bit1.level1 = 0;
    byte_enc_config.flags.msb_first = 1;
    switch (pixelType)
    {
    case PixelDriver::WS2811:
        byte_enc_config.bit0.duration0 = 5;
        byte_enc_config.bit0.duration1 = 20;
        byte_enc_config.bit1.duration0 = 12;
        byte_enc_config.bit1.duration1 = 13;
        break;
    case PixelDriver::WS2812:
    case PixelDriver::WS2815:
        byte_enc_config.bit0.duration0 = 3;
        byte_enc_config.bit0.duration1 = 9;
        byte_enc_config.bit1.duration0 = 9;
        byte_enc_config.bit1.duration1 = 3;
        break;
    case PixelDriver::SK6812:
        byte_enc_config.bit0.duration0 = 3;
        byte_enc_config.bit0.duration1 = 9;
        byte_enc_config.bit1.duration0 = 6;
        byte_enc_config.bit1.duration1 = 6;
        break;
    case PixelDriver::UCS1903:
        byte_enc_config.bit0.duration0 = 4;
        byte_enc_config.bit0.duration1 = 8;
        byte_enc_config.bit1.duration0 = 8;
        byte_enc_config.bit1.duration1 = 4;
        break;

    default:
        // Should not enter here
        throw std::runtime_error("Unknown pixel driver in LED strip");
        break;
    }
    ESP_ERROR_CHECK(rmt_new_bytes_encoder(&byte_enc_config, &encHandle));
    if (!encHandle)
        throw std::runtime_error("LEDStrip: rmt_new_bytes_encoder() failed");

    // Configure reset time (to show pixels)
    switch (pixelType)
    {
    case PixelDriver::WS2811:
        this->resetTimeNs = 50000; // 50 microseconds
        break;
    case PixelDriver::SK6812:
        this->resetTimeNs = 80000; // 80 microseconds
        break;
    case PixelDriver::UCS1903:
        this->resetTimeNs = 24000; // 24 microseconds
        break;
    default:
        this->resetTimeNs = 280000; // 280 microseconds
        break;
    }

    // Initialize instance
    this->pixelCount = pixelCount;
    this->pixelFormat = pixelFormat;
    this->pixelData = new uint8_t[pixelCount * 3];
    clear();
}

LEDStrip::~LEDStrip()
{
    if (rmtHandle)
    {
        ESP_ERROR_CHECK(rmt_disable(rmtHandle));
        ESP_ERROR_CHECK(rmt_del_channel(rmtHandle));
    }
    if (encHandle)
        ESP_ERROR_CHECK(rmt_del_encoder(encHandle));
    if (pixelData)
        delete pixelData;
}

//-----------------------------------------------------------------------------
// LED strip: display
//-----------------------------------------------------------------------------

void LEDStrip::show()
{
    if (changed)
    {
        changed = false;
        ESP_ERROR_CHECK(
            rmt_transmit(
                rmtHandle,
                encHandle,
                (const void *)pixelData,
                pixelCount * 3,
                &rmt_transmit_config));
        ESP_ERROR_CHECK(
            rmt_tx_wait_all_done(
                rmtHandle,
                -1));
        active_wait_ns(resetTimeNs);
        // internals::hal::gpio::wait_propagation(resetTimeNs);
    }
}

//-----------------------------------------------------------------------------
// LED Strip: Set pixel color
//-----------------------------------------------------------------------------

void LEDStrip::normalizeColor(uint8_t &r, uint8_t &g, uint8_t &b)
{
    // Normalize to a common brightness
    if (brightnessWeight)
    {
        // Note: "">> 8" is equal to "/ 256"
        r = (r * brightnessWeight) >> 8;
        g = (g * brightnessWeight) >> 8;
        b = (b * brightnessWeight) >> 8;
    }
}

//-----------------------------------------------------------------------------

void LEDStrip::rawPixelRGB(
    uint8_t pixelIndex,
    uint8_t redChannel,
    uint8_t greenChannel,
    uint8_t blueChannel)
{
    // Note: caller must check that pixelIndex is in range

    size_t dataIndex = (pixelIndex * 3);
    switch (pixelFormat)
    {
    case PixelFormat::BGR:
        pixelData[dataIndex++] = blueChannel;
        pixelData[dataIndex++] = greenChannel;
        pixelData[dataIndex] = redChannel;
        break;
    case PixelFormat::BRG:
        pixelData[dataIndex++] = blueChannel;
        pixelData[dataIndex++] = redChannel;
        pixelData[dataIndex] = greenChannel;
        break;
    case PixelFormat::GBR:
        pixelData[dataIndex++] = greenChannel;
        pixelData[dataIndex++] = blueChannel;
        pixelData[dataIndex] = redChannel;
        break;
    case PixelFormat::GRB:
        pixelData[dataIndex++] = greenChannel;
        pixelData[dataIndex++] = redChannel;
        pixelData[dataIndex] = blueChannel;
        break;
    case PixelFormat::RBG:
        pixelData[dataIndex++] = redChannel;
        pixelData[dataIndex++] = blueChannel;
        pixelData[dataIndex] = greenChannel;
        break;
    case PixelFormat::RGB:
        pixelData[dataIndex++] = redChannel;
        pixelData[dataIndex++] = greenChannel;
        pixelData[dataIndex] = blueChannel;
        break;
    default:
        return;
    }
    changed = true;
}

//-----------------------------------------------------------------------------

void LEDStrip::pixelRGB(
    uint8_t pixelIndex,
    uint8_t redChannel,
    uint8_t greenChannel,
    uint8_t blueChannel)
{
    if (pixelIndex < pixelCount)
    {
        normalizeColor(redChannel, greenChannel, blueChannel);
        rawPixelRGB(pixelIndex, redChannel, greenChannel, blueChannel);
    }
}

//-----------------------------------------------------------------------------

void LEDStrip::pixelRangeRGB(
    uint8_t fromPixelIndex,
    uint8_t toPixelIndex,
    uint8_t redChannel,
    uint8_t greenChannel,
    uint8_t blueChannel)
{
    normalizeColor(redChannel, greenChannel, blueChannel);
    for (uint8_t i = fromPixelIndex; (i <= toPixelIndex) && (i < pixelCount); i++)
        rawPixelRGB(i, redChannel, greenChannel, blueChannel);
}

//-----------------------------------------------------------------------------

void LEDStrip::shiftToNext()
{
    if (pixelCount > 1)
    {
        size_t lastPixelIndex = pixelCount - 1;
        uint8_t aux0 = pixelData[lastPixelIndex * 3];
        uint8_t aux1 = pixelData[(lastPixelIndex * 3) + 1];
        uint8_t aux2 = pixelData[(lastPixelIndex * 3) + 2];
        for (size_t pixelIndex = lastPixelIndex; (pixelIndex > 0); pixelIndex--)
        {
            uint8_t byteIndex = pixelIndex * 3;
            pixelData[byteIndex] = pixelData[byteIndex - 3];
            pixelData[byteIndex + 1] = pixelData[byteIndex - 2];
            pixelData[byteIndex + 2] = pixelData[byteIndex - 1];
        }
        pixelData[0] = aux0;
        pixelData[1] = aux1;
        pixelData[2] = aux2;
        changed = true;
    }
}

//-----------------------------------------------------------------------------

void LEDStrip::shiftToPrevious()
{
    if (pixelCount > 1)
    {
        uint8_t aux0 = pixelData[0];
        uint8_t aux1 = pixelData[1];
        uint8_t aux2 = pixelData[2];
        for (size_t pixelIndex = 1; pixelIndex < pixelCount; pixelIndex++)
        {
            uint8_t byteIndex = pixelIndex * 3;
            pixelData[byteIndex - 3] = pixelData[byteIndex];
            pixelData[byteIndex - 2] = pixelData[byteIndex + 1];
            pixelData[byteIndex - 1] = pixelData[byteIndex + 2];
        }
        size_t lastByteIndex = (pixelCount - 1) * 3;
        pixelData[lastByteIndex] = aux0;
        pixelData[lastByteIndex + 1] = aux1;
        pixelData[lastByteIndex + 2] = aux2;
        changed = true;
    }
}

//-----------------------------------------------------------------------------

void LEDStrip::clear()
{
    std::memset((void *)pixelData, 0, pixelCount * 3);
    changed = true;
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

OLEDBase::OLEDBase(uint8_t address7bits, I2CBus bus)
{
    internals::hal::i2c::require(bus);
    device = static_cast<void *>(
        internals::hal::i2c::add_device(address7bits, 4, bus));
}

OLEDBase::OLEDBase(::std::initializer_list<uint8_t> &&try_addresses, I2CBus bus)
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
        log_e("Unable to auto-detect OLED I2C address");
}

OLEDBase::~OLEDBase()
{
    internals::hal::i2c::remove_device(I2C_SLAVE(device));
}

bool OLEDBase::write_cmd(uint8_t command)
{
    uint8_t buffer[2];
    buffer[0] = 0x00; // CONTROL_COMMAND
    buffer[1] = command;
    esp_err_t err =
        i2c_master_transmit(I2C_SLAVE(device), buffer, 2, I2C_TIMEOUT_MS);
    last_i2c_result = (err == ESP_OK);
    return (err == ESP_OK);
}

bool OLEDBase::write_cmd(uint8_t command, uint8_t arg)
{
    uint8_t buffer[3];
    buffer[0] = 0x00; // CONTROL_COMMAND
    buffer[1] = command;
    buffer[2] = arg;
    esp_err_t err =
        i2c_master_transmit(I2C_SLAVE(device), buffer, 3, I2C_TIMEOUT_MS);
    last_i2c_result = (err == ESP_OK);
    return (err == ESP_OK);
}

bool OLEDBase::write_cmd(uint8_t command, uint8_t arg1, uint8_t arg2)
{
    uint8_t buffer[4];
    buffer[0] = 0x00; // CONTROL_COMMAND
    buffer[1] = command;
    buffer[2] = arg1;
    buffer[4] = arg2;
    esp_err_t err =
        i2c_master_transmit(I2C_SLAVE(device), buffer, 4, I2C_TIMEOUT_MS);
    last_i2c_result = (err == ESP_OK);
    return (err == ESP_OK);
}

bool OLEDBase::write_cmd(const uint8_t *buffer, ::std::size_t size)
{
    if (buffer && size)
    {
        uint8_t CONTROL_COMMAND = 0x00;
        i2c_master_transmit_multi_buffer_info_t info[2]{
            {
                .write_buffer = (uint8_t *)&CONTROL_COMMAND,
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
        last_i2c_result = (err == ESP_OK);
        return (err == ESP_OK);
    }
    return false;
}

bool OLEDBase::write_gdd_ram(const uint8_t *buffer, ::std::size_t size)
{
    if (buffer && size)
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
        last_i2c_result = (err == ESP_OK);
        return (err == ESP_OK);
    }
    return false;
}

bool OLEDBase::read_status(uint8_t &status)
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
    last_i2c_result = (err == ESP_OK);
    return (err == ESP_OK);
}

OLEDBase::Controller OLEDBase::guess_controller()
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
//
// Implementation heavily inspired by SS_OLED:
// https://github.com/bitbank2/ss_oled
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

OLED::OLED(
    I2CBus bus,
    OLED_resolution res)
    : OLEDBase({0b0111100, 0b0111101}, bus),
      _resolution{res}
{
    init();
}

OLED::OLED(
    uint8_t address7bits,
    I2CBus bus,
    OLED_resolution res)
    : OLEDBase(address7bits, bus),
      _resolution{res}
{
    init();
}

void OLED::init()
{
    // Characterize the display controller
    _controller = guess_controller();
    if (_controller == OLEDBase::Controller::UNKNOWN)
        // Fail early
        return;
    else if (_controller == OLED::Controller::SH1107)
        inverse_display(true);

    // Send the initialization sequence
    switch (_resolution)
    {
    case OLED_resolution::_128x128:
    {
        uint8_t init_seq[] = {0x00, 0xae, 0xdc, 0x00, 0x81, 0x40,
                              0xa1, 0xc8, 0xa8, 0x7f, 0xd5, 0x50,
                              0xd9, 0x22, 0xdb, 0x35, 0xb0, 0xda,
                              0x12, 0xa4, 0xa6, 0xaf};
        write_cmd(init_seq, sizeof(init_seq));
    }
    break;
    case OLED_resolution::_128x32:
    case OLED_resolution::_96x16:
    {
        uint8_t init_seq[] = {0x00, 0xae, 0xd5, 0x80, 0xa8, 0x1f,
                              0xd3, 0x00, 0x40, 0x8d, 0x14, 0xa1,
                              0xc8, 0xda, 0x02, 0x81, 0x7f, 0xd9,
                              0xf1, 0xdb, 0x40, 0xa4, 0xa6, 0xaf};
        write_cmd(init_seq, sizeof(init_seq));
    }
    break;
    case OLED_resolution::_72x40:
    {
        uint8_t init_seq[] = {0x00, 0xae, 0xa8, 0x3f, 0xd3, 0x00,
                              0x40, 0xa1, 0xc8, 0xda, 0x12, 0x81,
                              0xff, 0xad, 0x30, 0xd9, 0xf1, 0xa4,
                              0xa6, 0xd5, 0x80, 0x8d, 0x14, 0xaf,
                              0x20, 0x02};
        write_cmd(init_seq, sizeof(init_seq));
    }
    break;
    default:
    {
        uint8_t init_seq[] = {0x00, 0xae, 0xa8, 0x3f, 0xd3, 0x00,
                              0x40, 0xa1, 0xc8, 0xda, 0x12, 0x81,
                              0xff, 0xa4, 0xa6, 0xd5, 0x80, 0x8d,
                              0x14, 0xaf, 0x20, 0x02};
        write_cmd(init_seq, sizeof(init_seq));
    }
    break;
    } // switch

    // Compute width
    switch (_resolution)
    {
    case OLED_resolution::_64x32:
        _width = 64;
        break;
    case OLED_resolution::_96x16:
        _width = 96;
        break;
    case OLED_resolution::_72x40:
        _width = 72;
        break;
    default:
        _width = 128;
        break;
    } // switch

    // Compute height
    switch (_resolution)
    {
    case OLED_resolution::_128x128:
        _height = 128;
        break;
    case OLED_resolution::_128x32:
    case OLED_resolution::_64x32:
        _height = 32;
        break;
    case OLED_resolution::_96x16:
        _height = 16;
        break;
    case OLED_resolution::_72x40:
        _height = 40;
        break;
    default:
        _height = 64;
        break;
    } // switch

    // Initialize frame buffer
    clear();
} // OLED::init()

void OLED::inverse_display(bool yesOrNo)
{
    uint8_t cmd = 0xA6;
    if (yesOrNo)
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

void OLED::locate(uint8_t x, uint8_t y)
{
    _screen_offset = (y * 128) + x;
    switch (_resolution)
    {
    case OLED_resolution::_128x64:
        if (_controller == OLED::Controller::SH1106)
            // 128 pixels centered in 132 segments
            x += 2;
        break;
    case OLED_resolution::_96x16:
        // Display starts at line 2
        //    if (_flip)
        //         x += 32;
        //     else
        y += 2;
        break;
    case OLED_resolution::_72x40:
        // Display starts at 28,3
        x += 28;
        // if (_flip)
        // {
        //     y += 3;
        // }
        break;
    case OLED_resolution::_64x32:
        // visible display starts at column 32, row 4
        x += 32;
        // if (!_flip)
        //     non-flipped display starts from line 4
        //     y += 4;
        break;
    default:
        break;
    } // switch
    write_cmd((0xb0 | y), (x & 0xf), (0x10 | (x >> 4)));
}

void OLED::clear()
{
    memset(_frame, 0, sizeof(_frame));
}

void OLED::show()
{

} // OLED::show()

void OLED::setPixel(uint8_t x, uint8_t y, bool color)
{
    if ((x < _width) && (y < _height))
    {
        uint8_t page_index = (y >> 3); // = y/8;
        uint8_t bit_index = (y % 8);
        uint8_t byte_index = (page_index * _width) + x;
        if (color)
            _frame[byte_index] |= (1 << bit_index);
        else
            _frame[byte_index] &= ~(1 << bit_index);
    }
}