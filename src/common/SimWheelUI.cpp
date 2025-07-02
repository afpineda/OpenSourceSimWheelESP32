/**
 * @file SimWheelUI.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-10-09
 * @brief User interfaces for telemetry data and notifications.
 *
 * @copyright Licensed under the EUPL
 *
 */

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "SimWheelUI.hpp"
#include "HAL.hpp"
#include "InternalServices.hpp"

//-------------------------------------------------------------------
// GLOBALS
//-------------------------------------------------------------------

// SINGLE-LED display modes
#define MODE_OFF 0       // No light
#define MODE_ON 1        // Light
#define MODE_MAX_POWER 2 // Flash
#define MODE_MAX_RPM 3   // Flash quicker

// Utility
#define CEIL_DIV(dividend, divisor) (dividend + divisor - 1) / divisor

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

SimpleShiftLight::SimpleShiftLight(OutputGPIO ledPin)
{
    led = new SingleLED(ledPin);

    // Initialize
    this->requiresPowertrainTelemetry = true;
    this->blinkTimer = 0;
    setMode(MODE_OFF);
}

SimpleShiftLight::~SimpleShiftLight()
{
    delete led;
}

void SimpleShiftLight::onStart()
{
    int soc;
    if (BatteryService::call::hasBattery())
        soc = BatteryService::call::getLastBatteryLevel();
    else
        soc = 100;

    int blinkCount = ((100 - soc) / 10) + 1;
    int blinkTimeMs = 1500 / blinkCount;

    for (int i = 0; i < blinkCount; i++)
    {
        led->set(true);
        led->show();
        DELAY_MS(blinkTimeMs);
        led->set(false);
        led->show();
        DELAY_MS(100);
    }
    DELAY_MS(400);
}

void SimpleShiftLight::onConnected()
{
    led->set(true);
    led->show();
    DELAY_MS(250);
    led->set(false);
    led->show();
    DELAY_MS(250);
    led->set(true);
    led->show();
    DELAY_MS(250);
    led->set(false);
    led->show();
}

void SimpleShiftLight::onTelemetryData(
    const TelemetryData *pTelemetryData)
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
            led->swap();
        break;
    case MODE_MAX_POWER:
        if (frameTimer(blinkTimer, elapsedMs, 180) % 2 > 0)
            led->swap();
        break;
    default:
        break;
    }
    led->show();
}

void SimpleShiftLight::setMode(uint8_t newMode)
{
    if (newMode == ledMode)
        return;
    blinkTimer = 0;
    if (newMode == MODE_OFF)
        led->set(false);
    else
        led->set(true);
    ledMode = newMode;
}

void SimpleShiftLight::shutdown()
{
    led->set(false);
    led->show();
}

//-----------------------------------------------------------------------------
// PCF8574-driven rev lights
//-----------------------------------------------------------------------------

PCF8574RevLights::PCF8574RevLights(
    uint8_t hardwareAddress,
    I2CBus bus,
    uint8_t factoryAddress,
    RevLightsMode displayMode)
{
    // Announce required telemetry data
    requiresPowertrainTelemetry = true;

    // Check parameters
    internals::hal::i2c::abortOnInvalidAddress(hardwareAddress, 0, 7);
    internals::hal::i2c::abortOnInvalidAddress(factoryAddress >> 3, 0, 15);

    // Initialize
    uint8_t address7Bits = ((factoryAddress & 0b01111000) | hardwareAddress);
    this->driver = new PCF8574LedDriver(bus, address7Bits);
    timer = 0;
    litCount = 0;
    blinkState = false;
    blink = false;
    displayBitePoint = false;
    this->displayMode = displayMode;

    // Turn LEDs off
    driver->setState(0);
    driver->show();
}

PCF8574RevLights::~PCF8574RevLights()
{
    delete driver;
}

void PCF8574RevLights::onStart()
{
    int soc;
    if (BatteryService::call::hasBattery())
        soc = BatteryService::call::getLastBatteryLevel();
    else
        soc = 100;

    driver->setState(0x00);
    for (int i = 0; i < soc / 8; i++)
        driver->setLed(i, true);
    driver->show();
    DELAY_MS(1500);
    driver->setState(0x00);
    driver->show();
    DELAY_MS(500);
}

void PCF8574RevLights::onConnected()
{
    driver->setState(0xFF);
    driver->show();
    DELAY_MS(250);
    driver->setState(0x00);
    driver->show();
    DELAY_MS(250);
    driver->setState(0xFF);
    driver->show();
    DELAY_MS(250);
    driver->setState(0x00);
    driver->show();
}

void PCF8574RevLights::onTelemetryData(
    const TelemetryData *pTelemetryData)
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
        litCount = CEIL_DIV(lastBitePoint * 8, CLUTCH_FULL_VALUE);
        driver->setState(~(0xFF << litCount));
    }
    else if (blink && !blinkState)
        driver->setState(0);
    else
    {
        uint8_t litCount2;
        switch (displayMode)
        {
        case RevLightsMode::RIGHT_TO_LEFT:
            driver->setState(0xFF << (8 - litCount));
            break;

        case RevLightsMode::IN_OUT:
            litCount2 = litCount >> 1; // = litCount / 2
            driver->setState(in_out_mode[litCount2]);
            break;

        case RevLightsMode::OUT_IN:
            litCount2 = litCount >> 1; // = litCount / 2
            driver->setState(out_in_mode[litCount2]);
            break;

        default: // LEFT_TO_RIGHT
            driver->setState(~(0xFF << litCount));
            break;
        }
    }
    driver->show();
}

void PCF8574RevLights::onBitePoint(uint8_t bitePoint)
{
    displayBitePoint = true;
    lastBitePoint = bitePoint;
    timer = 0;
}

void PCF8574RevLights::onLowBattery()
{
    driver->setState(0b01010101);
    driver->show();
    DELAY_MS(150);
    driver->setState(0b10101010);
    driver->show();
    DELAY_MS(150);
    driver->setState(0b01010101);
    driver->show();
    DELAY_MS(150);
    driver->setState(0b10101010);
    driver->show();
    DELAY_MS(150);
    driver->setState(0x00);
    driver->show();
}

void PCF8574RevLights::onSaveSettings()
{
    driver->setState(0b10000001);
    driver->show();
    DELAY_MS(100);
    driver->setState(0b01000010);
    driver->show();
    DELAY_MS(100);
    driver->setState(0b00100100);
    driver->show();
    DELAY_MS(100);
    driver->setState(0b00011000);
    driver->show();
    DELAY_MS(100);
    driver->setState(0x00);
    driver->show();
}

void PCF8574RevLights::shutdown()
{
    driver->setState(0);
    driver->show();
}
