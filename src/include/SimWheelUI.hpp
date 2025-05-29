/**
 * @file SimWheelUI.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-10-09
 * @brief User interfaces for telemetry data and notifications
 *
 * @copyright Licensed under the EUPL
 *
 */

#pragma once

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "OutputHardware.hpp"

//-------------------------------------------------------------------
// Auxiliary types
//-------------------------------------------------------------------

/**
 * @brief Display modes for "rev lights"
 *
 */
enum class RevLightsMode : uint8_t
{
    /// @brief Moving from left to right
    LEFT_TO_RIGHT,
    /// @brief Moving from right to left
    RIGHT_TO_LEFT,
    /// @brief Moving from center to edges
    IN_OUT,
    /// @brief Moving from edges to center
    OUT_IN
};

//-----------------------------------------------------------------------------
// Single Color-Single LED user interface
//-----------------------------------------------------------------------------

/**
 * @brief Use a single-color LED as a shift light indicator
 *
 */
class SimpleShiftLight : public AbstractUserInterface
{
public:
    /**
     * @brief Create a simple "shift" light.
     *
     * @param ledPin GPIO pin where the LED is attached to.
     */
    SimpleShiftLight(OutputGPIO ledPin);
    ~SimpleShiftLight();

    virtual void onStart() override;
    virtual void onConnected() override;
    virtual void onTelemetryData(const TelemetryData *pTelemetryData) override;
    virtual void serveSingleFrame(uint32_t elapsedMs) override;
    virtual void shutdown() override;
    virtual uint8_t getMaxFPS() override { return 60; }
    virtual uint16_t getStackSize() override { return 1024; }

private:
    SingleLED *led;
    uint8_t ledMode;
    uint32_t blinkTimer;
    void setMode(uint8_t newMode);
};

//-----------------------------------------------------------------------------
// PCF8574-driven rev lights
//-----------------------------------------------------------------------------

/**
 * @brief Use eight single-color LEDS as "rev lights"
 *
 */
class PCF8574RevLights : public AbstractUserInterface
{
public:
    /**
     * @brief Create "rev lights" using PCF8574 and
     *        single-color LEDs
     *
     * @param hardwareAddress An I2C hardware address (3 bits), as configured
     *                        using pins A0, A1 and A2.
     * @param bus             I2C bus.
     * @param factoryAddress  Fixed factory-defined part of the full I2C address (7 bits).
     * @param displayMode     Display mode.
     */
    PCF8574RevLights(
        uint8_t hardwareAddress,
        I2CBus bus = I2CBus::SECONDARY,
        uint8_t factoryAddress = 0b0100000,
        RevLightsMode displayMode = RevLightsMode::LEFT_TO_RIGHT);
    ~PCF8574RevLights();

    virtual void onStart() override;
    virtual void onConnected() override;
    virtual void onTelemetryData(const TelemetryData *pTelemetryData) override;
    virtual void serveSingleFrame(uint32_t elapsedMs) override;
    virtual void onBitePoint(uint8_t bitePoint) override;
    virtual void onLowBattery() override;
    virtual void onSaveSettings() override;
    virtual void shutdown() override;
    virtual uint8_t getMaxFPS() override { return 40; }
    virtual uint16_t getStackSize() override { return 2048; }

private:
    PCF8574LedDriver *driver;
    uint8_t litCount, lastBitePoint;
    uint32_t timer;
    bool displayBitePoint;
    bool blink;
    bool blinkState;
    RevLightsMode displayMode;
};
