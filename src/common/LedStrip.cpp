/**
 * @file LedStrip.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-10-24
 * @brief Interface to single-wire LED strips
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "LedStrip.h"
#include "esp32-hal.h"
#include "driver/gpio.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------

#define PIXEL_TO_SYMBOL_COUNT(pixelCount) pixelCount * 3 * 8
static rmt_transmit_config_t rmt_transmit_config = {
    .loop_count = 0,
    .flags = {
        .eot_level = 0,
        .queue_nonblocking = false}};

// API REFERENCE:
// https://docs.espressif.com/projects/esp-idf/en/release-v5.1/esp32/api-reference/peripherals/rmt.html
//
// EXAMPLES:
// https://github.com/espressif/esp-idf/tree/master/examples/peripherals/rmt/led_strip/main

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------

LEDStrip::LEDStrip(
    gpio_num_t dataPin,
    uint8_t pixelCount,
    bool useLevelShift,
    pixel_driver_t pixelType,
    pixel_format_t pixelFormat)
{
    // Check parameters
    if (!GPIO_IS_VALID_OUTPUT_GPIO(dataPin))
    {
        log_e("Requested GPIO %d can't be used as output", dataPin);
        abort();
    }
    if (pixelCount == 0)
    {
        log_e("LEDStrip: pixel count can not be zero");
        abort();
    }

    // Compute pixel format when required
    if (pixelFormat == pixel_format_t::AUTO)
    {
        switch (pixelType)
        {
        case WS2811:
        case UCS1903:
            pixelFormat = pixel_format_t::RGB;
            break;
        default:
            pixelFormat = pixel_format_t::GRB;
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
        .gpio_num = dataPin,
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
    case WS2811:
        byte_enc_config.bit0.duration0 = 5;
        byte_enc_config.bit0.duration1 = 20;
        byte_enc_config.bit1.duration0 = 12;
        byte_enc_config.bit1.duration1 = 13;
        break;
    case WS2812:
    case WS2815:
        byte_enc_config.bit0.duration0 = 3;
        byte_enc_config.bit0.duration1 = 9;
        byte_enc_config.bit1.duration0 = 9;
        byte_enc_config.bit1.duration1 = 3;
        break;
    case SK6812:
        byte_enc_config.bit0.duration0 = 3;
        byte_enc_config.bit0.duration1 = 9;
        byte_enc_config.bit1.duration0 = 6;
        byte_enc_config.bit1.duration1 = 6;
        break;
    case UCS1903:
        byte_enc_config.bit0.duration0 = 4;
        byte_enc_config.bit0.duration1 = 8;
        byte_enc_config.bit1.duration0 = 8;
        byte_enc_config.bit1.duration1 = 4;
        break;

    default:
        // Should not enter here
        log_e("Unknown pixel driver %d in LED strip", pixelType);
        abort();
        break;
    }
    ESP_ERROR_CHECK(rmt_new_bytes_encoder(&byte_enc_config, &encHandle));
    if (!encHandle)
        abort();

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
// Display
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
        vTaskDelay(1);
    }
}

//-----------------------------------------------------------------------------
// Set pixel color
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
    case pixel_format_t::BGR:
        pixelData[dataIndex++] = blueChannel;
        pixelData[dataIndex++] = greenChannel;
        pixelData[dataIndex] = redChannel;
        break;
    case pixel_format_t::BRG:
        pixelData[dataIndex++] = blueChannel;
        pixelData[dataIndex++] = redChannel;
        pixelData[dataIndex] = greenChannel;
        break;
    case pixel_format_t::GBR:
        pixelData[dataIndex++] = greenChannel;
        pixelData[dataIndex++] = blueChannel;
        pixelData[dataIndex] = redChannel;
        break;
    case pixel_format_t::GRB:
        pixelData[dataIndex++] = greenChannel;
        pixelData[dataIndex++] = redChannel;
        pixelData[dataIndex] = blueChannel;
        break;
    case pixel_format_t::RBG:
        pixelData[dataIndex++] = redChannel;
        pixelData[dataIndex++] = blueChannel;
        pixelData[dataIndex] = greenChannel;
        break;
    case pixel_format_t::RGB:
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
    memset((void *)pixelData, 0, pixelCount * 3);
    changed = true;
}

//-----------------------------------------------------------------------------
