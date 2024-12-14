/**
 * @file LedStrip.h
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-10-24
 * @brief Interface to single-wire LED strips
 *
 * @copyright Licensed under the EUPL
 *
 */

#ifndef __LED_STRIP_H__
#define __LED_STRIP_H__

#include <stdint.h>
#include "driver/rmt_tx.h"
#include "hal/gpio_types.h" // declares gpio_num_t
#include "SimWheelTypes.h"

// Datasheets:
// SK6812: https://cdn-shop.adafruit.com/product-files/1138/SK6812%20LED%20datasheet%20.pdf
// UCS1903: https://www.led-stuebchen.de/download/UCS1903_English.pdf

/**
 * @brief Low-level interface to LED strips
 *
 */
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
        bool useLevelShift,
        pixel_driver_t pixelType = WS2812,
        pixel_format_t pixelFormat = pixel_format_t::AUTO);
    ~LEDStrip();

    /**
     * @brief Retrieve the pixel count in the strip.
     *
     * @return uint8_t Pixel count.
     */
    uint8_t getPixelCount() { return pixelCount; }

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

    // protected:
    /**
     * @brief Turn off all LEDs
     * @note Effective after show() is called.
     *
     */
    void clear();

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
     * @param blueChannel Blue component of the color.
     * @note Effective after show() is called.
     */
    void pixelRangeRGB(
        uint8_t fromPixelIndex,
        uint8_t toPixelIndex,
        uint8_t redChannel,
        uint8_t greenChannel,
        uint8_t blueChannel);

    /**
     * @brief Set pixel color in RGB format
     *
     * @param pixelIndex Index of the pixel.
     * @param packedRGB Pixel color in packet RGB format
     */
    void pixelRGB(
        uint8_t pixelIndex,
        uint32_t packedRGB)
    {
        pixelRGB(pixelIndex,
                 (uint8_t)(packedRGB >> 16),
                 (uint8_t)(packedRGB >> 8),
                 (uint8_t)(packedRGB));
    }

    /**
     * @brief Set color (in RGB format) to a range of pixels
     *
     * @param fromPixelIndex Index of the first pixel.
     * @param toPixelIndex Index of the last pixel.
     * @param packedRGB Pixel color in packet RGB format
     */
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
     * @brief Show pixel colors.
     *
     */
    void show();

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