/**
 * @file SimWheelUI_gfx.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2026-03-09
 * @brief User interfaces for telemetry data and notifications
 *        requiring Adafruit's GFX library
 *
 * @copyright Licensed under the EUPL
 *
 */

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "SimWheelUI.hpp"
#include "Adafruit_GFX.h"
#include "HAL.hpp"
#include "InternalServices.hpp"

// DEVELOPER NOTE:
// You must uninstall the NeoPixel library in Arduino IDE
// if there is a compilation conflict with Adafruit_GFX

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// OLED UI 128x64
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

struct OledTelemetry128x64::Implementation
{
    /// @brief Frame buffer
    GFXcanvas1 frame{128, 64};
    /// @brief Timer to hide the bite point bar
    uint32_t bitePointTimer = 0;
    /// @brief True if currently flashing
    bool flashing = false;
    /// @brief Show the bite point bar instead of the telemetry data
    bool showBitePoint = false;
    /// @brief True if the screen must flash
    bool flash = false;
    /// @brief Device connected to host computer
    bool connected = false;
    /// @brief The frame buffer was updated and must be displayed
    bool updated = false;
}; // struct OledTelemetry128x64::Implementation

//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

OledTelemetry128x64::OledTelemetry128x64(
    const OLEDParameters &params,
    I2CBus bus,
    bool enableFlashing)
    : _impl{::std::make_unique<Implementation>()},
      _display(OLEDParameters::withResolution(128, 64, params), bus),
      _enableFlashing{enableFlashing}
{
    _display.clear();
    requiresPowertrainTelemetry = true;
    requiresECUTelemetry = true;
}

OledTelemetry128x64::OledTelemetry128x64(
    const OLEDParameters &params,
    uint8_t address7bits,
    I2CBus bus,
    bool enableFlashing)
    : _impl{::std::make_unique<Implementation>()},
      _display(OLEDParameters::withResolution(128, 64, params), bus),
      _enableFlashing{enableFlashing}
{
    _display.clear();
    requiresPowertrainTelemetry = true;
    requiresECUTelemetry = true;
}

//-----------------------------------------------------------------------------
// Protected methods
//-----------------------------------------------------------------------------

void OledTelemetry128x64::display_battery_level(uint8_t value)
{
    _impl->frame.fillScreen(0);
    _impl->frame.drawRoundRect(4, 16, 72, 32, 4, 0xFFFF);
    _impl->frame.drawRect(76, 24, 8, 16, 0xFFFF);
    uint8_t w = map_value(value, 0, 100, 0, 72);
    _impl->frame.fillRoundRect(4, 16, w, 32, 4, 0xFFFF);
    _impl->frame.setCursor(90, 25);
    _impl->frame.setTextSize(2);
    _impl->frame.setTextColor(0xFF, 0);
    if (value < 100)
        _impl->frame.printf("%02.2u%%", value);
    else
        _impl->frame.print("100");
    _display.show(_impl->frame.getBuffer());
}

void OledTelemetry128x64::stopFlashing()
{
    if (_impl->flash)
    {
        _impl->flash = false;
        _display.inverse_display(false);
    }
}

void OledTelemetry128x64::clearFrameBuffer()
{
    _impl->frame.fillScreen(0);
    _impl->updated = true;
}

//-----------------------------------------------------------------------------
// Notifications
//-----------------------------------------------------------------------------

void OledTelemetry128x64::onStart()
{
    stopFlashing();
    BatteryStatus status;
    BatteryService::call::getStatus(status);
    if (BatteryService::call::hasBattery() &&
        status.stateOfCharge.has_value())
        display_battery_level(status.stateOfCharge.value());
    else
    {
        _impl->frame.fillScreen(0);
        _impl->frame.drawRoundRect(0, 0, 128, 64, 4, 0xFFFF);
        _impl->frame.setTextSize(2);
        _impl->frame.setTextColor(0xFF, 0);
        _impl->frame.setCursor(10, 10);
        _impl->frame.print("\x02 ESP32 \x02");
        _impl->frame.setCursor(10, 30);
        _impl->frame.print("sim wheel");
        _display.show(_impl->frame.getBuffer());
    }
    DELAY_MS(2000);
    clearFrameBuffer();
}

void OledTelemetry128x64::onConnected()
{
    stopFlashing();
    _impl->connected = true;
    clearFrameBuffer();
}

void OledTelemetry128x64::onBLEdiscovering()
{
    stopFlashing();
    _impl->connected = false;
    _impl->frame.fillScreen(0);
    _impl->frame.drawRoundRect(0, 0, 128, 64, 4, 0xFFFF);
    _impl->frame.setTextSize(2);
    _impl->frame.setTextColor(0xFF, 0);
    _impl->frame.setCursor(22, 24);
    _impl->frame.print("(((\x07)))");
    _impl->updated = true;
}

void OledTelemetry128x64::onTelemetryData(const TelemetryData *pTelemetryData)
{
    if (_impl->showBitePoint)
        return;

    if (pTelemetryData)
    {
        if (_impl->flash && !pTelemetryData->powertrain.shiftLight2)
        {
            // Stop flashing
            _impl->flash = false;
            _display.inverse_display(false);
        }
        else
            _impl->flash = pTelemetryData->powertrain.shiftLight2;
        uint8_t aux;

        _impl->frame.fillScreen(0);
        // Draw RPM bar
        aux = map(pTelemetryData->powertrain.rpmPercent, 0, 100, 0, 128);
        _impl->frame.drawRect(0, 0, 128, 7, 0xFFFF);
        _impl->frame.fillRect(0, 0, aux, 7, 0xFFFF);

        // Draw gear
        _impl->frame.setTextSize(3);
        aux = (pTelemetryData->powertrain.shiftLight1) ? 0 : 0xFF;
        if (pTelemetryData->powertrain.shiftLight1)
            _impl->frame.fillRect(52, 25, 21, 27, 0xFFFF);
        _impl->frame.drawChar(
            55,
            28,
            pTelemetryData->powertrain.gear,
            aux,
            !aux, 3);

        // Draw brake bias
        _impl->frame.setTextSize(1);
        _impl->frame.setTextColor(0xFFFF);
        _impl->frame.setCursor(43, 56);
        _impl->frame.printf(
            "BB: %02.2u%%",
            pTelemetryData->ecu.brakeBias);

        // Draw ABS
        if (pTelemetryData->ecu.absEngaged)
        {
            _impl->frame.fillRect(107, 15, 19, 8, 0xFFFF);
            aux = 0;
        }
        else
            aux = 0xFF;
        // frame.setTextSize(1);
        _impl->frame.setCursor(108, 16);
        _impl->frame.setTextColor(aux, !aux);
        _impl->frame.print("ABS");
        _impl->frame.setTextSize(2);
        _impl->frame.setTextColor(0xFFFF, 0);
        _impl->frame.setCursor(103, 26);
        _impl->frame.printf(
            "%02.2u",
            pTelemetryData->ecu.absLevel);

        // Draw TC
        if (pTelemetryData->ecu.tcEngaged)
        {
            _impl->frame.fillRect(0, 15, 18, 9, 0xFFFF);
            _impl->frame.drawChar(1, 16, 'T', 0, 0xFFFF, 1);
            _impl->frame.drawChar(9, 16, 'C', 0, 0xFFFF, 1);
        }
        else
        {
            _impl->frame.drawChar(1, 16, 'T', 0xFFFF, 0, 1);
            _impl->frame.drawChar(9, 16, 'C', 0xFFFF, 0, 1);
        }
        // frame.setTextSize(2);
        _impl->frame.setCursor(1, 26);
        _impl->frame.printf(
            "%02.2u",
            pTelemetryData->ecu.tcLevel);

        // Draw fuel warning
        if (pTelemetryData->ecu.lowFuelAlert)
            _impl->frame.drawChar(61, 10, 'F', 0xFF, 0, 1);
    }
    else
    {
        _impl->frame.fillScreen(0);
        stopFlashing();
    }
    _impl->updated = true;
} // OledTelemetry128x64::onTelemetryData()

void OledTelemetry128x64::serveSingleFrame(uint32_t elapsedMs)
{
    if (_impl->showBitePoint &&
        (frameTimer(_impl->bitePointTimer, elapsedMs, 2000) > 0))
    {
        clearFrameBuffer();
        _impl->showBitePoint = false;
    }

    if (_impl->updated)
    {
        _impl->updated = false;
        _display.show(_impl->frame.getBuffer());
    }

    if (_enableFlashing && _impl->flash)
    {
        _impl->flashing = !(_impl->flashing);
        _display.inverse_display(_impl->flashing);
    }
}

void OledTelemetry128x64::onBitePoint(uint8_t bitePoint)
{
    if (_impl->connected)
    {
        stopFlashing();
        _impl->bitePointTimer = 0;
        _impl->showBitePoint = true;

        _impl->frame.fillScreen(0);
        _impl->frame.drawRoundRect(0, 0, 128, 64, 4, 0xFFFF);
        _impl->frame.fillRoundRect(0, 0, 128, 20, 4, 0xFFFF);
        _impl->frame.setTextSize(1);
        _impl->frame.setTextColor(0);
        _impl->frame.setCursor(34, 6);
        _impl->frame.print("Bite point");
        uint8_t percent = map_value(bitePoint, 0, 254, 0, 100);
        _impl->frame.setTextSize(2);
        _impl->frame.setTextColor(0xFFFF);
        _impl->frame.setCursor(48, 30);
        _impl->frame.printf("%02.2u%%", percent);
        _impl->updated = true;
    }
}

void OledTelemetry128x64::onLowBattery()
{
    stopFlashing();
    _impl->frame.fillScreen(0);
    _impl->frame.drawRoundRect(4, 16, 72, 32, 4, 0xFFFF);
    _impl->frame.drawRect(76, 24, 8, 16, 0xFFFF);
    _impl->frame.drawChar(98, 25, 0x01, 0xFFFF, 0, 2);
    _impl->frame.drawLine(24, 56, 56, 8, 0xFFFF);
    _display.show(_impl->frame.getBuffer());
    DELAY_MS(3000);
    clearFrameBuffer();
}

void OledTelemetry128x64::onSaveSettings()
{
    stopFlashing();
    _impl->frame.fillScreen(0);

    // Draw frame rect
    _impl->frame.drawRoundRect(0, 0, 128, 64, 4, 0xFFFF);

    // Draw arrow
    _impl->frame.drawRect(63, 8, 2, 40, 0xFFFF);
    _impl->frame.drawLine(48, 32, 64, 48, 0xFFFF);
    _impl->frame.drawLine(49, 32, 65, 48, 0xFFFF);
    _impl->frame.drawLine(64, 48, 80, 32, 0xFFFF);
    _impl->frame.drawLine(63, 48, 79, 32, 0xFFFF);

    // Draw tray
    _impl->frame.drawLine(48, 56, 80, 56, 0xFFFF);
    _impl->frame.drawLine(40, 48, 48, 56, 0xFFFF);
    _impl->frame.drawLine(80, 56, 88, 48, 0xFFFF);

    // Display
    _display.show(_impl->frame.getBuffer());
    DELAY_MS(1500);
    clearFrameBuffer();
}

void OledTelemetry128x64::shutdown()
{
    stopFlashing();
    _display.turn(false);
}