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
#include "driver/rmt_tx.h"
#include "i2cTools.h"

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
// LED strip
//-----------------------------------------------------------------------------

/**
 * @brief Pixel driver
 *
 */
typedef enum
{
    PIXEL_WS2811, // WS2811
    PIXEL_WS2812  // WS2812 and WS2812B
} pixel_drive_t;

//-----------------------------------------------------------------------------

/**
 * @brief Byte order of pixel data
 *
 */
typedef enum
{
    AUTO, // Auto-detect based on pixel driver
    RGB,  // Red-green-blue
    RBG,  // Red-blue-green
    GRB,  // Green-red-blue
    GBR,  // Green-blue-red
    BRG,  // Blue-red-green
    BGR   // Blue-green-red
} pixel_format_t;

//-----------------------------------------------------------------------------

class LEDStrip
{
public:
    /**
     * @brief Create an LED strip object.
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
    LEDStrip(
        gpio_num_t dataPin,
        uint8_t pixelCount,
        bool useLevelShift = false,
        pixel_drive_t pixelType = PIXEL_WS2812,
        pixel_format_t pixelFormat = pixel_format_t::AUTO);
    ~LEDStrip();

    /**
     * @brief Retrieve the pixel count in the strip.
     *
     * @return uint8_t Pixel count.
     */
    uint8_t getPixelCount() { return pixelCount; }

    /**
     * @brief Set pixel color in RGB format
     *
     * @param pixelIndex Index of the pixel in the strip.
     * @param redChannel Red component of the color.
     * @param greenChannel Green component of the color.
     * @param blueChannel Blue component of the color.
     * @note Effective after show() is called.
     */
    void pixelRGB(
        uint8_t pixelIndex,
        uint8_t redChannel,
        uint8_t greenChannel,
        uint8_t blueChannel);

    /**
     * @brief Set color (in RGB format) to a range of pixels
     *
     * @param fromPixelIndex Index of the first pixel.
     * @param toPixelIndex Index of the last pixel.
     * @param redChannel Red component of the color.
     * @param greenChannel Green component of the color.
     * @param blueChannel  lue component of the color.
     * @note Effective after show() is called.
     */
    void pixelRangeRGB(
        uint8_t fromPixelIndex,
        uint8_t toPixelIndex,
        uint8_t redChannel,
        uint8_t greenChannel,
        uint8_t blueChannel);

    void pixelRGB(
        uint8_t pixelIndex,
        uint32_t packedRGB)
    {
        pixelRGB(pixelIndex,
                 (uint8_t)(packedRGB >> 16),
                 (uint8_t)(packedRGB >> 8),
                 (uint8_t)(packedRGB));
    }

    void pixelRangeRGB(
        uint8_t fromPixelIndex,
        uint8_t toPixelIndex,
        uint32_t packedRGB)
    {
        pixelRangeRGB(fromPixelIndex,
                      toPixelIndex,
                      (uint8_t)(packedRGB >> 16),
                      (uint8_t)(packedRGB >> 8),
                      (uint8_t)(packedRGB));
    }

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
    void brightness(uint8_t value) { brightnessWeight = value + 1; }

private:
    uint8_t pixelCount;
    uint8_t *pixelData;
    pixel_format_t pixelFormat;
    rmt_channel_handle_t rmtHandle = nullptr;
    rmt_encoder_handle_t encHandle = nullptr;
    bool changed = false;
    uint8_t brightnessWeight = 16;

    void normalizeColor(uint8_t &r, uint8_t &g, uint8_t &b);
    void rawPixelRGB(
        uint8_t pixelIndex,
        uint8_t redChannel,
        uint8_t greenChannel,
        uint8_t blueChannel);
};

#endif