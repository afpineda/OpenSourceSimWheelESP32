/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Main header for common sources
 *
 * @section DESCRIPTION
 *
 * This file declares all the namespaces that builds the system architecture.
 * See the [Firmware Architecture document](../../doc/firmware/FirmwareArchitecture_en.md).
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#ifndef __SIMWHEEL_H__
#define __SIMWHEEL_H__

#include "SimWheelTypes.h"
#include "esp32-hal.h" // declares gpio_num_t
#include <string>

/**
 * @brief Device capabilities
 *
 */
namespace capabilities
{
    // For read only
    extern volatile uint32_t flags;

    /**
     * @brief Set a device capability. Should be called before `hidImplementation::begin()`.
     *        All capabilities are static and should not change once set.
     *        Will be called from other namespaces. No need for manual set up.
     *
     * @param newFlag A device capability.
     * @param setOrClear When true, the flag is set. Otherwise, the flag is cleared.
     */
    void setFlag(deviceCapability_t newFlag, bool setOrClear = true);

    /**
     * @brief Ask for a device capability
     *
     * @param flag Requested capability
     * @return true The device has the given capability
     * @return false The device does not have the given capability
     */
    bool inline hasFlag(deviceCapability_t flag);
}

/**
 * @brief Current state of the clutch paddles (if any) and,
 *        for convenience, mode of "ALT" buttons
 *
 */
namespace clutchState
{
    // For read only. do not touch
    extern volatile clutchValue_t leftAxis;
    extern volatile clutchValue_t rightAxis;
    extern volatile clutchValue_t combinedAxis;
    extern volatile clutchValue_t bitePoint;
    extern volatile clutchFunction_t currentFunction;
    extern volatile bool altModeForAltButtons;

    /**
     * @brief Must be called before anything else in this namespace. Will
     *        load user settings from flash memory.
     *
     * @note Do not call while testing. This way, autosaving will get disabled.
     **/
    void begin();

    /**
     * @brief Set operation mode for "ALT" buttons (not clutch related)
     *
     * @param newMode When True, "ALT" buttons should activate "ALT" mode.
     *                Otherwise, they behave as regular buttons.
     */
    void setALTModeForALTButtons(bool newMode);

    /**
     * @brief Assign a function to the clutch paddles
     *
     * @param newFunction Function of the clutch paddles
     */
    void setFunction(clutchFunction_t newFunction);

    /**
     * @brief Set the current bite point
     *
     * @param newBitePoint
     */
    void setBitePoint(clutchValue_t newBitePoint);

    /**
     * @brief Set the position of the left padlle axis
     *
     * @param newValue Axis position
     */
    void setLeftAxis(clutchValue_t newValue);

    /**
     * @brief Set the position of the right paddle axis
     *
     * @param newValue Axis position
     */
    void setRightAxis(clutchValue_t newValue);

    /**
     * @brief Check if any clutch paddle is pressed while
     *        "ALT" function is selected
     *
     * @return true if ALT mode is requested
     * @return false otherwise
     */
    bool isALTRequested();

    /**
     * @brief Check if bite point calibration is requested by the user
     *
     * @return true if bite point can be manually calibrated
     * @return false otherwise
     */
    bool isCalibrationInProgress();
}

/**
 * @brief Calibrate and measure battery charge
 *
 * @note This namespace provides two algorithms to map a battery reading into a
 *       battery level percentage:
 *       - Calibrated: a calibration procedure is required
 *       - Auto-calibrated: fallback arlogorithm based on LiPo characterization data.
 *         Requires a fully charged battery for auto-calibration.
 */
namespace batteryCalibration
{
    /**
     * @brief Check if the calibration procedure is in progress. Exposed for testing. DO NOT TOUCH.
     *
     */
    extern volatile bool calibrationInProgress;

    /**
     * @brief Load or initialize calibration data. Must be called at system startup.
     *
     */
    void begin();

    /**
     * @brief Clear calibration data (but may persist in flash memory).
     *
     */
    void clear();

    /**
     * @brief Add an ADC reading to calibration data. The battery should get fully charged
     *        before first call. Will clear previous data at first call.
     *        Calibration data will **not** be available until `save()` is called.
     *
     * @param reading An ADC reading of current battery(+) voltage.
     */
    void addSample(int reading);

    /**
     * @brief Get calibration data. For internal use.
     *
     * @param index Internal index of data
     * @return uint16_t -1 if there is no calibration data for such an index. A positive number, otherwise
     */
    int getCalibration(uint8_t index);

    /**
     * @brief Set calibration data all at once. For internal use.
     *
     * @param data Array of calibration data. Array size is 32.
     */
    void restoreCalibrationData(const uint16_t data[]);

    /**
     * @brief Save calibration data to flash memory.
     *        The battery should get fully depleted before calling.
     *        Calibration data will be available from now on.
     *
     */
    void save();

    /**
     * @brief Get a percentage of battery charge based on calibration data.
     *
     * @param reading An ADC reading of current battery(+) voltage.
     * @return int -1 if the battery has not been calibrated. Otherwise,
     *             a percentage in the range 0%-100%.
     */
    int getBatteryLevel(int reading);

    /**
     * @brief Maximun battery reading ever. Exposed for testing. DO NOT TOUCH.
     *
     */
    extern volatile int maxBatteryReadingEver;

    /**
     * @brief Get a rough estimation of the battery charge based on
     *        LiPo battery characterization data. Not accurate, but allways available.
     *
     * @param reading An ADC reading of current battery(+) voltage.
     * @return int A percentage in the range 0%-100%
     *
     * @note Based on https://blog.ampow.com/lipo-voltage-chart/
     */
    int getGenericLiPoBatteryLevel(int reading);

    /**
     * @brief Restart autocalibration algorithm.
     *
     */
    void restartAutoCalibration();

    /**
     * @brief Get a percentage of battery charge using auto-calibration.
     *        Based on `getGenericLiPoBatteryLevel()`. Will provide incorrect
     *        battery levels (higher) until the battery is fully charged.
     *        Anyway, this algorithm is not accurate.
     *
     * @param reading An ADC reading of current battery(+) voltage
     *
     * @return If auto-calibration is available, a percentage in the range 0%-100%.
     *         Otherwise, the constant `UNKNOWN_BATTERY_LEVEL`.
     */
    int getBatteryLevelAutoCalibrated(int reading);

}

namespace power
{
    /**
     * @brief Initialize power management
     *
     * @param wakeUpPins Array of RTC-capable GPIO pins to wake up the system after deep sleep.
     * @param wakeUpPinCount Number of pins in `wakeUpPins`. If no pins are provided, a system
     *                       reset is required for wakeup.
     * @param AnyHighOrAllLow If *true*, wakeup happens when *any* given pin are set to high voltage.
     *                        If *false*, wakeup happens when *all* given pins are set to low voltage.
     */
    void begin(
        const gpio_num_t wakeUpPins[],
        const uint8_t wakeUpPinCount,
        bool AnyHighOrAllLow = true);

    /**
     * @brief Initialize power management for single button wake up
     *
     * @param wakeUpPin RTC-capable GPIO pin to wake up the system after deep sleep.
     * @param wakeUpHighOrLow If *true*, wakeup happens when `wakeUpPin` is set to high voltage.
     *                        If *false*, wakeup happens when `wakeUpPin` is set to low voltage.
     */
    void begin(
        const gpio_num_t wakeUpPin, bool wakeUpHighOrLow = true);

    /**
     * @brief Configure an external latch circuit for power on and off
     *
     * @param latchPin Output-capable GPIO that drives power on/off.
     * @param mode Expected behaviour of 'latchPin' to keep power on or go to power off.
     * @param waitTicks A delay to wait for power off to happen, in milliseconds.
     */
    void setPowerLatch(gpio_num_t latchPin, powerLatchMode_t mode, uint32_t waitMs);

    /**
     * @brief Enable monitorization of battery charge. Do not call if there is no battery.
     *
     * @param battENPin Output pin to enable/disable the battery monitor circuit.
     *                  Set to `GPIO_NUM_NC` (-1) if `battREADPin` is attached to
     *                  a simple voltage divider. This is the case for most battery-enabled devkits.
     * @param battREADPin ADC pin used to read battery voltage
     * @param testing Set to TRUE for unit testing: will get a battery sample every 5 seconds,
     *                will not power off the device.
     */
    void startBatteryMonitor(
        gpio_num_t battENPin,
        gpio_num_t battREADPin,
        bool testing = false);

    /**
     * @brief Get ADC reading of the battery pin for testing porpouses. Must **not** be called while
     *        the battery monitor is running
     *
     * @param battENPin Output-capable GPIO pin to enable battery readings. Must have been properly initialized.
     * @param battREADPin ADC pin for reading. Must have been properly initialized.
     * @return int ADC reading
     */
    int getBatteryReadingForTesting(gpio_num_t battENPin, gpio_num_t battREADPin);

    /**
     * @brief Get last known battery level
     *
     * @return int Percentage of battery charge (0% to 100%)
     */
    int getLastBatteryLevel();

    /**
     * @brief If an external power latch circuit is in place, the system will be powered off.
     *        Otherwise, deep sleep mode will be enabled.
     */
    void powerOff();
}

/**
 * @brief Configure user inputs and notify input events
 *
 */
namespace inputs
{
    /**
     * @brief Must be called before anything else in this namespace
     *
     */
    void begin();

    /**
     * @brief Start the polling loop, so input events are available from now on.
     *        Must be called after `inputHub::begin()` and also after any `add*()` or `set*()`
     *        function.
     */
    void start();

    /**
     * @brief Force an update in inputs' current state. Called from other namespaces after a 
     *        change in configuration.
     * 
     */
    void update();

    /**
     * @brief Used from input classes to report a new event.
     *        Should not be called if there is no change since the last report (of the same input).
     *
     * @param mask Bit mask for the input being reported
     * @param state Current state of such an input. Should not set any bit outside of its mask.
     */
    void notifyInputEvent(inputBitmap_t mask, inputBitmap_t state);

    /**
     * @brief Add a digital button bound to a specific input number. Must be called before `start()`
     *
     * @param[in] pinNumber Pin where the digital button is attached to
     * @param[in] pullupOrPulldown TRUE if a pullup resistor is used (LOW signal when pressed).
     *                             FALSE if a pulldown resistor is used (HIGH signal when pressed)
     * @param[in] enableInternalPull TRUE if the internal pullup or pulldown resistor must be enabled.
     *                               Ignored if the GPIO pin does not provide a pull resistor.
     * @param[in] inputNumber Requested input number for this button
     */
    void addDigital(
        gpio_num_t pinNumber,
        inputNumber_t inputNumber,
        bool pullupOrPulldown = true,
        bool enableInternalPull = true);

    /**
     * @brief Add incremental rotary encoder inputs bound to specific input numbers.
     *        Must be called before `start()`
     *
     * @param clkPin pin number attached to CLK or A
     * @param dtPin pin number attached to DT or B
     * @param[in] cwButtonNumber A number for the "virtual button" of a clockwise rotation event.
     * @param[in] ccwButtonNumber A number for the "virtual button" of a counter-clockwise rotation event.
     * @param[in] useAlternateEncoding Set to true in order to use the signal encoding of
     *                                 ALPS RKJX series of rotary encoders, and the alike.
     *
     * @note Only rotation events are considered for input. Rotary's push button must be added
     *       with `addDigital()`
     */
    void addRotaryEncoder(
        gpio_num_t clkPin,
        gpio_num_t dtPin,
        inputNumber_t cwInputNumber,
        inputNumber_t ccwInputNumber,
        bool useAlternateEncoding = false);

    /**
     * @brief Add a button matrix bound to specific button numbers.
     *        Must be called before `start()`. You can have more than one.
     *
     * @param selectorPins Array of GPIO numbers for selector pins.
     * @param selectorPinCount Length of `selectorPins` array.
     * @param inputPins Array of GPIO numbers for input pins
     * @param inputPinCount Length of `inputPins`array.
     * @param buttonNumbersArray Array of input numbers to be assigned to every button.
     *                           The length of this array is expected to match the
     *                           product of `selectorPinCount` and `inputPinCount`.
     */
    void addButtonMatrix(
        const gpio_num_t selectorPins[],
        const uint8_t selectorPinCount,
        const gpio_num_t inputPins[],
        const uint8_t inputPinCount,
        inputNumber_t *buttonNumbersArray);

    /**
     * @brief Set two potentiometers attached to clutch paddles. Each one
     *        will work as an analog axis. Must be called before `start()`.
     *
     * @param leftClutchPin ADC pin for the left clutch paddle
     * @param rightClutchPin ADC pin for the right clutch paddle.
     *        Must differ from `leftClutchPin`.
     * @param leftClutchInputNumber Input number for the left paddle
     *        (to be reported in button mode)
     * @param rightClutchInputNumber Input number for the right paddle
     *        (to be reported in button mode)
     *
     * @note Only one `set*ClutchPaddles` method can be called and only once.
     *       Otherwise an error is raised.
     */
    void setAnalogClutchPaddles(
        const gpio_num_t leftClutchPin,
        const gpio_num_t rightClutchPin,
        const inputNumber_t leftClutchInputNumber,
        const inputNumber_t rightClutchInputNumber);

    /**
     * @brief Set two inputs to digital clutch paddles. Must be called before `start()`.
     *
     * @param leftClutchInputNumber Input number for the left clutch
     * @param rightClutchInputNumber Input number for the right clutch.
     *        Must differ from `leftClutchInputNumber`.
     *
     * @note Only one `set*ClutchPaddles` method can be called and only once.
     *       Otherwise an error is raised.
     */
    void setDigitalClutchPaddles(
        const inputNumber_t leftClutchInputNumber,
        const inputNumber_t rightClutchInputNumber);

    /**
     * @brief Force autocalibration of all axes (analog clutch paddles)
     *
     */
    void recalibrateAxes();

    /**
     * @brief Exposed for testing. Do not call.
     *
     */
    void notifyInputEventForTesting(
        uint8_t id,
        inputBitmap_t bitmap,
        inputBitmap_t mask,
        clutchValue_t value);
}

/**
 * @brief Combine the state of all inputs into a single global state
 *
 */
namespace inputHub
{
    /**
     * @brief Handle a change in the state of any input. The states of all digital inputs
     *        are combined into a single global state.
     *        Will be called from a single separate background thread.
     *
     * @param globalState Current state of all inputs
     * @param changes A bit array assembling which inputs have changed since last report.
     *                1 means changed, 0 means not changed.
     *
     * @note May be called even if `changes==0`, due to changes in the state of
     *       an analog axis or a digital clutch.
     */
    void onStateChanged(inputBitmap_t globalState, inputBitmap_t changes);

    /**
     * @brief Set the bitmap for all ALT buttons. For example:
     *        `setALTBitmap(BITMAP(3)|BITMAP(4))`.
     *
     * @param altBmp Bitmap of ALT buttons.
     */
    void setALTBitmap(const inputBitmap_t altBmp);

    /**
     * @brief Set up a single button for the alternate mode.
     *        May be used instead of `setALTBitmap()`
     *
     * @param altNumber Number assigned to the ALT button
     */
    void setALTButton(const inputNumber_t altNumber);

    /**
     * @brief Set up buttons for clutch calibration while one and only one clutch paddle is pressed
     *
     * @param upButtonNumber Button number to increase bite point
     * @param downButtonNumber Button number to decrease bite point
     */
    void setClutchCalibrationButtons(
        const inputNumber_t upButtonNumber,
        const inputNumber_t downButtonNumber);

    /**
     * @brief Configure directional pad buttons
     *
     * @param padUpNumber Button number assigned to "up" direction
     * @param padDownNumber Button number assigned to "down" direction
     * @param padLeftNumber Button number assigned to "left" direction
     * @param padRightNumber Button number assigned to "right" direction
     * @param padUpLeftNumber Button number assigned to "up and left" direction (optional)
     * @param padUpRightNumber Button number assigned to "up and right" direction (optional)
     * @param padDownLeftNumber Button number assigned to "down and left" direction (optional)
     * @param padDownRightNumber Button number assigned to "down and right" direction (optional)
     *
     * @note Just one pad button can be pressed at a time, except if "diagonal" button numbers are **not** provided.
     *       In such a case, the software will combine pressed buttons from different axis.
     *       If two buttons in the same axis are pressed, nothing will be reported to the HID implementation.
     */
    void setDPADControls(
        inputNumber_t padUpNumber,
        inputNumber_t padDownNumber,
        inputNumber_t padLeftNumber,
        inputNumber_t padRightNumber,
        inputNumber_t padUpLeftNumber = UNSPECIFIED_INPUT_NUMBER,
        inputNumber_t padUpRightNumber = UNSPECIFIED_INPUT_NUMBER,
        inputNumber_t padDownLeftNumber = UNSPECIFIED_INPUT_NUMBER,
        inputNumber_t padDownRightNumber = UNSPECIFIED_INPUT_NUMBER);

    /**
     * @brief Set up a bitmap of buttons to cycle the function of "ALT" buttons (if any).
     *        All buttons in the bitmap must be pressed at the same time and none of the others.
     *
     * @param bitmap A bitmap of button numbers, for example `BITMAP(JOY_BACK) | BITMAP(JOY_A)`.
     *
     * @note Make sure all buttons in the bitmap are able to be pressed at the same time.
     */
    void setCycleALTFunctionBitmap(const inputBitmap_t bitmap);

    /**
     * @brief Set up a bitmap of buttons to cycle the function of clutch paddles (if any).
     *        All buttons in the bitmap must be pressed at the same time and none of the others.
     *
     * @param bitmap A bitmap of button numbers, for example `BITMAP(JOY_BACK) | BITMAP(JOY_B)`.
     *
     * @note Make sure all buttons in the bitmap are able to be pressed at the same time.
     */
    void setCycleClutchFunctionBitmap(const inputBitmap_t bitmap);

    /**
     * @brief Set up a bitmap of buttons to enable each specific clutch function for clutch paddles.
     *        All buttons in the bitmap must be pressed at the same time and none of the others.
     *        This is compatible with `setCycleClutchFunctionbitmap`.
     *
     * @param clutchModeBitmap A bitmap of button numbers to enable the F1-style clutch function
     * @param axisModeBitmap A bitmap of button numbers to enable the analog axes function
     * @param altModeBitmap A bitmap of button numbers to enable the "ALT" function
     * @param buttonModeBitmap A bitmap of button numbers to enable the "regular buttons" function
     *
     * @note Set the bitmap to `0` if a particular function must not be mapped to any input.
     *       Make sure all buttons in a bitmap are able to be pressed at the same time.
     */
    void setSelectClutchFunctionBitmaps(
        const inputBitmap_t clutchModeBitmap,
        const inputBitmap_t axisModeBitmap,
        const inputBitmap_t altModeBitmap,
        const inputBitmap_t buttonModeBitmap);

    /**
     * @brief Set up a bitmap of buttons for two recalibration commands.
     *        All buttons in the bitmap must be pressed at the same time and none of the others.
     * 
     * @param recalibrateAxisBitmap A bitmap of button numbers to ask for autocalibration of analog axes.
     * @param recalibrateBatteryBitmap A bitmap of button numbers to ask for recalibration of battery levels.
     * 
     * @note Set to zero if a command is not required (e.g. there is no battery)
     */
    void setCalibrationCommandBitmaps(
        const inputBitmap_t recalibrateAxisBitmap,
        const inputBitmap_t recalibrateBatteryBitmap = 0);
}

/**
 * @brief Show notifications to the user if a user interface (UI) is available.
 *        Different user interfaces may be implemented in the future.
 *
 */
namespace notify
{
    /**
     * @brief Set up an UI-dependant implementation for user notifications.
     *        Do not call if there is no user interface.
     *
     * @param implementation An object that actually implements user notifications.
     *                       Must remain valid forever (do not destroy).
     *
     */

    void begin(AbstractNotificationInterface *implementation);

    /**
     * @brief Notify current clutch's bite point
     *
     * @param aBitePoint Current bite point
     */
    void bitePoint(clutchValue_t aBitePoint);

    /**
     * @brief Notify the device is connected to a host computer
     *
     */
    void connected();

    /**
     * @brief Notify bluetooth radio in discovery mode
     * 
     */
    void BLEdiscovering();

    /**
     * @brief Notify the device is going to power off or deep sleep
     *
     */
    void powerOff();

    /**
     * @brief Notify a very low battery level
     *
     */
    void lowBattery();
}

/**
 * @brief Implementation of the "Human Interface Device"
 *
 */
namespace hidImplementation
{
    /**
     * @brief Initialize bluetooth/USB device
     *
     * @param deviceName Name of this device shown to the host computer
     * @param deviceManufacturer Name of the manufacturer of this device
     * @param enableAutoPowerOff True to power off when not connected within a certain time lapse.
     *        Set to FALSE if there is no battery or for testing.
     */
    void begin(
        std::string deviceName,
        std::string deviceManufacturer,
        bool enableAutoPowerOff = true);

    /**
     * @brief Tell if there is a Bluetooth connection
     *
     * @return true when connected to a computer
     * @return false when not connected
     */
    bool isConnected();

    /**
     * @brief Report a change in user settings (clutch function, etc.)
     *
     */
    void reportChangeInConfig();

    /**
     * @brief Report current battery level to the host computer
     *
     * @param batteryLevel Remaining battery charge as a percentage (0-100)
     */
    void reportBatteryLevel(int batteryLevel);

    /**
     * @brief Report HID inputs
     *
     * @param globalState input bitmap
     * @param altEnabled TRUE if ALT is enabled
     * @param POVtate State of the hat switch (POV or DPAD), this is, a button number
     *                in the range 0 (no input) to 8 (up-left)
     *
     * @note Axis values are taken from `clutchState`.
     */
    void reportInput(
        inputBitmap_t globalState,
        bool altEnabled,
        uint8_t POVstate);

    /**
     * @brief Report all inputs as not active
     *
     */
    void reset();
}

#endif