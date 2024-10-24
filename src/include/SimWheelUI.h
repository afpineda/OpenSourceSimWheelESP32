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
// Segment to LED strip interface
//-----------------------------------------------------------------------------

class LEDSegmentToStripInterface
{
public:
    /**
     * @brief Set the color of a range of consecutive pixels.
     *
     * @param fromPixelIndex Index of the first pixel.
     * @param toPixelIndex Index of the last pixel.
     * @param packedRGB Pixel color in RGB format.
     */
    virtual void setPixelColor(
        uint8_t fromPixelIndex,
        uint8_t toPixelIndex,
        uint32_t packedRGB) = 0;

    /**
     * @brief Set the color of a single pixel.
     *
     * @param pixelIndex Pixel index.
     * @param packedRGB Pixel color in RGB format.
     */
    virtual void setPixelColor(
        uint8_t pixelIndex,
        uint32_t packedRGB) = 0;
};

//-----------------------------------------------------------------------------
// Abstract LED strip segment
//-----------------------------------------------------------------------------

class LEDStripTelemetry; // forward declaration

class LEDSegment
{
public:
    LEDSegment(
        LEDStripTelemetry *ledStripTelemetry,
        bool requiresPowertrainTelemetry,
        bool requiresECUTelemetry,
        bool requiresRaceControlTelemetry,
        bool requiresGaugeTelemetry);
    ~LEDSegment();

public:
    /**
     * @brief Notify new telemetry data
     *
     * @param pTelemetryData Pointer to telemetry data. Can not be null.
     *                       Safe to store for later use.
     * @param ledInterface Interface to set pixel colors.
     * @note Must not enter an infinite loop. Must return as soon as possible.
     *
     */
    virtual void onTelemetryData(
        const telemetryData_t *pTelemetryData,
        LEDSegmentToStripInterface &ledInterface) = 0;

    /**
     * @brief Draw a single frame.
     *
     * @param elapsedMs Elapsed milliseconds since last call.
     *
     * @note Must not enter an infinite loop. Must return as soon as possible.
     *
     */
    virtual void serveSingleFrame(
        uint32_t elapsedMs,
        LEDSegmentToStripInterface &ledInterface) = 0;

    /**
     * @brief Notify a change in current bite point.
     *
     * @note Read userSetting::bitePoint to know the last value
     */
    virtual void onBitePoint(LEDSegmentToStripInterface &ledInterface) {};

protected:
    /**
     * @brief Simple timer.
     *
     * @note To be used in serveSingleFrame().
     *       Initialize the timer variable to zero.
     *
     * @param timerVariable Timer variable.
     * @param elapsedTimeMs Time elapsed since last call in miliseconds.
     * @param timeLimitMs Expiration time in miliseconds.
     * @return uint32_t The count of times the timer has expired in the elapsed time.
     */
    uint32_t frameTimer(
        uint32_t &timerVariable,
        uint32_t elapsedTimeMs,
        uint32_t timeLimitMs)
    {
        timerVariable += elapsedTimeMs;
        uint32_t result = timerVariable / timeLimitMs;
        timerVariable %= timeLimitMs;
        return result;
    };
};

//-----------------------------------------------------------------------------
// LED strip telemetry
//-----------------------------------------------------------------------------

class LEDStripTelemetry : public AbstractUserInterface,
                          public LEDSegmentToStripInterface
{
    friend class LEDSegment;

public:
    /**
     * @brief Create an LED strip for telemetry display.
     *
     * @param dataPin GPIO number attached to `Din` (data input).
     * @param pixelCount Total count of pixels in the strip.
     * @param useLevelShift Set to `false` when using 3.3V logic.
     *                      Set to `true` when using the level
     *                      shifter in open-drain mode.
     * @param pixelType Pixel driver.
     * @param pixelFormat Format of color data (byte order).
     *                    Set to `AUTO` for auto-detection.
     */
    LEDStripTelemetry(
        gpio_num_t dataPin,
        uint8_t pixelCount,
        bool useLevelShift,
        pixel_driver_t pixelType = WS2812,
        pixel_format_t pixelFormat = pixel_format_t::AUTO);
    ~LEDStripTelemetry();

    /**
     * @brief Set global LED brightness
     *
     * @param value Brightness.
     *              255 is the highest and
     *              0 will turn all LEDs off.
     *
     * @note LEDs are very bright.
     *       Keep this value low for a comfortable experience.
     *       Defaults to 15 (decimal).
     */
    void brightness(uint8_t value) { ledStrip->brightness(value); }

    /**
     * @brief Retrieve the count of pixels in the strip.
     *
     * @return uint8_t Pixel count.
     */
    uint8_t getPixelCount() { return ledStrip->getPixelCount(); }

public: // LEDSegmentToStripInterface implementation
    virtual void setPixelColor(
        uint8_t fromPixelIndex,
        uint8_t toPixelIndex,
        uint32_t packedRGB) override;

    virtual void setPixelColor(
        uint8_t pixelIndex,
        uint32_t packedRGB) override;

public: // AbstractUserInterface implementation
    virtual void onStart() override;
    virtual void onTelemetryData(const telemetryData_t *pTelemetryData) override;
    virtual void serveSingleFrame(uint32_t elapsedMs) override;
    virtual void onBitePoint() override;
    virtual void onConnected() override;
    virtual void onBLEdiscovering() override;
    virtual void onLowBattery() override;

private:
    LEDStrip *ledStrip;
    bool started = false;
    std::vector<LEDSegment *> ledSegments;

    void addSegment(
        LEDSegment *segment,
        bool requiresPowertrainTelemetry,
        bool requiresECUTelemetry,
        bool requiresRaceControlTelemetry,
        bool requiresGaugeTelemetry);
};

//-----------------------------------------------------------------------------
// LED segment: shift light
//-----------------------------------------------------------------------------

class ShiftLightLEDSegment : public LEDSegment
{
public:
    ShiftLightLEDSegment(
        LEDStripTelemetry *ledStripTelemetry,
        uint8_t pixelIndex,
        uint32_t maxTorqueColor = 0xFFFF00,
        uint32_t maxRPMColor = 0xFF0000);

public: // LEDSegment implementation
    virtual void onTelemetryData(
        const telemetryData_t *pTelemetryData,
        LEDSegmentToStripInterface &ledInterface) override;
    virtual void serveSingleFrame(
        uint32_t elapsedMs,
        LEDSegmentToStripInterface &ledInterface) override;

private:
    uint8_t pixelIndex;
    uint32_t blinkTimer;
    uint32_t maxTorqueColor;
    uint32_t maxRPMColor;
    bool blinkState;
    bool blink;
};

#endif