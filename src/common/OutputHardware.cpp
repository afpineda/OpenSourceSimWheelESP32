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

//-------------------------------------------------------------------
// GLOBALS
//-------------------------------------------------------------------

// #define AS_GPIO(pin) static_cast<gpio_num_t>((int)(pin))
#define I2C_TIMEOUT_TICKS pdMS_TO_TICKS(30)

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// LED strips
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

// LED Strips
#define PIXEL_TO_SYMBOL_COUNT(pixelCount) pixelCount * 3 * 8
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

    // Compute buffer size
    size_t rawDataCount = PIXEL_TO_SYMBOL_COUNT(pixelCount);
    // The API requires an even number of block symbols
    if (rawDataCount % 2)
        rawDataCount++;
    // The API requires 64 bytes minimum
    if (rawDataCount < 64)
        rawDataCount = 64;

    // Configure RMT channel
    rmt_tx_channel_config_t tx_config = {
        .gpio_num = AS_GPIO(dataPin),
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10000000, // 10MHz resolution, 1 tick = 0.1us
        .mem_block_symbols = rawDataCount,
        .trans_queue_depth = 4,
        .intr_priority = 0,
        .flags{
            .invert_out = 0,
            .with_dma = 0,
            .io_loop_back = 0,
            .io_od_mode = 0}};
    if (useLevelShift)
    {
        tx_config.flags.io_od_mode = 1;
    }
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_config, &rmtHandle));
    if (!rmtHandle)
        abort();
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
        abort();

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
    ESP_ERROR_CHECK(rmt_disable(rmtHandle));
    ESP_ERROR_CHECK(rmt_del_encoder(encHandle));
    ESP_ERROR_CHECK(rmt_del_channel(rmtHandle));
    delete this->pixelData;
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
        internals::hal::gpio::wait_propagation(resetTimeNs);
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
    internals::hal::i2c::require(1, bus); // PCF8574 requires x1 speed
    _address8bit = address7bits << 1;
    _state = 0;
    _bus = bus;
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
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, _address8bit | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, state, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(AS_PORT(_bus), cmd, I2C_TIMEOUT_TICKS);
    i2c_cmd_link_delete(cmd);
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