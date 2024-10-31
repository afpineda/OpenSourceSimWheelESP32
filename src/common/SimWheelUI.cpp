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

// Flag colors
#define COLOR_BLACK_FLAG 0
#define COLOR_CHECKERED_FLAG 0
#define COLOR_BLUE_FLAG 0x0000FF
#define COLOR_GREEN_FLAG 0x00FF00
#define COLOR_ORANGE_FLAG 0xFF8000
#define COLOR_WHITE_FLAG 0xFFFFFF
#define COLOR_YELLOW_FLAG 0xFFFF00

// Rev lights display patterns
const uint8_t in_out_mode[] = {
    0b00000000,
    0b00011000,
    0b00111100,
    0b01111110,
    0b11111111};

const uint8_t out_in_mode[] = {
    0b00000000,
    0b10000001,
    0b11000011,
    0b11100111,
    0b11111111};

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
    uint8_t factoryAddress,
    revLightsMode_t displayMode)
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
    timer = 0;
    litCount = 0;
    blinkState = false;
    blink = false;
    displayBitePoint = false;
    this->displayMode = displayMode;

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
    if (displayBitePoint)
        return;
    if (pTelemetryData)
    {
        // litCount = (pTelemetryData->powertrain.rpmPercent * 8) / 100;
        litCount = CEIL_DIV(pTelemetryData->powertrain.rpmPercent * 8, 100);
        if (!blink && (pTelemetryData->powertrain.revLimiter))
        {
            timer = 0;
            blinkState = true;
        }
        blink = (pTelemetryData->powertrain.revLimiter);
    }
    else
        litCount = 0;
}

void PCF8574RevLights::serveSingleFrame(uint32_t elapsedMs)
{
    // Use timer
    if (displayBitePoint && (frameTimer(timer, elapsedMs, 2000) % 2 > 0))
    {
        displayBitePoint = false;
        litCount = 0;
    }
    else if (blink && (frameTimer(timer, elapsedMs, 60) % 2 > 0))
        blinkState = !blinkState;

    // Build LED state
    if (displayBitePoint)
    {
        // litCount = (userSettings::bitePoint * 8) / CLUTCH_FULL_VALUE;
        litCount = CEIL_DIV(userSettings::bitePoint * 8, CLUTCH_FULL_VALUE);
        write(~(0xFF << litCount));
    }
    else if (blink && !blinkState)
        write(0);
    else
    {
        uint8_t litCount2;
        switch (displayMode)
        {
        case RIGHT_TO_LEFT:
            write(0xFF << (8 - litCount));
            break;

        case IN_OUT:
            litCount2 = litCount >> 1; // = litCount / 2
            write(in_out_mode[litCount2]);
            break;

        case OUT_IN:
            litCount2 = litCount >> 1; // = litCount / 2
            write(out_in_mode[litCount2]);
            break;

        default: // LEFT_TO_RIGHT
            write(~(0xFF << litCount));
            break;
        }
    }
}

void PCF8574RevLights::onBitePoint()
{
    displayBitePoint = true;
    timer = 0;
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

//-----------------------------------------------------------------------------
// LED segment: race flags
//-----------------------------------------------------------------------------

uint32_t RaceFlagsLEDSegment::color_blackFlag = COLOR_BLACK_FLAG;
uint32_t RaceFlagsLEDSegment::color_checkeredFlag = COLOR_CHECKERED_FLAG;
uint32_t RaceFlagsLEDSegment::color_blueFlag = COLOR_BLUE_FLAG;
uint32_t RaceFlagsLEDSegment::color_greenFlag = COLOR_GREEN_FLAG;
uint32_t RaceFlagsLEDSegment::color_orangeFlag = COLOR_ORANGE_FLAG;
uint32_t RaceFlagsLEDSegment::color_whiteFlag = COLOR_WHITE_FLAG;
uint32_t RaceFlagsLEDSegment::color_yellowFlag = COLOR_YELLOW_FLAG;

RaceFlagsLEDSegment::RaceFlagsLEDSegment(
    LEDStripTelemetry *ledStripTelemetry,
    uint8_t pixelIndex,
    uint32_t blinkRateMs) : LEDSegment(ledStripTelemetry, false, false, true, false)
{
    this->pixelIndex = pixelIndex;
    this->blinkRateMs = blinkRateMs;
    blinkState = true; // Must always be true if blinkRateMs == 0
    update = false;
    litColor = 0;
    blinkTimer = 0;
    ledStripTelemetry->setPixelColor(pixelIndex, 0);
}

void RaceFlagsLEDSegment::onTelemetryData(
    const telemetryData_t *pTelemetryData,
    LEDSegmentToStripInterface &ledInterface)
{
    uint32_t newLitColor;
    if (pTelemetryData)
    {
        if (pTelemetryData->raceControl.blueFlag)
            newLitColor = RaceFlagsLEDSegment::color_blueFlag;
        else if (pTelemetryData->raceControl.yellowFlag)
            newLitColor = RaceFlagsLEDSegment::color_yellowFlag;
        else if (pTelemetryData->raceControl.whiteFlag)
            newLitColor = RaceFlagsLEDSegment::color_whiteFlag;
        else if (pTelemetryData->raceControl.greenFlag)
            newLitColor = RaceFlagsLEDSegment::color_greenFlag;
        else if (pTelemetryData->raceControl.orangeFlag)
            newLitColor = RaceFlagsLEDSegment::color_orangeFlag;
        else if (pTelemetryData->raceControl.blackFlag)
            newLitColor = RaceFlagsLEDSegment::color_blackFlag;
        else if (pTelemetryData->raceControl.checkeredFlag)
            newLitColor = RaceFlagsLEDSegment::color_checkeredFlag;
        else
            newLitColor = 0;
    }
    else
        newLitColor = 0;
    update = (newLitColor != litColor);
    litColor = newLitColor;
}

void RaceFlagsLEDSegment::serveSingleFrame(
    uint32_t elapsedMs,
    LEDSegmentToStripInterface &ledInterface)
{
    if (blinkRateMs && (frameTimer(blinkTimer, elapsedMs, blinkRateMs) % 2 > 0))
    {
        update = true;
        blinkState = !blinkState;
    }
    if (update)
    {
        update = false;
        if (blinkState || !blinkRateMs)
            ledInterface.setPixelColor(pixelIndex, litColor);
        else
            ledInterface.setPixelColor(pixelIndex, 0);
    }
}

//-----------------------------------------------------------------------------
// LED segment: ECU witness light
//-----------------------------------------------------------------------------

WitnessLEDSegment::WitnessLEDSegment(
    LEDStripTelemetry *ledStripTelemetry,
    uint8_t pixelIndex,
    witness_t witness1,
    uint32_t witness1Color,
    witness_t witness2,
    uint32_t witness2Color,
    witness_t witness3,
    uint32_t witness3Color) : LEDSegment(ledStripTelemetry, false, true, false, false)
{
    this->pixelIndex = pixelIndex;
    this->witness1 = witness1;
    this->witness2 = witness2;
    this->witness2 = witness3;
    this->witness1Color = witness1Color;
    this->witness2Color = witness2Color;
    this->witness3Color = witness3Color;
    ledStripTelemetry->setPixelColor(pixelIndex, 0);
}

bool witnessState(
    witness_t witness,
    const telemetryData_t *pTelemetryData)
{
    switch (witness)
    {
    case LOW_FUEL:
        return pTelemetryData->ecu.lowFuelAlert;
    case TC_ENGAGED:
        return pTelemetryData->ecu.tcEngaged;
    case ABS_ENGAGED:
        return pTelemetryData->ecu.absEngaged;
    case DRS_ENGAGED:
        return pTelemetryData->ecu.drsEngaged;
    case PIT_LIMITER:
        return pTelemetryData->ecu.pitLimiter;
    default:
        break;
    }
    return false;
}

void WitnessLEDSegment::onTelemetryData(
    const telemetryData_t *pTelemetryData,
    LEDSegmentToStripInterface &ledInterface)
{
    uint32_t newColor;
    if (pTelemetryData)
    {
        bool state = witnessState(witness1, pTelemetryData);
        if (state)
            newColor = witness1Color;
        else
        {
            state = witnessState(witness2, pTelemetryData);
            if (state)
                newColor = witness2Color;
            else
            {
                state = witnessState(witness3, pTelemetryData);
                if (state)
                    newColor = witness3Color;
                else
                    newColor = 0;
            }
        }
    }
    else
        newColor = 0;
    ledInterface.setPixelColor(pixelIndex, newColor);
}
