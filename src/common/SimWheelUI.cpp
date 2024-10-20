/**
 * @file SimWheelUI.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-10-09
 * @brief User interfaces.
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheelUI.h"
#include "i2cTools.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------

// SINGLE-LED display modes
#define MODE_OFF 0       // No light
#define MODE_ON 1        // Light
#define MODE_MAX_POWER 2 // Flash
#define MODE_MAX_RPM 3   // Flash quicker

// LED strips
#define PIXEL_TO_SYMBOL_COUNT(pixelCount) pixelCount * 3 * 8

//-----------------------------------------------------------------------------
// Single Color-Single LED user interface
//-----------------------------------------------------------------------------

SimpleShiftLight::SimpleShiftLight(gpio_num_t ledPin)
{
    // Check parameter
    if (!GPIO_IS_VALID_OUTPUT_GPIO(ledPin))
    {
        log_e("Requested GPIO %d can't be used as output", ledPin);
        abort();
    }

    // Configure GPIO
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT_OD;
    io_conf.pin_bit_mask = (1ULL << ledPin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Initialize
    this->requiresPowertrainTelemetry = true;
    this->ledPin = ledPin;
    this->blinkTimer = 0;
    this->ledMode = MODE_MAX_RPM;
    setMode(MODE_OFF);
}

void SimpleShiftLight::onStart()
{
    setLED(true);
    vTaskDelay(pdMS_TO_TICKS(100));
    setLED(false);
    vTaskDelay(pdMS_TO_TICKS(100));
    setLED(true);
    vTaskDelay(pdMS_TO_TICKS(200));
    setLED(false);
}

void SimpleShiftLight::onConnected()
{
    setLED(true);
    vTaskDelay(pdMS_TO_TICKS(250));
    setLED(false);
    vTaskDelay(pdMS_TO_TICKS(250));
    setLED(true);
    vTaskDelay(pdMS_TO_TICKS(250));
    setLED(false);
}

void SimpleShiftLight::onTelemetryData(
    const telemetryData_t *pTelemetryData)
{
    if (pTelemetryData == nullptr)
        setMode(MODE_OFF);
    else if (pTelemetryData->powertrain.revLimiter)
        setMode(MODE_MAX_RPM);
    else if (pTelemetryData->powertrain.shiftLight2 > 0)
        setMode(MODE_MAX_POWER);
    else if (pTelemetryData->powertrain.shiftLight1 > 0)
        setMode(MODE_ON);
    else
        setMode(MODE_OFF);
}

void SimpleShiftLight::serveSingleFrame(uint32_t elapsedMs)
{
    switch (ledMode)
    {
    case MODE_MAX_RPM:
        if (frameTimer(blinkTimer, elapsedMs, 60) % 2 > 0)
            swapLED();
        break;
    case MODE_MAX_POWER:
        if (frameTimer(blinkTimer, elapsedMs, 180) % 2 > 0)
            swapLED();
        break;
    default:
        break;
    }
}

void SimpleShiftLight::setLED(bool state)
{
    gpio_set_level(ledPin, !state); // Low = led ON
    ledState = state;
}

void SimpleShiftLight::setMode(uint8_t newMode)
{
    if (newMode == ledMode)
        return;
    blinkTimer = 0;
    if (newMode == MODE_OFF)
        setLED(false);
    else
        setLED(true);
    ledMode = newMode;
    log_e("setMode %d", ledMode);
}

//-----------------------------------------------------------------------------
// PCF8574-driven rev lights
//-----------------------------------------------------------------------------

PCF8574RevLights::PCF8574RevLights(
    uint8_t hardwareAddress,
    bool useSecondaryBus,
    uint8_t factoryAddress)
{
    // Announce required telemetry data
    requiresPowertrainTelemetry = true;

    // Check parameters
    i2c::abortOnInvalidAddress(hardwareAddress, 0, 7);
    i2c::abortOnInvalidAddress(factoryAddress >> 3, 0, 15);

    // Initialize
    i2c::require(1, useSecondaryBus);
    busDriver = i2c::getBus(useSecondaryBus);
    uint8_t address7Bits = ((factoryAddress & 0b01111000) | hardwareAddress);
    address8bits = address7Bits << 1;
    blinkTimer = 0;
    ledState = 0;
    isBlinking = false;
    forceUpdate = false;

    // Check chip availability
    if (!i2c::probe(address7Bits, useSecondaryBus))
    {
        log_e("PCF8574 chip not found. Bus: %d, Full address: %X (hex)", busDriver, address7Bits);
        abort();
    }

    // Turn leds off
    write(0x00);
}

void PCF8574RevLights::write(uint8_t state)
{
    state = ~state; // use negative logic
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, address8bits | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, state, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(busDriver, cmd, DEBOUNCE_TICKS);
    i2c_cmd_link_delete(cmd);
}

void PCF8574RevLights::onStart()
{
    write(0xFF);
    vTaskDelay(pdMS_TO_TICKS(100));
    write(0x00);
    vTaskDelay(pdMS_TO_TICKS(100));
    write(0xFF);
    vTaskDelay(pdMS_TO_TICKS(200));
    write(0x00);
}

void PCF8574RevLights::onConnected()
{
    write(0xFF);
    vTaskDelay(pdMS_TO_TICKS(250));
    write(0x00);
    vTaskDelay(pdMS_TO_TICKS(250));
    write(0xFF);
    vTaskDelay(pdMS_TO_TICKS(250));
    write(0x00);
}

void PCF8574RevLights::onTelemetryData(
    const telemetryData_t *pTelemetryData)
{
    uint8_t newLedState;
    if (pTelemetryData)
    {
        uint8_t aux = (pTelemetryData->powertrain.rpmPercent) * 8 / 100;
        newLedState = 0xFF >> (8 - aux);
        if (isBlinking ^ pTelemetryData->powertrain.revLimiter)
        {
            isBlinking = pTelemetryData->powertrain.revLimiter;
            blinkTimer = 0;
            blinkState = false;
            forceUpdate = true;
        }
    }
    else
    {
        newLedState = 0;
    }
    forceUpdate = (newLedState != ledState);
    ledState = newLedState;
}

void PCF8574RevLights::serveSingleFrame(uint32_t elapsedMs)
{
    if (isBlinking && (frameTimer(blinkTimer, elapsedMs, 60) % 2 > 0))
    {
        blinkState = !blinkState;
        forceUpdate = true;
    }
    if (forceUpdate)
    {
        if (blinkState)
            write(0x00);
        else
            write(ledState);
    }
}

//-----------------------------------------------------------------------------
// NeoPixel
//-----------------------------------------------------------------------------
//
// API REFERENCE:
// https://docs.espressif.com/projects/esp-idf/en/release-v5.1/esp32/api-reference/peripherals/rmt.html
//
// EXAMPLES:
// https://github.com/espressif/esp-idf/tree/master/examples/peripherals/rmt/led_strip/main
//
//-----------------------------------------------------------------------------

static rmt_transmit_config_t rmt_transmit_config = {
    .loop_count = 0,
    .flags = {
        .eot_level = 0,
        .queue_nonblocking = false}};

LEDStrip::LEDStrip(
    gpio_num_t dataPin,
    uint8_t pixelCount,
    pixel_type_t pixelType)
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

    // Compute buffer size
    int rawDataCount = PIXEL_TO_SYMBOL_COUNT(pixelCount);
    // The API requires an even number of block symbols
    if (rawDataCount % 2)
        rawDataCount++;

    // Configure RMT channel
    rmt_tx_channel_config_t tx_config = {
        .gpio_num = dataPin,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10000000, // 10MHz resolution, 1 tick = 0.1us
        .mem_block_symbols = rawDataCount,
        .trans_queue_depth = 1,
        .intr_priority = 0,
        .flags{
            .invert_out = 0,
            .with_dma = 0,
            .io_loop_back = 0,
            .io_od_mode = 0 // This is critical to level shifting
        }};
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_config, &rmtHandle));
    if (!rmtHandle)
        abort();

    // Configure byte encoder
    rmt_bytes_encoder_config_t byte_enc_config = {};
    byte_enc_config.bit0.level0 = 1;
    byte_enc_config.bit0.level1 = 0;
    byte_enc_config.bit1.level0 = 1;
    byte_enc_config.bit1.level1 = 0;
    switch (pixelType)
    {
    case PIXEL_WS2811:
        byte_enc_config.bit0.duration0 = 5;
        byte_enc_config.bit0.duration1 = 20;
        byte_enc_config.bit1.duration0 = 12;
        byte_enc_config.bit1.duration1 = 13;
        break;
    case PIXEL_WS2812:
        byte_enc_config.bit0.duration0 = 4;
        byte_enc_config.bit0.duration1 = 8;
        byte_enc_config.bit1.duration0 = 8;
        byte_enc_config.bit1.duration1 = 4;
        break;

    default:
        // Should not enter here
        abort();
        break;
    }
    ESP_ERROR_CHECK(rmt_new_bytes_encoder(&byte_enc_config, &encHandle));
    if (!encHandle)
        abort();

    // Initialize instance
    this->pixelCount = pixelCount;
    this->rawData = new uint8_t[pixelCount * 3];
    clear();
}

LEDStrip::~LEDStrip()
{
    ESP_ERROR_CHECK(rmt_del_encoder(encHandle));
    ESP_ERROR_CHECK(rmt_del_channel(rmtHandle));
}

void LEDStrip::show()
{
    if (changed)
    {
        changed = false;
        ESP_ERROR_CHECK(
            rmt_transmit(
                rmtHandle,
                encHandle,
                (const void *)rawData,
                pixelCount * 3,
                &rmt_transmit_config));
        ESP_ERROR_CHECK(
            rmt_tx_wait_all_done(
                rmtHandle,
                portMAX_DELAY));
        vTaskDelay(1);
    }
}

void LEDStrip::pixelColor(
    uint8_t pixelIndex,
    uint8_t redChannel,
    uint8_t greenChannel,
    uint8_t blueChannel)
{
    if (pixelIndex < pixelCount)
    {
        int rawIndex = (pixelIndex * 3);
        rawData[rawIndex++] = greenChannel;
        rawData[rawIndex++] = blueChannel;
        rawData[rawIndex] = redChannel;
        changed = true;
    }
}

void LEDStrip::pixelRangeColor(
    uint8_t fromPixelIndex,
    uint8_t toPixelIndex,
    uint8_t redChannel,
    uint8_t greenChannel,
    uint8_t blueChannel)
{
    if (fromPixelIndex > toPixelIndex)
        pixelRangeColor(
            toPixelIndex,
            fromPixelIndex,
            redChannel,
            greenChannel,
            blueChannel);
    else
    {
        for (uint8_t i = fromPixelIndex; i <= toPixelIndex; i++)
            pixelColor(i, redChannel, greenChannel, blueChannel);
        changed = true;
    }
}

void LEDStrip::clear()
{
    memset((void *)rawData, 0, pixelCount * 3);
    changed = true;
}
