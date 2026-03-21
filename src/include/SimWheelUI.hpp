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
#include <memory> // For std::unique_ptr

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
}; // class PCF8574RevLights

//-----------------------------------------------------------------------------
// OLED UI 128x64
//-----------------------------------------------------------------------------

/**
 * @brief Available dashboards for monochrome OLED displays
 *
 */
enum class OledDashboard : uint8_t
{
    /// @brief RPM bar, gear, TC, ABS, Low fuel, Pit limiter
    STANDARD = 0,
    /// @brief Dashboard not available yet
    ALTERNATE,
    /// @brief Current battery level
    ///        (available only in battery-operated firmwares)
    BATTERY,
    /// @brief Default dashboard
    _DEFAULT = STANDARD
};

/**
 * @brief Telemetry display via 128x64 monochrome OLED
 *
 */
class OledTelemetry128x64 : public AbstractUserInterface
{
private:
    /// @brief Private implementation type
    struct Implementation; // https://cpppatterns.com/patterns/pimpl.html
    /// @brief Private implementation instance
    ::std::unique_ptr<Implementation> _impl;
    /// @brief Display hardware
    OLED _display;
    /// @brief Whether to allow screen flashing or not
    bool _enableFlashing{true};

    /// @brief Internal initialization
    /// @param nextDash Input number to cycle to the next dashboard
    /// @param initialDashboard Dashboard to show at first
    /// @note Called from the constructor exclusively
    void init(InputNumber nextDash, OledDashboard initialDashboard);

    /// @brief Display the battery level into the internal frame buffer
    /// @param value Battery level
    inline void draw_battery_level();

    /// @brief Draw the main dashboard into the internal frame buffer
    /// @param pTelemetryData Pointer to telemetry data (maybe null)
    inline void draw_main_dashboard(const TelemetryData *pTelemetryData);

    /// @brief Draw the alternate dashboard into the internal frame buffer
    /// @param pTelemetryData Pointer to telemetry data (maybe null)
    void draw_alt_dashboard(const TelemetryData *pTelemetryData);

    /// @brief Stop flashing
    void stopFlashing();

    /// @brief Clear the internal frame buffer
    void clearFrameBuffer();

public:
    /**
     * @brief Create an Oled Telemetry display (128x64)
     *
     * @param params OLED hardware parameters
     * @param bus I2C bus
     * @param enableFlashing Whether to allow screen flashing or not
     * @param initialDashboard Dashboard to show at first
     * @param nextDash Input number to cycle to the next dashboard.
     *                 Set to UNSPECIFIED::VALUE to disable.
     *                 The input number must be routed by
     *                 inputHub::route_to_ui::add()
     */
    OledTelemetry128x64(
        const OLEDParameters &params,
        I2CBus bus = I2CBus::SECONDARY,
        bool enableFlashing = true,
        OledDashboard initialDashboard = OledDashboard::_DEFAULT,
        InputNumber nextDash = UNSPECIFIED::VALUE);

    /**
     * @brief Create an Oled Telemetry display (128x64)
     *
     * @param params OLED hardware parameters
     * @param address7bits Full I2C address in 7 bit format
     * @param bus I2C bus
     * @param enableFlashing Whether to allow screen flashing or not
     * @param initialDashboard Dashboard to show at first
     * @param nextDash Input number to cycle to the next dashboard.
     *                 Set to UNSPECIFIED::VALUE to disable.
     *                 The input number must be routed by
     *                 inputHub::route_to_ui::add()
     */
    OledTelemetry128x64(
        const OLEDParameters &params,
        uint8_t address7bits,
        I2CBus bus = I2CBus::SECONDARY,
        bool enableFlashing = true,
        OledDashboard initialDashboard = OledDashboard::_DEFAULT,
        InputNumber nextDash = UNSPECIFIED::VALUE);

    virtual void onStart() override;
    virtual void onConnected() override;
    virtual void onBLEdiscovering() override;
    virtual void onTelemetryData(const TelemetryData *pTelemetryData) override;
    virtual void serveSingleFrame(uint32_t elapsedMs) override;
    virtual void onBitePoint(uint8_t bitePoint) override;
    virtual void onLowBattery() override;
    virtual void onSaveSettings() override;
    virtual void onUserInput(uint8_t inputNumber) override;
    virtual void shutdown() override;
    virtual uint8_t getMaxFPS() override { return 30; }
    virtual uint16_t getStackSize() override { return 2560; }
}; // class OledTelemetry128x64
