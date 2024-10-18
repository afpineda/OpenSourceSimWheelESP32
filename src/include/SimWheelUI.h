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

typedef enum
{
    PIXEL_WS2811,
    PIXEL_WS2812
} pixel_type_t;

//-----------------------------------------------------------------------------

class LEDStrip
{
public:
    /**
     * @brief Create an LED strip object.
     *
     * @param dataPin GPIO number attached to `Din` (data input).
     * @param pixelCount Total count of pixels in the strip.
     * @param pixelType Type of pixel controller.
     */
    LEDStrip(
        gpio_num_t dataPin,
        uint8_t pixelCount,
        pixel_type_t pixelType = PIXEL_WS2812);
    ~LEDStrip();

    /**
     * @brief Retrieve the pixel count in the strip.
     *
     * @return uint8_t Pixel count.
     */
    uint8_t getPixelCount() { return pixelCount; }

    /**
     * @brief Set pixel color
     *
     * @param pixelIndex Index of the pixel in the strip.
     * @param redChannel Red component of the color.
     * @param greenChannel Green component of the color.
     * @param blueChannel Blue component of the color.
     * @note Effective after show() is called.
     */
    void pixelColor(
        uint8_t pixelIndex,
        uint8_t redChannel,
        uint8_t greenChannel,
        uint8_t blueChannel);

    void pixelRangeColor(
        uint8_t fromPixelIndex,
        uint8_t toPixelIndex,
        uint8_t redChannel = 0,
        uint8_t greenChannel = 0,
        uint8_t blueChannel = 0);

    /**
     * @brief Turn off all LEDs
     * @note Effective after show() is called.
     *
     */
    void clear();

    /**
     * @brief Show pixel colors.
     *
     */
    void show();

private:
    uint8_t pixelCount;
    byte *rawData;
    rmt_channel_handle_t rmtHandle = nullptr;
    rmt_encoder_handle_t encHandle = nullptr;
    bool changed = false;
};

#endif