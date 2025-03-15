/**
 * @file OutputHardware.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-02
 * @brief Output hardware classes
 *
 * @copyright Licensed under the EUPL
 *
 */

#pragma once

//---------------------------------------------------------------
// Imports
//---------------------------------------------------------------

#include "SimWheelTypes.hpp"
#include "driver/rmt_tx.h" // For rmt_channel_handle_t & rmt_encoder_handle_t
#include "driver/i2c.h"    // For I2C operation

//---------------------------------------------------------------
// Led strip encoder
//---------------------------------------------------------------

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
        OutputGPIO dataPin,
        uint8_t pixelCount,
        bool useLevelShift,
        PixelDriver pixelType = PixelDriver::WS2812,
        PixelFormat pixelFormat = PixelFormat::AUTO);
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
     * @brief Shift all pixel colors to the next pixel index
     *
     */
    void shiftToNext();

    /**
     * @brief Shift all pixel colors to the previous pixel index
     *
     */
    void shiftToPrevious();

    /**
     * @brief Show pixel colors.
     *
     */
    void show();

private:
    uint8_t pixelCount;
    uint8_t *pixelData;
    PixelFormat pixelFormat;
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

//---------------------------------------------------------------
// PCF8574 LED Driver
//---------------------------------------------------------------

/**
 * @brief 8-LED driver based on the PCF8574 GPIO expander
 *
 * @note For single-color LEDs.
 *       P0 is the left-most LED.
 *       P7 is the right-most LED.
 */
class PCF8574LedDriver
{
public:
    /**
     * @brief Construct a LED driver
     *
     * @param bus I2C bus where the chip is attached to.
     * @param address7bits Full I2C address in 7-bit format.
     *
     */
    PCF8574LedDriver(
        I2CBus bus,
        uint8_t address7bits);

    /**
     * @brief Set the state of a single LED
     *
     * @note Not displayed immediately
     *
     * @param index LED index in the range [0,7].
     *              P0 has index 0.
     * @param state True to turn on, false to turn off.
     */
    void setLed(uint8_t index, bool state);

    /**
     * @brief Shift all lights to the left
     *
     * @note Not displayed immediately.
     *       P0 is the left-most LED.
     */
    void shiftLeft();

    /**
     * @brief Shift all lights to the right
     *
     * @note Not displayed immediately.
     *       P0 is the left-most LED.
     */
    void shiftRight();

    /**
     * @brief Invert the state of all LEDs
     *
     * @note Not displayed immediately
     */
    void swap() { _state = ~_state; }

    /**
     * @brief Show the required LEDs all at once
     *
     */
    void show() const;

    /**
     * @brief Get the state of each LED
     *
     * @return uint8_t A bitmap. 1 means turn on. 0 means turn off.
     */
    uint8_t getState() const { return _state; };

    /**
     * @brief Set the state of each LED all at once
     *
     * @note Not displayed immediately
     *
     * @param state A bitmap. 1 means turn on. 0 means turn off.
     */
    void setState(uint8_t state) { _state = state; }

    ~PCF8574LedDriver()
    {
        setState(0);
        show();
    }

private:
    uint8_t _state = 0;
    uint8_t _address8bit;
    I2CBus _bus;
};

//---------------------------------------------------------------
// Single LED
//---------------------------------------------------------------

/**
 * @brief A simple LED driver for a single LED
 *
 * @note You may attach two or more LEDs to the output pin,
 *       but all of them will behave as a single LED.
 *
 */
class SingleLED
{
public:
    /**
     * @brief Create a new single-LED driver
     *
     * @param pin
     */
    SingleLED(OutputGPIO pin);

    /**
     * @brief Set the state of the LED
     *
     * @param state true to turn on, false to turn off.
     */
    void set(bool state) { _state = state; }

    /**
     * @brief Get the state of the LED
     *
     * @return true If on
     * @return false If off
     */
    bool get() { return _state; }

    /**
     * @brief Invert the state of the LED
     *
     * @note Not displayed immediately
     */
    void swap() { _state = !_state; }

    /**
     * @brief Show the state of the LED
     *
     */
    void show();

    ~SingleLED() { _pin.grant(); }

private:
    OutputGPIO _pin;
    bool _state = false;
};