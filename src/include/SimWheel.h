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
     * 
     * @param newFlag A device capability.
     * @param setOrClear When true, the flag is set. Otherwise, the flag is cleared. 
     *                   Should not be called more than once for each flag.
     */
    void setFlag(deviceCapability_t newFlag, bool setOrClear=true);
}

/**
 * @brief Language for the user interface
 *
 */
namespace language
{
    /**
     * @brief Must be called before anything else in this namespace. Will
     *        load the configured UI language from flash memory.
     *
     **/
    void begin();

    /**
     * @brief Set the current language for the UI. Will be stored in flash memory.
     *
     * @param lang Language
     */
    void setLanguage(language_t lang);

    /**
     * @brief Retrieve the current language for the UI
     *
     * @return language_t Current language for the UI
     */
    language_t getLanguage();
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
     * @param battENPin Output pin to enable/disable the battery monitor circuit
     * @param battREADPin ADC pin used to read battery voltage
     * @param testing Set to TRUE for unit testing. Will get a battery sample every 5 seconds.
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
     * @return int Percentage of battery charge
     */
    int getLastBatteryLevel();

    /**
     * @brief Check if the battery monitor is running
     *
     * @return true If the battery monitor is running
     * @return false Otherwise
     */
    bool hasBatteryMonitor();

    /**
     * @brief If an external power latch circuit is in place, the system will be powered off.
     *        Otherwise, deep sleep mode will be enabled.
     *
     * @param forced Set to TRUE when requested by the user, FALSE otherwise.
     */
    void powerOff(bool forced = false);
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
     * @brief Used from input classes to report a new event.
     *        Should not be called if there is no change since the last report (of the same input).
     *
     * @param mask Bit mask for the input being reported
     * @param state Current state of such an input. Should not set any bit outside of its mask.
     */
    void notifyInputEvent(inputBitmap_t mask, inputBitmap_t state);

    /**
     * @brief Add a digital button. Must be called before `start()`
     *
     * @param[in] pinNumber Pin where the digital button is attached to
     * @param[in] pullupOrPulldown TRUE if a pullup resistor is used (LOW signal when pressed).
     *                             FALSE if a pulldown resistor is used (HIGH signal when pressed)
     * @param[in] enableInternalPull TRUE if the internal pullup or pulldown resistor must be enabled.
     *                               Ignored if the GPIO pin does not provide a pull resistor.
     * @return inputNumber_t Number assigned to the button.
     */
    inputNumber_t addDigital(
        gpio_num_t pinNumber,
        bool pullupOrPulldown = true,
        bool enableInternalPull = true);

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
    void addDigitalExt(
        gpio_num_t pinNumber,
        inputNumber_t inputNumber,
        bool pullupOrPulldown = true,
        bool enableInternalPull = true);

    /**
     * @brief Add incremental rotary encoder inputs. Must be called before `start()`
     *
     * @param clkPin pin number attached to CLK or A
     * @param dtPin pin number attached to DT or B
     * @param[in] useAlternateEncoding Set to true in order to use the signal encoding of
     *                                 ALPS RKJX series of rotary encoders, and the alike.
     * @return inputNumber_t Number assigned to the clockwise rotation. `Number+1` is the
     *         assigned number to the counter-clockwise rotation.
     * @note Only rotation events are considered for input. Rotary's push button must be added
     *       with `addDigital()`
     */
    inputNumber_t addRotaryEncoder(
        gpio_num_t clkPin,
        gpio_num_t dtPin,
        bool useAlternateEncoding = true);

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
    void addRotaryEncoderExt(
        gpio_num_t clkPin,
        gpio_num_t dtPin,
        inputNumber_t cwInputNumber,
        inputNumber_t ccwInputNumber,
        bool useAlternateEncoding = true);

    /**
     * @brief Setup a single button matrix. Must be called before `start()`
     *
     * @param selectorPins Array of pin numbers used as selector pins
     * @param selectorPinCount Length of the `selectorPins` array
     * @param inputPins Array of pin numbers used as input pins
     * @param inputPinCount Length of the `inputPins` array
     * @return inputNumber_t Number assigned to the first button.
     */
    inputNumber_t setButtonMatrix(
        const gpio_num_t selectorPins[],
        const uint8_t selectorPinCount,
        const gpio_num_t inputPins[],
        const uint8_t inputPinCount);

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
    void addButtonMatrixExt(
        const gpio_num_t selectorPins[],
        const uint8_t selectorPinCount,
        const gpio_num_t inputPins[],
        const uint8_t inputPinCount,
        inputNumber_t *buttonNumbersArray);
}

/**
 * @brief Combine the state of all inputs into a single global state
 *
 */
namespace inputHub
{
    /**
     * @brief Must be called before anything else in this namespace
     *
     */
    void begin();

    /**
     * @brief Handle a change in the state of any input. The states of all inputs
     *        are combined into a single global state.
     *        Will be called from a single separate background thread.
     *
     * @param globalState Current state of all inputs
     * @param changes A bit array assembling which inputs have changed since last report.
     *                1 means changed, 0 means not changed.
     */
    void onStateChanged(inputBitmap_t globalState, inputBitmap_t changes);

    /**
     * @brief Get current clutch bite point
     *
     * @return calibrated clutch value
     */
    clutchValue_t getClutchBitePoint();

    /**
     * @brief Get current function of ALT buttons
     *
     * @return TRUE if ALT buttons trigger alternate mode. FALSE if ALT buttons are reported
     *         as regular buttons.
     */
    bool getALTFunction();

    /**
     * @brief Get current function of clutch paddles
     *
     * @return Clutch paddles function
     */
    clutchFunction_t getClutchFunction();

    /**
     * @brief Check if clutch paddles have been set up
     *
     * @return FALSE if there are no clutch paddles
     */
    bool hasClutchPaddles();

    /**
     * @brief Check if ALT buttons have been set up
     *
     * @return FALSE if there are no ALT buttons
     */
    bool hasALTButtons();

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
     * @brief Set the calibrated bite point for the clutch
     *
     * @param calibrationValue calibrated value of the clutch
     * @param save Request autosave
     */
    void setClutchBitePoint(clutchValue_t calibrationValue, bool save = false);

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
     * @brief Set a function for the ALT buttons
     *
     * @param altFunction TRUE if ALT buttons must enable alternate mode. FALSE if
     *        ALT button inputs are to be reported as regular inputs. Default is TRUE.
     * @param save Request autosave
     */
    void setALTFunction(bool altFunction, bool save = false);

    /**
     * @brief Set a function for the clutch paddles
     *
     * @param newFunction Requested function for the clutch paddles
     * @param save Request autosave
     */
    void setClutchFunction(clutchFunction_t newFunction, bool save = false);

    /**
     * @brief Set up buttons for clutch operation
     *
     * @param leftClutchNumber Button number for the left clutch paddle
     * @param rightClutchNumber Button number for the right clutch paddle
     */
    void setClutchPaddles(
        const inputNumber_t leftClutchNumber,
        const inputNumber_t rightClutchNumber);

    /**
     * @brief Set a button number to invoke the configuration menu.
     *        May be used instead of `setMenuBitmap()`.
     *
     * @note This button must be pressed for two seconds or more, and then released, for the menu to show up
     *       No other button must be pressed at the same time.
     *       If no long press is detected, it will be reported as a regular button.
     *
     * @param menuButtonNumber Button number assigned to the menu function
     */
    void setMenuButton(const inputNumber_t menuButtonNumber);

    /**
     * @brief Set a bitmap to invoke the configuration menu. All buttons
     *        in the bitmap must be pressed at the same time, and no other button.
     *        May be used instead of `setMenuButton()`.
     *
     * @note These buttons must be pressed for two seconds or more, and then released, for the menu to show up.
     *       If no long press is detected, they will be reported as regular buttons.
     *
     * @param menuButtonsBitmap Bitmap of buttons assigned to the menu function
     */
    void setMenuBitmap(const inputBitmap_t menuButtonsBitmap);

    /**
     * @brief Called from `configMenu` to notify that the menu is not shown anymore
     *
     */
    void notifyMenuExit();

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
}

/**
 * @brief Coordinate the use of the OLED display from multiple threads
 *
 */
namespace uiManager
{
    /**
     * @brief Must be called before anything else in this namespace
     *
     */
    void begin();

    /**
     * @brief Acquire exclusive access to the display buffer assigned to a certain screen priority
     *
     * @param priority Screen priority
     * @return uint8_t* buffer used to draw the screen
     *
     * @note Every call to `enterDisplay` must be followed by a call to `exitDisplay`.
     *       Two calls in a row will cause a deadlock.
     */
    uint8_t *enterDisplay(screenPriority_t priority);

    /**
     * @brief Release exclusive access to a buffer and display it.
     *
     * @param priority Screen priority
     * @param autoHide If TRUE, this screen will be cleared after a few seconds.
     *
     * @note Every call to `exitDisplay` must be preceded by a single call to `enterDisplay`.
     *
     */
    void exitDisplay(screenPriority_t priority, bool autoHide);

    /**
     * @brief Acquire a buffer for the frame server daemon.
     *        Must be called from a single thread. Reserved for use by a frame server.
     *
     * @return uint8_t* buffer used to draw the screen. Must not be shared betweeen threads.
     */
    uint8_t *getFrameServerBuffer();

    /**
     * @brief Display the buffer assigned to the frame server.
     *        Must be called from a single thread. Reserved for use by a frame server.
     *
     */
    void unsafeDisplayFrameServerBuffer();

    /**
     * @brief Clear screen
     *
     * @param priority Screen priority
     *
     * @note Must not be called between `enterDisplay` and `exitDisplay`, otherwise a deadlock
     *       will occur.
     */
    void hide(screenPriority_t priority);
}

/**
 * @brief Implementation of a user interface
 *
 */
namespace ui
{
    /**
     * @brief Initialize display
     *
     * @param pixels_width Resolution width in pixels
     * @param pixels_height Resolution height in pixels
     * @param displayType Type of display controller
     * @param flipUpsideDown Set to true if the display is mounted upside down
     * @return true OLED is available
     * @return false OLED is not available
     */
    bool begin(
        int pixels_width = 128,
        int pixels_height = 64,
        displayType_t displayType = SSOLED_132x64,
        bool flipUpsideDown = false);

    /**
     * @brief Clear the screen. Called exclusively from `uiManager`. Must not call `uiManager`.
     */
    void clear();

    /**
     * @brief Display the given buffer. Called exclusively from `uiManager`. Must not call `uiManager`.
     *
     * @param buffer data to display
     */
    void display(uint8_t *buffer);

    /**
     * @brief Display current clutch's bite point
     *
     * @param value Bite point
     * @return Screen ID
     */
    void showBitePoint(clutchValue_t value);

    /**
     * @brief Show a menu screen
     *
     * @param title Menu title
     * @param selection Currently selected option or submenu
     */
    void showMenu(const char *title, const char *selection);

    /**
     * @brief Show message
     *
     * @param title Title. Optional: Can be null
     * @param info Message to display
     * @param priority Screen priority
     *
     * @note Will auto-hide after a few seconds
     */
    void showInfo(const char *title, const char *info, screenPriority_t priority = SCR_INFO_PRIORITY);

    /**
     * @brief Show message and wait for it to disapear.
     *
     * @param title Title. Optional: Can be null
     * @param info Message to display
     * @param priority Screen priority
     *
     * @note Will not return inmediately, but after a few seconds. Use wisely.
     */
    void showModal(const char *title, const char *info, screenPriority_t priority = SCR_INFO_PRIORITY);

    /**
     * @brief Macro to hide SCR_MENU_PRIORITY screen
     *
     */
    void hideMenu();

    /**
     * @brief Macro to show a data save notice
     *
     */
    void showSaveNote();

    /**
     * @brief Macro to show a Bluetooth connection notice
     *
     */
    void showConnectedNotice();

    /**
     * @brief Macro to tell the user that Bluetooth discovery is active
     *
     */
    void showBLEDiscoveringNotice();

    /**
     * @brief Macro to warn the user that the battery charge is about to deplete
     *
     */
    void showLowBatteryNotice();

    /**
     * @brief Cut power to the display in order to preserve battery.
     *        Will be called before system shutdown.
     */
    void turnOff();

    /**
     * @brief Activate or deactivate the frame server in order to
     *        display simulation data (low priority)
     *
     * @param state TRUE to enable, FALSE to disable
     */
    void frameServerSetEnabled(bool state);

    /**
     * @brief  Check if the OLED is available
     *
     * @return true Available
     * @return false Not available
     */
    bool isAvailable();
}

/**
 * @brief Operarion of the user config menu
 *
 */
namespace configMenu
{
    /**
     * @brief Show configuration menu if not shown and vice-versa
     *
     * @return true Menu is shown
     * @return false Menu is not shown or there is no user interface available
     */
    bool toggle();

    /**
     * @brief Route input events to the configuration menu, when shown
     *
     * @param globalState input bitmap
     * @param changes bitmap of inputs that have changed since last report
     *
     * @note Called exclusivelly from `inputHub`
     */
    void onInput(inputBitmap_t globalState, inputBitmap_t changes);

    /**
     * @brief Configured buttons used to navigate through menu options
     *
     * @param prevButtonNumber button number for previous option or submenu
     * @param nextButtonNumber button number for next option or submenu
     * @param selectButtonNumber button number to select current option or enter submenu
     * @param cancelButtonNumber button number to go back to previous menu
     */
    void setNavButtons(
        inputNumber_t prevButtonNumber,
        inputNumber_t nextButtonNumber,
        inputNumber_t selectButtonNumber,
        inputNumber_t cancelButtonNumber);
}

/**
 * @brief Implementation of the "Human Interface Device" protocol and UART service
 *
 */
namespace hidImplementation
{
    /**
     * @brief Initialize bluetooth device
     *
     * @param deviceName Name of this device shown to the host computer
     * @param deviceManufacturer Name of the manufacturer of this device
     * @param enableAutoPowerOff True to power off when not connected within a certain time lapse. Set to FALSE for testing.
     * @param enableUART True to enable the NuS protocol. False to disable.
     */
    void begin(std::string deviceName, std::string deviceManufacturer, bool enableAutoPowerOff = true, bool enableUART = true);

    /**
     * @brief Tell if there is a Bluetooth connection
     *
     * @return true when connected to a computer
     * @return false when not connected
     */
    bool isConnected();

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
     * @param clutchValue Current value of the clutch axis
     * @param POVtate State of the hat switch (POV or DPAD), this is, a button number
     *                in the range 0 (no input) to 8 (up-left)
     */
    void reportInput(
        inputBitmap_t globalState,
        bool altEnabled,
        clutchValue_t clutchValue,
        uint8_t POVstate = 0);

    /**
     * @brief Report all inputs as not active
     *
     */
    void reset();

    /**
     * @brief Send text through the UaS protocol
     *
     * @param text text to be sent
     * @return true if sucessfull
     * @return false on error
     */
    bool uartSendText(char *text);
}

/**
 * @brief Receive and process commands through the UART service at `hidImplementation`
 *
 */
namespace uartServer
{
    void onReceive(char *text);

    // For read only
    extern volatile char gear;
    extern volatile uint8_t rpmPercent;
    extern volatile uint16_t speed;
    extern volatile uint8_t engineMap;
    extern volatile uint8_t absLevel;
    extern volatile uint8_t tcLevel;
    extern volatile uint64_t frameCount;
}

#endif