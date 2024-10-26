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

// Utility
#define CEIL_DIV(dividend, divisor) (dividend + divisor - 1) / divisor

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

SimpleShiftLight::~SimpleShiftLight()
{
    log_e("SimpleShiftLight instance was deleted");
    abort();
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

    // Turn LEDs off
    write(0x00);
}

PCF8574RevLights::~PCF8574RevLights()
{
    log_e("PCF8574RevLights instance was deleted");
    abort();
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
// LED Strip telemetry display
//-----------------------------------------------------------------------------

LEDStripTelemetry::LEDStripTelemetry(
    gpio_num_t dataPin,
    uint8_t pixelCount,
    bool useLevelShift,
    pixel_driver_t pixelType,
    pixel_format_t pixelFormat)
{
    this->ledStrip = new LEDStrip(dataPin, pixelCount, useLevelShift, pixelType, pixelFormat);
}

LEDStripTelemetry::~LEDStripTelemetry()
{
    delete this->ledStrip;
    log_e("An LEDStripTelemetry object was destroyed");
    abort();
}

void LEDStripTelemetry::onStart()
{
    this->ledStrip->pixelRangeRGB(0, 0xFF, 0x7F7F7F); // white
    this->ledStrip->show();
    vTaskDelay(pdMS_TO_TICKS(500));
    this->started = true;
}

void LEDStripTelemetry::onBLEdiscovering()
{
    this->ledStrip->pixelRangeRGB(0, 0xFF, 128, 0, 128); // purple
    this->ledStrip->show();
    vTaskDelay(pdMS_TO_TICKS(250));
}

void LEDStripTelemetry::onConnected()
{
    this->ledStrip->pixelRangeRGB(0, 0xFF, 0, 128, 0); // green
    this->ledStrip->show();
    vTaskDelay(pdMS_TO_TICKS(500));
    this->ledStrip->clear();
    this->ledStrip->show();
}

void LEDStripTelemetry::onLowBattery()
{
    // Flash 3 times in white
    for (int i = 0; i < 3; i++)
    {
        this->ledStrip->pixelRangeRGB(0, 0xFF, 0xFFFFFF); // white
        this->ledStrip->show();
        vTaskDelay(pdMS_TO_TICKS(250));
        this->ledStrip->clear();
        this->ledStrip->show();
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

void LEDStripTelemetry::onBitePoint()
{
    for (size_t i = 0; i < ledSegments.size(); i++)
        ledSegments[i]->onBitePoint(*this);
    this->ledStrip->show();
}

void LEDStripTelemetry::onTelemetryData(const telemetryData_t *pTelemetryData)
{
    for (size_t i = 0; i < ledSegments.size(); i++)
        ledSegments[i]->onTelemetryData(pTelemetryData, *this);
}

void LEDStripTelemetry::serveSingleFrame(uint32_t elapsedMs)
{
    for (size_t i = 0; i < ledSegments.size(); i++)
        ledSegments[i]->serveSingleFrame(elapsedMs, *this);
    this->ledStrip->show();
}

void LEDStripTelemetry::setPixelColor(
    uint8_t fromPixelIndex,
    uint8_t toPixelIndex,
    uint32_t packedRGB)
{
    this->ledStrip->pixelRangeRGB(fromPixelIndex, toPixelIndex, packedRGB);
}

void LEDStripTelemetry::setPixelColor(
    uint8_t pixelIndex,
    uint32_t packedRGB)
{
    this->ledStrip->pixelRGB(pixelIndex, packedRGB);
}

void LEDStripTelemetry::addSegment(
    LEDSegment *segment,
    bool requiresPowertrainTelemetry,
    bool requiresECUTelemetry,
    bool requiresRaceControlTelemetry,
    bool requiresGaugeTelemetry)
{
    if (started)
    {
        log_e("An LED segment was created after notify::begin()");
        abort();
    }
    else if (segment)
    {
        ledSegments.push_back(segment);
        this->requiresPowertrainTelemetry |= requiresPowertrainTelemetry;
        this->requiresECUTelemetry |= requiresECUTelemetry;
        this->requiresRaceControlTelemetry |= requiresRaceControlTelemetry;
        this->requiresGaugeTelemetry |= requiresGaugeTelemetry;
    }
}

//-----------------------------------------------------------------------------
// Abstract LED strip segment
//-----------------------------------------------------------------------------

LEDSegment::LEDSegment(
    LEDStripTelemetry *ledStripTelemetry,
    bool requiresPowertrainTelemetry,
    bool requiresECUTelemetry,
    bool requiresRaceControlTelemetry,
    bool requiresGaugeTelemetry)
{
    if (ledStripTelemetry)
    {
        ledStripTelemetry->addSegment(
            this,
            requiresPowertrainTelemetry,
            requiresECUTelemetry,
            requiresRaceControlTelemetry,
            requiresGaugeTelemetry);
    }
    else
    {
        log_e("LEDStripTelemetry object is null");
        abort();
    }
}

LEDSegment::~LEDSegment()
{
    log_e("An LED segment was destroyed.");
    abort();
}

//-----------------------------------------------------------------------------
// LED segment: shift light
//-----------------------------------------------------------------------------

ShiftLightLEDSegment::ShiftLightLEDSegment(
    LEDStripTelemetry *ledStripTelemetry,
    uint8_t pixelIndex,
    uint32_t maxTorqueColor,
    uint32_t maxRPMColor) : LEDSegment(ledStripTelemetry, true, false, false, false)
{
    this->pixelIndex = pixelIndex;
    this->maxTorqueColor = maxTorqueColor;
    this->maxRPMColor = maxRPMColor;
    this->blinkTimer = 0;
    this->blink = false;
    this->blinkState = false;
    ledStripTelemetry->setPixelColor(pixelIndex, 0);
}

void ShiftLightLEDSegment::onTelemetryData(
    const telemetryData_t *pTelemetryData,
    LEDSegmentToStripInterface &ledInterface)
{
    uint32_t currentColor;
    if (pTelemetryData == nullptr)
    {
        currentColor = 0;
        blink = false;
        blinkTimer = 0;
    }
    else if (pTelemetryData->powertrain.revLimiter)
    {
        currentColor = maxRPMColor;
        blink = true;
    }
    else if (pTelemetryData->powertrain.shiftLight2 > 0)
    {
        currentColor = maxRPMColor;
        blink = false;
        blinkTimer = 0;
    }
    else if (pTelemetryData->powertrain.shiftLight1 > 0)
    {
        currentColor = maxTorqueColor;
        blink = false;
        blinkTimer = 0;
    }
    else
    {
        currentColor = 0;
        blink = false;
        blinkTimer = 0;
    };
    ledInterface.setPixelColor(pixelIndex, currentColor);
}

void ShiftLightLEDSegment::serveSingleFrame(
    uint32_t elapsedMs,
    LEDSegmentToStripInterface &ledInterface)
{
    if (blink && (frameTimer(blinkTimer, elapsedMs, 60) % 2 > 0))
    {
        blinkState = !blinkState;
        if (blinkState)
            ledInterface.setPixelColor(pixelIndex, maxRPMColor);
        else
            ledInterface.setPixelColor(pixelIndex, 0);
    }
}

//-----------------------------------------------------------------------------
// LED segment: Rev Lights
//-----------------------------------------------------------------------------

RevLightsLEDSegment::RevLightsLEDSegment(
    LEDStripTelemetry *ledStripTelemetry,
    uint8_t firstPixelIndex,
    uint8_t pixelCount,
    uint32_t mainColor,
    uint32_t maxTorqueColor,
    uint32_t maxPowerColor,
    uint32_t bitePointColor) : LEDSegment(ledStripTelemetry, true, false, false, false)
{
    this->firstPixelIndex = firstPixelIndex;
    this->pixelCount = pixelCount;
    this->mainColor = mainColor;
    this->maxTorqueColor = maxTorqueColor;
    this->maxPowerColor = maxPowerColor;
    this->bitePointColor = bitePointColor;
    litCount = 0;
    litColor = 0;
    timer = 0;
    blinkState = false;
    blink = false;
    displayBitePoint = false;
    ledStripTelemetry->setPixelColor(firstPixelIndex, firstPixelIndex + pixelCount - 1, litColor);
}

void RevLightsLEDSegment::buildLEDs(LEDSegmentToStripInterface &ledInterface)
{
    if (displayBitePoint)
    {
        // litCount = (userSettings::bitePoint * pixelCount) / CLUTCH_FULL_VALUE;
        litCount = CEIL_DIV(userSettings::bitePoint * pixelCount, CLUTCH_FULL_VALUE);
        ledInterface.setPixelColor(firstPixelIndex, firstPixelIndex + litCount - 1, bitePointColor);
    }
    else
    {
        if (!blink || blinkState)
            ledInterface.setPixelColor(firstPixelIndex, firstPixelIndex + litCount - 1, litColor);
        else
            ledInterface.setPixelColor(firstPixelIndex, firstPixelIndex + litCount - 1, 0);
    }
    ledInterface.setPixelColor(firstPixelIndex + litCount, pixelCount - 1, 0);
}

void RevLightsLEDSegment::onTelemetryData(
    const telemetryData_t *pTelemetryData,
    LEDSegmentToStripInterface &ledInterface)
{
    if (displayBitePoint)
        return;
    if (pTelemetryData)
    {
        // litCount = (pTelemetryData->powertrain.rpmPercent * pixelCount) / 100;
        litCount = CEIL_DIV(pTelemetryData->powertrain.rpmPercent * pixelCount, 100);
        if (pTelemetryData->powertrain.shiftLight2)
            litColor = maxPowerColor;
        else if (pTelemetryData->powertrain.shiftLight1)
            litColor = maxTorqueColor;
        else
            litColor = mainColor;
        if (!blink && (pTelemetryData->powertrain.revLimiter))
        {
            timer = 0;
            blinkState = true;
        }
        blink = (pTelemetryData->powertrain.revLimiter);
    }
    else
        litCount = 0;
    buildLEDs(ledInterface);
}

void RevLightsLEDSegment::serveSingleFrame(
    uint32_t elapsedMs,
    LEDSegmentToStripInterface &ledInterface)
{
    if (displayBitePoint && (frameTimer(timer, elapsedMs, 2000) % 2 > 0))
    {
        displayBitePoint = false;
        litCount = 0;
        buildLEDs(ledInterface);
    }
    else if (blink && (frameTimer(timer, elapsedMs, 60) % 2 > 0))
    {
        blinkState = !blinkState;
        buildLEDs(ledInterface);
    }
}

void RevLightsLEDSegment::onBitePoint(
    LEDSegmentToStripInterface &ledInterface)
{
    if (bitePointColor)
    {
        displayBitePoint = true;
        timer = 0;
        buildLEDs(ledInterface);
    }
}