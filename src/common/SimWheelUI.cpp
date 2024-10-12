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

#define MODE_OFF 0
#define MODE_ON 1
#define MODE_MAX_POWER 2
#define MODE_MAX_RPM 3

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
    io_conf.mode = GPIO_MODE_OUTPUT;
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
    // Check parameters
    i2c::abortOnInvalidAddress(hardwareAddress, 0, 7);
    i2c::abortOnInvalidAddress(factoryAddress >> 3, 0, 15);

    // Initialize
    i2c::require(1, useSecondaryBus);
    busDriver = i2c::getBus(useSecondaryBus);
    address8bits = ((factoryAddress & 0b01111000) | hardwareAddress) << 1;
    blinkTimer = 0;
    ledState = 0;
    isBlinking = false;
    forceUpdate = false;
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
    uint8_t aux = (pTelemetryData->powertrain.rpmPercent) * 8 / 100;
    uint8_t newLedState = 0xFF >> (8 - aux);
    if (isBlinking ^ pTelemetryData->powertrain.revLimiter)
    {
        isBlinking = pTelemetryData->powertrain.revLimiter;
        blinkTimer = 0;
        blinkState = false;
        forceUpdate = true;
    }
    else
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
