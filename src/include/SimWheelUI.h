/**
 * @file SimWheelUI.h
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-10-09
 * @brief User interfaces.
 *
 * @copyright Licensed under the EUPL
 *
 */

#ifndef __SIM_WHEEL_UI__
#define __SIM_WHEEL_UI__

#include "SimWheelTypes.h"
#include "driver/i2c.h"
#include "i2cTools.h"
#include "LedStrip.h"

//-----------------------------------------------------------------------------
// Single Color-Single LED user interface
//-----------------------------------------------------------------------------

class SimpleShiftLight : public AbstractUserInterface
{
public:
    /**
     * @brief Create a simple "shift" light.
     *
     * @param ledPin GPIO pin where the LED is attached to.
     */
    SimpleShiftLight(gpio_num_t ledPin);

    virtual void onStart() override;
    virtual void onConnected() override;
    virtual void onTelemetryData(const telemetryData_t *pTelemetryData) override;
    virtual void serveSingleFrame(uint32_t elapsedMs) override;

private:
    gpio_num_t ledPin;
    uint32_t blinkTimer;
    bool ledState;
    uint8_t ledMode;
    void swapLED() { setLED(!ledState); }
    void setLED(bool newState);
    void setMode(uint8_t newMode);
};

//-----------------------------------------------------------------------------
// PCF8574-driven rev lights
//-----------------------------------------------------------------------------

class PCF8574RevLights : public AbstractUserInterface
{
public:
    /**
     * @brief Create "rev lights" using PCF8574 and
     *        single-color LEDs
     *
     * @param hardwareAddress An I2C hardware address (3 bits), as configured
     *                        using pins A0, A1 and A2.
     * @param useSecondaryBus `true` to use the secondary I2C bus (recommended).
     *                        `false` otherwise.
     * @param factoryAddress  Fixed factory-defined part of the full I2C address (7 bits).
     */
    PCF8574RevLights(
        uint8_t hardwareAddress,
        bool useSecondaryBus = true,
        uint8_t factoryAddress = 0b0100000);

    virtual void onStart() override;
    virtual void onConnected() override;
    virtual void onTelemetryData(const telemetryData_t *pTelemetryData) override;
    virtual void serveSingleFrame(uint32_t elapsedMs) override;

private:
    uint8_t address8bits;
    i2c_port_t busDriver;
    uint8_t ledState;
    bool forceUpdate;
    bool isBlinking;
    bool blinkState;
    uint32_t blinkTimer;
    void write(uint8_t state);
};

//-----------------------------------------------------------------------------
// LED strip telemetry
//-----------------------------------------------------------------------------

class LEDSegmentInterface
{

};

//-----------------------------------------------------------------------------

// class LEDStripTelemetry : public AbstractUserInterface
// {
// public:
//     /**
//      * @brief Create an LED strip for telemetry display.
//      *
//      * @param dataPin GPIO number attached to `Din` (data input).
//      * @param pixelCount Total count of pixels in the strip.
//      * @param useLevelShift Set to `false` when using 3.3V logic.
//      *                      Set to `true` when using the level
//      *                      shifter in open-drain mode.
//      * @param pixelType Pixel driver.
//      * @param pixelFormat Format of color data (byte order).
//      *                    Set to `AUTO` for auto-detection.
//      */
//     LEDStripTelemetry(
//         gpio_num_t dataPin,
//         uint8_t pixelCount,
//         bool useLevelShift,
//         pixel_driver_t pixelType = WS2812,
//         pixel_format_t pixelFormat = pixel_format_t::AUTO);
//     ~LEDStripTelemetry();

// public:
//     virtual void onStart() override;
//     virtual void onTelemetryData(const telemetryData_t *pTelemetryData) override;
//     virtual void serveSingleFrame(uint32_t elapsedMs) override;
//     virtual void onBitePoint() override;
//     virtual void onConnected() override;
//     virtual void onBLEdiscovering() override;

// private:
//     LEDStrip *ledStrip;
//     std::vector<LEDSegment *> ledSegments;
// }

//-----------------------------------------------------------------------------
// LED segment
//-----------------------------------------------------------------------------

// class LEDSegment
// {
//     friend class LEDStrip;

// public:
//     LEDSegment(
//         LEDStrip *strip,
//         uint8_t fromPixelIndex,
//         uint8_t toPixelIndex);

// protected:
//     /// Set to true to receive and use powertrain telemetry data
//     bool requiresPowertrainTelemetry = false;
//     /// Set to true to receive and use ECU telemetry data
//     bool requiresECUTelemetry = false;
//     /// Set to true to receive and use race control telemetry data
//     bool requiresRaceControlTelemetry = false;
//     /// Set to true to receive and use telemetry data for gauges
//     bool requiresGaugeTelemetry = false;

// private:
//     uint8_t fromPixelIndex;
//     uint8_t toPixelIndex;
//     LEDStrip *strip;
// }

#endif