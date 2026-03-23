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

#define OLED_BATT_UPDATE_MS 15000

struct OledTelemetry128x64::Implementation
{
    /// @brief Frame buffer
    GFXcanvas1 frame{128, 64};
    /// @brief Timer to hide the bite point bar
    uint32_t bitePointTimer = 0;
    /// @brief Timer to update the battery dashboard
    uint32_t batteryUpdateTimer = 0;
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
    /// @brief Input number to cycle dashboards
    uint8_t nextDash = 0xFF;
    /// @brief Current user-selected dashboard
    OledDashboard currentDash = OledDashboard::_DEFAULT;
}; // struct OledTelemetry128x64::Implementation

//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

OledTelemetry128x64::OledTelemetry128x64(
    const OLEDParameters &params,
    I2CBus bus,
    bool enableFlashing,
    OledDashboard initialDashboard,
    InputNumber nextDash)
    : _impl{::std::make_unique<Implementation>()},
      _display(OLEDParameters::withResolution(128, 64, params), bus),
      _enableFlashing{enableFlashing}
{
    init(nextDash, initialDashboard);
}

OledTelemetry128x64::OledTelemetry128x64(
    const OLEDParameters &params,
    uint8_t address7bits,
    I2CBus bus,
    bool enableFlashing,
    OledDashboard initialDashboard,
    InputNumber nextDash)
    : _impl{::std::make_unique<Implementation>()},
      _display(
          OLEDParameters::withResolution(128, 64, params),
          address7bits,
          bus),
      _enableFlashing{enableFlashing}
{
    init(nextDash, initialDashboard);
}

//-----------------------------------------------------------------------------
// Protected methods
//-----------------------------------------------------------------------------

void OledTelemetry128x64::init(
    InputNumber nextDash,
    OledDashboard initialDashboard)
{
    _impl->nextDash = nextDash;
    _impl->currentDash = initialDashboard;
    _display.clear();
    requiresPowertrainTelemetry = true;
    requiresECUTelemetry = true;
    requiresGaugeTelemetry = (nextDash != UNSPECIFIED::VALUE);
}

void OledTelemetry128x64::draw_battery_level()
{
    BatteryStatus status;
    BatteryService::call().getStatus(status);

    // Draw a battery shape
    _impl->frame.fillScreen(0);
    _impl->frame.drawRoundRect(4, 16, 72, 32, 4, 0xFFFF);
    _impl->frame.drawRect(76, 24, 8, 16, 0xFFFF);
    _impl->frame.setCursor(90, 25);
    _impl->frame.setTextSize(2);
    _impl->frame.setTextColor(0xFF, 0);

    if (status.stateOfCharge.has_value())
    {
        // Draw filled rectangle
        uint8_t value = status.stateOfCharge.value();
        uint8_t w = map_value(value, 0, 100, 0, 72);
        _impl->frame.fillRoundRect(4, 16, w, 32, 4, 0xFFFF);
        // Print battery charge
        if (value < 100)
            _impl->frame.printf("%2.2u%%", value);
        else
            _impl->frame.print("100");
    }
    else
    {
        // Print unknown percentage
        _impl->frame.printf("???");
    }
}

void OledTelemetry128x64::draw_main_dashboard(
    const TelemetryData *pTelemetryData)
{
    if (pTelemetryData)
    {
        // Determine flashing
        if (_impl->flash && !pTelemetryData->powertrain.shiftLight2)
        {
            // Stop flashing
            _impl->flash = false;
            _display.inverse_display(false);
        }
        else
            _impl->flash = pTelemetryData->powertrain.shiftLight2;
        uint8_t aux;

        // Clear the frame buffer
        _impl->frame.fillScreen(0);

        // Create 3 vertical sections
        _impl->frame.drawFastVLine(38, 8, 56, 0xFFFF);
        _impl->frame.drawFastVLine(89, 8, 56, 0xFFFF);

        // Draw RPM bar
        aux = map(pTelemetryData->powertrain.rpmPercent, 0, 100, 0, 128);
        _impl->frame.drawRect(0, 0, 128, 7, 0xFFFF);
        _impl->frame.fillRect(0, 0, aux, 7, 0xFFFF);

        // Draw gear
        _impl->frame.setTextSize(3);
        aux = (pTelemetryData->powertrain.shiftLight1) ? 0 : 0xFF;
        if (pTelemetryData->powertrain.shiftLight1)
            _impl->frame.fillRect(53, 17, 22, 27, 0xFFFF);
        _impl->frame.drawChar(
            56,
            20,
            pTelemetryData->powertrain.gear,
            aux,
            !aux, 3);

        // Draw brake bias
        _impl->frame.setTextSize(1);
        _impl->frame.setTextColor(0xFFFF);
        _impl->frame.setCursor(43, 56);
        _impl->frame.printf(
            "BB: %02u%%",
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
            "%2.2u",
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
            "%2.2u",
            pTelemetryData->ecu.tcLevel);

        // Draw fuel warning
        if (pTelemetryData->ecu.lowFuelAlert)
        {
            // Text size: 1
            _impl->frame.drawChar(11, 49, 'F', 0xFF, 0, 1);
        }

        // Draw pit limiter witness
        if (pTelemetryData->ecu.pitLimiter)
        {
            // Text size: 1
            _impl->frame.drawChar(116, 49, 'P', 0xFF, 0, 1);
        }
    }
    else if (_impl->connected) // && !pTelemetryData
    {
        _impl->frame.fillScreen(0);
        stopFlashing();
    }
} // OledTelemetry128x64::draw_main_dashboard()

void OledTelemetry128x64::draw_alt_dashboard(
    const TelemetryData *pTelemetryData)
{
    if (pTelemetryData)
    {
        uint8_t aux;

        // Determine flashing
        if (_impl->flash && !pTelemetryData->powertrain.shiftLight2)
        {
            // Stop flashing
            _impl->flash = false;
            _display.inverse_display(false);
        }
        else
            _impl->flash = pTelemetryData->powertrain.shiftLight2;

        // Clear the frame buffer
        _impl->frame.fillScreen(0);

        // Draw literals size 1
        _impl->frame.setTextColor(0xFFFF);
        _impl->frame.setTextSize(1);
        _impl->frame.setCursor(0, 0);
        _impl->frame.printf("Speed");
        _impl->frame.setCursor(110, 0);
        _impl->frame.printf("RPM");

        // Draw pit limiter witness (size 1)
        if (pTelemetryData->ecu.pitLimiter)
            _impl->frame.drawChar(61, 0, 'P', 0xFF, 0, 1);

        // Draw ABS (size1)
        if (pTelemetryData->ecu.absEngaged)
        {
            _impl->frame.fillRect(0, 32, 21, 9, 0xFFFF);
            aux = 0;
        }
        else
            aux = 0xFF;
        _impl->frame.setCursor(0, 33);
        _impl->frame.setTextColor(aux, !aux);
        _impl->frame.printf(
            "ABS:%02hhu",
            pTelemetryData->ecu.absLevel);

        // Draw TC (size1)
        if (pTelemetryData->ecu.tcEngaged)
        {
            _impl->frame.fillRect(0, 42, 14, 9, 0xFFFF);
            aux = 0;
        }
        else
            aux = 0xFF;
        _impl->frame.setCursor(0, 43);
        _impl->frame.setTextColor(aux, !aux);
        _impl->frame.printf(
            "TC:%02hhu",
            pTelemetryData->ecu.tcLevel);

        // Draw Fuel (size1)
        if (pTelemetryData->ecu.lowFuelAlert)
        {
            _impl->frame.fillRect(48, 32, 8, 9, 0xFFFF);
            aux = 0;
        }
        else
            aux = 0xFF;
        _impl->frame.setCursor(49, 33);
        _impl->frame.setTextColor(aux, !aux);
        _impl->frame.printf(
            "F:%-3hhu",
            pTelemetryData->gauges.relativeRemainingFuel);

        // Draw brake bias (size1)
        _impl->frame.setCursor(0, 53);
        _impl->frame.setTextColor(0xFFFF, 0);
        _impl->frame.printf(
            "BB:%-3hhu",
            pTelemetryData->ecu.brakeBias);

        // Draw Speed size 2
        _impl->frame.setTextSize(2);
        // _impl->frame.setTextColor(0xFFFF);
        _impl->frame.setCursor(0, 10);
        _impl->frame.printf("%3hu", pTelemetryData->powertrain.speed);

        // Draw RPM size 2
        // _impl->frame.setTextSize(2);
        // _impl->frame.setTextColor(0xFFFF);
        _impl->frame.setCursor(68, 10);
        _impl->frame.printf("%5hu", pTelemetryData->powertrain.rpm);

        // Draw gear
        if (pTelemetryData->powertrain.shiftLight1)
        {
            _impl->frame.fillRect(106, 38, 27, 28, 0xFFFF);
            aux = 0;
        }
        else
            aux = 0xFF;
        _impl->frame.drawChar(
            109,
            40,
            pTelemetryData->powertrain.gear,
            aux,
            !aux, 3);

        // Draw frame lines
        _impl->frame.writeFastHLine(0, 29, 128, 0xFF);
        _impl->frame.writeFastVLine(45, 0, 64, 0xFF);
    }
    else if (_impl->connected) // && !pTelemetryData
    {
        _impl->frame.fillScreen(0);
        stopFlashing();
    }
} // void OledTelemetry128x64::draw_alt_dashboard()

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
    if (BatteryService::call().hasBattery())
        draw_battery_level();
    else
    {
        // Show welcome message
        _impl->frame.fillScreen(0);
        _impl->frame.drawRoundRect(0, 0, 128, 64, 4, 0xFFFF);
        _impl->frame.setTextSize(2);
        _impl->frame.setTextColor(0xFF, 0);
        _impl->frame.setCursor(10, 10);
        _impl->frame.print("\x02 ESP32 \x02");
        _impl->frame.setCursor(10, 30);
        _impl->frame.print("sim wheel");
    }
    _display.show(_impl->frame.getBuffer());
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

void OledTelemetry128x64::onUserInput(uint8_t inputNumber)
{
    if (inputNumber == _impl->nextDash)
    {
        stopFlashing();
        switch (_impl->currentDash)
        {
        case OledDashboard::STANDARD:
            _impl->currentDash = OledDashboard::ALTERNATE;
            break;
        case OledDashboard::ALTERNATE:
            _impl->batteryUpdateTimer = OLED_BATT_UPDATE_MS;
            if (BatteryService::call().hasBattery())
                _impl->currentDash = OledDashboard::BATTERY;
            else
                _impl->currentDash = OledDashboard::STANDARD;
            break;
        case OledDashboard::BATTERY:
            _impl->currentDash = OledDashboard::STANDARD;
            break;
        default:
            // Should not enter here
            _impl->currentDash = OledDashboard::_DEFAULT;
            break;
        }
        clearFrameBuffer();
    }
}

void OledTelemetry128x64::onTelemetryData(const TelemetryData *pTelemetryData)
{
    if (_impl->showBitePoint)
        return;

    switch (_impl->currentDash)
    {
    case OledDashboard::STANDARD:
        draw_main_dashboard(pTelemetryData);
        break;
    case OledDashboard::ALTERNATE:
        draw_alt_dashboard(pTelemetryData);
        break;
    default:
        return;
    }
    _impl->updated = true;
}

void OledTelemetry128x64::serveSingleFrame(uint32_t elapsedMs)
{
    if (_impl->showBitePoint &&
        (frameTimer(_impl->bitePointTimer, elapsedMs, 2000) > 0))
    {
        clearFrameBuffer();
        _impl->showBitePoint = false;
    }

    if ((_impl->currentDash == OledDashboard::BATTERY) &&
        (frameTimer(
             _impl->batteryUpdateTimer,
             elapsedMs,
             OLED_BATT_UPDATE_MS) > 0))
    {
        _impl->batteryUpdateTimer = 0;
        _impl->updated = true;
        draw_battery_level();
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
        _impl->frame.printf("%02u%%", percent);
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