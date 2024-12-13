/**
 * @file SimWheel.h
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Main header for common sources
 *
 * @section DESCRIPTION
 *
 * This file declares all the namespaces that builds the system architecture.
 * See the [Firmware Architecture document](../../doc/firmware/FirmwareArchitecture_en.md).
 *
 * @copyright Licensed under the EUPL
 *
 */

#ifndef __SIM_WHEEL_H__
#define __SIM_WHEEL_H__

#include "SimWheelTypes.h"
// #include "esp32-hal.h" // declares gpio_num_t
#include <string>

/**
 * @brief Device capabilities
 *
 */
namespace capabilities
{
    // For read only. Do not write
    extern volatile uint32_t flags;
    extern volatile inputBitmap_t availableInputs;

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
    bool hasFlag(deviceCapability_t flag);

    /**
     * @brief Add an input number to the set of available input numbers
     *
     * @param[in] number A firmware-defined input number.
     */
    void addInputNumber(inputNumber_t number);
}

/**
 * @brief Current state of the clutch paddles (if any) and,
 *        for convenience, working mode of "ALT" buttons
 *
 */
namespace userSettings
{
    /**
     * @brief Current clutch's bite point.
     * @note For read only. Do not overwrite.
     */
    extern volatile clutchValue_t bitePoint;

    /**
     * @brief Current working mode of clutch paddles.
     * @note For read only. Do not overwrite.
     */
    extern volatile clutchFunction_t cpWorkingMode;

    /**
     * @brief Current working mode of ALT buttons.
     * @note For read only. Do not overwrite.
     */
    extern volatile bool altButtonsWorkingMode;

    /**
     * @brief Current working mode of directional pad.
     * @note For read only. Do not overwrite.
     */
    extern volatile bool dpadWorkingMode;

    /**
     * @brief User-defined buttons map.
     * @note For read only. Do not overwrite.
     */
    extern volatile inputNumber_t buttonsMap[2][64];

    /**
     * @brief Security lock on HID reports.
     * @note For read only. Do not overwrite.
     */
    extern volatile bool securityLock;

    /**
     * @brief Must be called before anything else in this namespace. Will
     *        load user settings from flash memory.
     *
     * @note Do not call while testing. This way, autosaving will get disabled.
     **/
    void begin();
    //
    /**
     * @brief Set working mode for "ALT" buttons
     *
     * @param newMode When True, "ALT" buttons should engage "ALT" mode.
     *                Otherwise, they behave as regular buttons.
     */
    void setALTButtonsWorkingMode(bool newMode);

    /**
     * @brief Assign a working mode to the clutch paddles
     *
     * @param newFunction Function of the clutch paddles
     */
    void setCPWorkingMode(clutchFunction_t newFunction);

    /**
     * @brief Set the current bite point
     *
     * @param newBitePoint Clutch bite point
     */
    void setBitePoint(clutchValue_t newBitePoint);

    /**
     * @brief Set working mode of directional pad
     *
     * @param[in] newMode True for navigation. False for regular buttons.
     */
    void setDPADWorkingMode(bool newMode);

    /**
     * @brief Reset user-defined buttons map to "factory defaults"
     * @note Not automatically saved.
     */
    void resetButtonsMap();

    /**
     * @brief Maps a single "firmware" input number to a user-defined button number
     *
     * @note Changes are **not** automatically saved to flash memory. Must call saveNow().
     *
     * @param altMode True if this setting is intended for "ALT" mode, false otherwise.
     * @param rawInputNumber Firmware-defined input number in the range 0-63.
     * @param userInputNumberNoAlt User-defined input number in the range 0-127
     *                             to be used when ALT mode is disengaged.
     *                             Set to `UNSPECIFIED_INPUT_NUMBER` for "factory defaults".
     * @param userInputNumberAlt User-defined input number in the range 0-127
     *                             to be used when ALT mode is engaged.
     *                             Set to `UNSPECIFIED_INPUT_NUMBER` for "factory defaults".
     */
    void setButtonMap(inputNumber_t rawInputNumber,
                      inputNumber_t userInputNumberNoAlt,
                      inputNumber_t userInputNumberAlt);

    /**
     * @brief Force permanent storage of all user settings immediately
     *
     * @note This is the only way to save the user-defined buttons map.
     */
    void saveNow();

    /**
     * @brief Get the user-defined button map or a factory default
     *
     * @param[in] rawInputNumber Firmware-defined input number in the range 0-63.
     * @param[out] userInputNumberNoAlt User-defined or factory default input number
     *                                  to be used when ALT mode is disengaged.
     * @param[out] userInputNumberAlt User-defined or factory default input number
     *                                to be used when ALT mode is engaged.
     * @return true On success
     * @return false If @p rawInputNumber is not valid
     */
    bool getEffectiveButtonMap(inputNumber_t rawInputNumber,
                               inputNumber_t &userInputNumberNoAlt,
                               inputNumber_t &userInputNumberAlt);

    /**
     * @brief Lock or unlock all writing attempts to HID reports
     *
     * @details For security concerns, the user can lock or unlock all writing
     *          attempts to HID reports by using just hardware inputs.
     *          This is a security precaution to stop unauthorized configuration
     *          modifications caused by rogue programs.
     *
     * @note By default, unlocked.
     *
     * @param yesOrNo true to lock, false to unlock.
     */
    void setSecurityLock(bool yesOrNo);
}

/**
 * @brief Battery profiling algorithm
 *
 * @note This namespace provides two algorithms to map a battery reading into a
 *       battery level percentage:
 *       - Calibrated: a calibration procedure is required
 *       - Auto-calibrated: fallback algorithm based on LiPo characterization data.
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
     *        Calibration data will **not** be available until `calibrationInProgress` is set to false.
     *
     * @param reading An ADC reading of current battery(+) voltage.
     * @param save When true, the given sample will be saved to flash memory.
     */
    void addSample(int reading, bool save = false);

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
     * @brief Maximum battery reading ever. Exposed for testing. DO NOT TOUCH.
     *
     */
    extern volatile int maxBatteryReadingEver;

    /**
     * @brief Restart autocalibration algorithm.
     *
     */
    void restartAutoCalibration();

    /**
     * @brief Get a percentage of battery charge using auto-calibration.
     *        Will provide incorrect battery levels (higher) until
     *        the battery is fully charged.
     *        Anyway, this algorithm is not accurate.
     *
     * @param reading An ADC reading of current battery(+) voltage
     *
     * @return If auto-calibration is available, a percentage in the range 0%-100%.
     *         Otherwise, the constant `UNKNOWN_BATTERY_LEVEL`.
     *
     * @note Based on https://blog.ampow.com/lipo-voltage-chart/
     */
    int getBatteryLevelAutoCalibrated(int reading);
}

/**
 * @brief Measurement of State of Charge
 *
 * @note Do not call if there is no battery.
 */
namespace batteryMonitor
{
    /**
     * @brief Monitor battery charge using a voltage divider or a battery monitor circuit
     *
     * @param battENPin Output pin to enable/disable the battery monitor circuit.
     *                  Set to `GPIO_NUM_NC` (-1) if `battREADPin` is attached to
     *                  a simple voltage divider. This is the case for most battery-enabled DevKits.
     * @param battREADPin ADC pin used to read battery voltage
     */
    void begin(gpio_num_t battENPin, gpio_num_t battREADPin);

    /**
     * @brief Monitor battery charge using a "fuel gauge"
     *
     * @note For MAX17043 chips or compatible
     *
     * @param i2c_address Full I2C address of the fuel gauge chip (7 bits).
     *                    Set to 0xFF to use a default address.
     */
    void begin(uint8_t i2c_address = 0xFF);

    /**
     * @brief Get last known battery level
     *
     * @return int Percentage of battery charge (0% to 100%)
     */
    int getLastBatteryLevel();

    /**
     * @brief Set time interval between measurements
     *
     * @param[in] seconds Time to wait from one measurement to the next (in seconds).
     *                    Set to zero for a default interval.
     */
    void setPeriod(uint32_t seconds);

    /**
     * @brief Set a battery level to warn to the user
     *
     * @note No effect if there is no user interface.
     *
     * @param[in] percentage. Value in the range from 0% (disable) to 100%.
     *                        Invalid values are ignored.
     */
    void setWarningSoC(uint8_t percentage);

    /**
     * @brief Set a battery level to shutdown the system
     *
     * @param[in] percentage. Value in the range from 0% (disable) to 100%.
     *                        Invalid values are ignored.
     */
    void setPowerOffSoC(uint8_t percentage);

    /**
     * @brief Configure period, warning and power-off limits for testing.
     *
     * @note No need to call in user code.
     */
    void configureForTesting();
}

/**
 * @brief Power management
 *
 */
namespace power
{
    /**
     * @brief Initialize power management
     *
     * @param wakeUpPin RTC-capable GPIO pin to wake up the system after deep sleep.
     *                  That pin is supposed to activate on LOW voltage.
     */
    void begin(const gpio_num_t wakeUpPin);

    /**
     * @brief Configure an external latch circuit for power on and off
     *
     * @param latchPin Output-capable GPIO that drives power on/off.
     * @param mode Expected behaviour of 'latchPin' to keep power on or go to power off.
     * @param waitTicks A delay to wait for power off to happen, in milliseconds.
     */
    void setPowerLatch(gpio_num_t latchPin, powerLatchMode_t mode, uint32_t waitMs);

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
     * @brief Start the polling loop, so input events are available from now on.
     *        Must be called after any `add*()` or `set*()`
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
     * @brief Add a digital button bound to a specific input number. Must be called before `start()`
     *
     * @note Low voltage is expected when active.
     *
     * @param[in] pinNumber Pin where the digital button is attached to
     * @param[in] inputNumber Requested input number for this button
     */
    void addDigital(gpio_num_t pinNumber, inputNumber_t inputNumber);

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
     * @brief Add a button matrix.
     *        Must be called before `start()`. You can have more than one.
     *
     * @param selectorPins Array of GPIO numbers for selector pins.
     * @param inputPins Array of GPIO numbers for input pins
     *
     * @return ButtonMatrixInputSpec& Input numbers specification
     */
    ButtonMatrixInputSpec &addButtonMatrix(
        const gpio_num_array_t selectorPins,
        const gpio_num_array_t inputPins);

    /**
     * @brief Add analog multiplexers for switches.
     *        Must be called before `start()`. You can have more than one.
     *
     * @param selectorPins Array of GPIO numbers for selector pins.
     * @param inputPins Array of GPIO numbers for input pins.
     *
     * @return Multiplexers8InputSpec& Input numbers specification
     */
    Multiplexers8InputSpec &addAnalogMultiplexer(
        const gpio_num_array_t &selectorPins,
        const gpio_num_array_t &inputPins);

    /**
     * @brief Add (a chain of) PISO shift registers for switches.
     *        Must be called before `start()`. You can have more than one.
     *
     * @param serialPin GPIO number of the serial output pin
     * @param loadPin GPIO number of the load pin
     * @param nextPin GPIO number of the next/clock pin
     * @param switchCount Count of switches
     *
     * @return ShiftRegisters8InputSpec& Input numbers specification
     */
    ShiftRegisters8InputSpec &addShiftRegisters(
        const gpio_num_t serialPin,
        const gpio_num_t loadPin,
        const gpio_num_t nextPin,
        const uint8_t switchCount);

    /**
     * @brief Add a PCF8574 GPIO expander for switches.
     *        Must be called before `start()`. You can have more than one.
     *
     * @param I2CAddress Either a hardware address (3 bits) or a full address (7 bits).
     * @param isFullAddress `true` if @p I2CAddress is a full address, `false` otherwise.
     *
     * @return PCF8574InputSpec& Input numbers specification
     */
    PCF8574InputSpec &addPCF8574Digital(
        uint8_t I2CAddress,
        bool isFullAddress = false);

    /**
     * @brief Add a MCP23017 GPIO expander for switches.
     *        Must be called before `start()`. You can have more than one.
     *
     * @param I2CAddress Either a hardware address (3 bits) or a full address (7 bits).
     * @param isFullAddress `true` if @p I2CAddress is a full address, `false` otherwise.
     *
     * @return MCP23017InputSpec& Input numbers specification
     */
    MCP23017InputSpec &addMCP23017Digital(
        uint8_t I2CAddress,
        bool isFullAddress = false);

    /**
     * @brief Initialize the primary I2C bus to certain pins.
     *
     * @note Must be called if your board does not feature a default I2C bus.
     *       Otherwise, there is no need to call, since the bus will be
     *       automatically initialized.
     * @note If required, must be called before addPCF8574Digital() or addMCP23017Digital()
     *
     * @param sdaPin SDA pin for the I2C bus.
     * @param sclPin SCL pin for the I2C bus.
     */
    void initializeI2C(gpio_num_t sdaPin, gpio_num_t sclPin);

    /**
     * @brief Set two potentiometers attached to clutch paddles. Each one
     *        will work as an analog axis. Must be called before `start()`.
     *
     * @param leftClutchPin ADC pin for the left clutch paddle
     * @param rightClutchPin ADC pin for the right clutch paddle.
     *        Must differ from `leftClutchPin`.
     */
    void setAnalogClutchPaddles(
        const gpio_num_t leftClutchPin,
        const gpio_num_t rightClutchPin);

    /**
     * @brief Force autocalibration of all axes (analog clutch paddles)
     *
     */
    void recalibrateAxes();

    /**
     * @brief Change polarity of left axis (if any)
     *
     * @note Saved to flash memory without delay
     */
    void reverseLeftAxis();

    /**
     * @brief Change polarity of right axis (if any)
     *
     * @note Saved to flash memory without delay
     */
    void reverseRightAxis();

    /**
     * @brief Exposed for testing. Do not call.
     *
     */
    void notifyInputEventForTesting(
        inputBitmap_t state,
        clutchValue_t leftAxisValue,
        clutchValue_t rightAxisValue);
}

/**
 * @brief Combine the state of all inputs into a single global state
 *
 */
namespace inputHub
{
    /**
     * @brief Handle raw user input
     *
     * @note Raw user input is translated into wheel functionality or HID reports.
     *
     * @param[in] rawInputBitmap Combined current state of all inputs
     * @param[in] rawInputChanges A bit array assembling which inputs have changed since last call.
     *                            1 means changed, 0 means not changed.
     * @param[in] leftAxis Position of the left analog clutch paddle (zero if not available)
     * @param[in] rightAxis Position of the right analog clutch paddle (zero if not available)
     * @param[in] axesAvailable True if there are analog clutch paddles. False otherwise.
     */
    void onRawInput(
        inputBitmap_t rawInputBitmap,
        inputBitmap_t rawInputChanges,
        clutchValue_t leftAxis,
        clutchValue_t rightAxis,
        bool axesAvailable);

    /**
     * @brief Set a list of input numbers for the "ALT" function.
     *        Any of them may engage "ALT" mode.
     *
     * @param inputNumbers Array of input numbers
     */
    void setALTInputNumbers(const inputNumberCombination_t inputNumbers);

    /**
     * @brief Assign two input numbers to work as clutch paddles and vice-versa
     *
     * @note If there are analog clutch paddles, the given parameters will be used in the "buttons"
     *       working mode of clutch paddles. If there are no analog clutch paddles,
     *       the given parameters must have been assigned to
     *       existing input hardware, so they become "digital" clutch paddles.
     *
     * @note The firmware automatically translates an input number to an axis position and vice-versa,
     *       depending on the user-selected mode for clutch paddles.
     *
     * @param leftClutchInputNumber Input number for the left clutch
     * @param rightClutchInputNumber Input number for the right clutch.
     *                               Must differ from @p leftClutchInputNumber.
     */
    void setClutchInputNumbers(
        const inputNumber_t leftClutchInputNumber,
        const inputNumber_t rightClutchInputNumber);

    /**
     * @brief Set inputs for clutch calibration while one and only one clutch paddle is pressed
     *
     * @param upButtonNumber Input number to increase bite point
     * @param downButtonNumber Input number to decrease bite point
     */
    void setClutchCalibrationInputNumbers(const inputNumber_t incInputNumber, const inputNumber_t decInputNumber);

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
     * @brief Set a combination of inputs to cycle the working mode of "ALT" buttons (if any).
     *        All inputs must be activated at the same time and none of the others.
     *
     * @note Make sure all inputs can be activated at the same time.
     *
     * @param inputNumbers Array of input numbers
     */
    void cycleALTButtonsWorkingMode_setInputNumbers(const inputNumberCombination_t inputNumbers);

    /**
     * @brief Set a combination of inputs to cycle the working mode of clutch paddles (if any).
     *        All inputs must be activated at the same time and none of the others.
     *
     * @note Make sure all inputs can be activated at the same time.
     *
     * @param inputNumbers Array of input numbers
     */
    void cycleCPWorkingMode_setInputNumbers(const inputNumberCombination_t inputNumbers);

    /**
     * @brief Set a combination of inputs to cycle the working mode of DPAD controls (if any).
     *        All inputs must be activated at the same time and none of the others.
     *
     * @note Make sure all inputs can be activated at the same time.
     *
     * @param inputNumbers Array of input numbers
     */
    void cycleDPADWorkingMode_setInputNumbers(const inputNumberCombination_t inputNumbers);

    /**
     * @brief Set several combinations of input numbers to engage each specific working mode of clutch paddles.
     *        All inputs (at each combination) must be activated at the same time and none of the others.
     *
     * @note Make sure all inputs can be activated at the same time.
     *
     * @param clutchModeCombination Array of input numbers for clutch mode
     * @param axisModeCombination Array of input numbers for axis mode
     * @param altModeCombination Array of input numbers for "ALT" mode
     * @param buttonModeCombination Array of input numbers for regular buttons mode
     */
    void cpWorkingMode_setInputNumbers(
        const inputNumberCombination_t clutchModeCombination,
        const inputNumberCombination_t axisModeCombination,
        const inputNumberCombination_t altModeCombination,
        const inputNumberCombination_t buttonModeCombination);

    /**
     * @brief Set a combination of inputs for analog axis calibration (at clutch paddles).
     *        All inputs must be activated at the same time and none of the others.
     *
     * @note Make sure all inputs can be activated at the same time.
     *
     * @param inputNumbers Array of input numbers
     */
    void cmdRecalibrateAnalogAxis_setInputNumbers(const inputNumberCombination_t inputNumbers);

    /**
     * @brief Set a combination of inputs for battery SoC calibration (if available).
     *        All inputs must be activated at the same time and none of the others.
     *
     * @note Make sure all inputs can be activated at the same time.
     *
     * @param inputNumbers Array of input numbers
     */
    void cmdRecalibrateBattery_setInputNumbers(const inputNumberCombination_t inputNumbers);

    /**
     * @brief Set a combination of inputs to cycle the state of the security lock.
     *        All inputs must be activated at the same time and none of the others.
     *
     * @param inputNumbers Array of input numbers
     */
    void cycleSecurityLock_setInputNumbers(const inputNumberCombination_t inputNumbers);
}

/**
 * @brief Show notifications to the user if a user interface (UI) is available.
 *        Different user interfaces may be implemented in the future.
 *
 */
namespace notify
{
    /**
     * @brief Last received telemetry data. Not for user code.
     *        Exposed for efficiency. Do not overwrite.
     *
     */
    extern volatile telemetryData_t telemetryData;

    /**
     * @brief Maximum supported FPS value. Not for user code.
     *        Exposed for efficiency. Do not overwrite.
     */
    extern uint8_t maxFPS;

    /**
     * @brief Set up an UI-dependant implementation for user notifications.
     *        Do not call if there is no user interface.
     *
     * @param implementors Array of pointers to objects implementing
     *                     the abstract user interface.
     *
     * @param framesPerSecond Target FPS for the frameserver.
     *                        Set to zero to disable.
     *
     * @param stackSize Stack size in bytes for the notification/frameserver daemon.
     *                  Set to zero to use a default value.
     *                  Increase stack size if you get "stack canary"
     *                  or "dual exception" errors and reboots.
     *
     * @note @p implementors objects must remain valid forever.
     *       Do not destroy.
     */

    void begin(
        notificationImplementorsArray_t implementors,
        uint8_t framesPerSecond = 0,
        uint16_t stackSize = 0);

    /**
     * @brief Notify current clutch's bite point.
     *
     */
    void bitePoint();

    /**
     * @brief Notify the device is connected to a host computer.
     *
     */
    void connected();

    /**
     * @brief Notify Bluetooth radio is in discovery mode.
     *
     */
    void BLEdiscovering();

    /**
     * @brief Notify a very low battery level.
     *
     */
    void lowBattery();
}

/**
 * @brief Implementation of the "Human Interface Device".
 *
 */
namespace hidImplementation
{
    /**
     * @brief Initialize Bluetooth/USB device
     *
     * @param deviceName Name of this device shown to the host computer
     * @param deviceManufacturer Name of the manufacturer of this device
     * @param enableAutoPowerOff True to power off when not connected within a certain time lapse.
     *        Set to FALSE if there is no battery or for testing.
     * @param productID Factory PID. Set to zero to use a default product ID.
     *                  Value 0xFFFF is reserved for testing.
     *                  Ignored in the USB implementation.
     */
    void begin(
        std::string deviceName,
        std::string deviceManufacturer,
        bool enableAutoPowerOff = true,
        uint16_t productID = 0);

    /**
     * @brief Tell if there is a Bluetooth/USB connection
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
     * @param[in] inputsLow State of input numbers 0 to 63
     * @param[in] inputsHigh State of input numbers 64 to 127
     * @param[in] POVstate State of the hat switch (POV or DPAD), this is, a button number
     *                     in the range 0 (no input) to 8 (up-left).
     * @param[in] leftAxis Position of the left clutch, in the range 0-254.
     * @param[in] rightAxis Position of the right clutch, in the range 0-254.
     * @param[in] clutchAxis Position of the combined clutch, in the range 0-254.
     */
    void reportInput(
        inputBitmap_t inputsLow,
        inputBitmap_t inputsHigh,
        uint8_t POVstate,
        clutchValue_t leftAxis,
        clutchValue_t rightAxis,
        clutchValue_t clutchAxis);

    /**
     * @brief Report all inputs as not active
     *
     */
    void reset();

    /**
     * @brief Common functionality to all HID implementations
     *
     * @note Implemented at file `hidImplementation_common.cpp`
     *
     */
    namespace common
    {
        /**
         * @brief Set a default hardware ID for this device
         *
         * @note If not called, default hardware ID is zero.
         *
         * @param vid Default vendor ID.
         * @param pid Default product ID.
         */
        void setFactoryHardwareID(uint16_t vid, uint16_t pid);

        /**
         * @brief Get the stored custom hardware ID
         *
         * @note Stored in flash memory.
         *
         * @param[out] vid Custom vendor ID or factory VID if not stored.
         * @param[out] pid Custom product ID or factory PID if not stored.
         */
        void loadHardwareID(uint16_t &vid, uint16_t &pid);

        /**
         * @brief Store a custom hardware ID for BLE devices
         *
         * @note Changes will be effective after the next reboot
         *
         * @param[in] vid Custom vendor ID
         * @param[in] pid Custom product ID
         */
        void storeHardwareID(uint16_t vid, uint16_t pid);

        /**
         * @brief Clear stored PID and VID (restore factory defaults)
         *
         */
        void clearStoredHardwareID();

        /**
         * @brief Send feature report
         *
         * @param[in] report_id A valid report id
         * @param[out] buffer Pointer to buffer to be sent
         * @param[in] len Size of @p buffer
         * @return uint16_t Count of bytes put into @p buffer
         */
        uint16_t onGetFeature(uint8_t report_id, uint8_t *buffer, uint16_t len);

        /**
         * @brief Receive a feature report
         *
         * @param[in] report_id Report identification
         * @param[in] buffer Pointer to buffer that contains received data
         * @param[in] len Size of @p buffer
         */
        void onSetFeature(uint8_t report_id, const uint8_t *buffer, uint16_t len);

        /**
         * @brief Receive an output report
         *
         * @param[in] report_id Report identification
         * @param[in] buffer Pointer to buffer that contains received data
         * @param[in] len Size of @p buffer
         */
        void onOutput(uint8_t report_id, const uint8_t *buffer, uint16_t len);

        /**
         * @brief Resets data for the input report
         *
         * @param[out] report Pointer to report buffer.
         *                    Size is defined by GAMEPAD_REPORT_SIZE.
         */
        void onReset(uint8_t *report);

        /**
         * @brief Sets data for the input report
         *
         * @param[out] report Pointer to report buffer.
         *                    Size is defined by GAMEPAD_REPORT_SIZE.
         */
        void onReportInput(
            uint8_t *report,
            bool &notifyConfigChanges,
            inputBitmap_t &inputsLow,
            inputBitmap_t &inputsHigh,
            uint8_t &POVstate,
            clutchValue_t &leftAxis,
            clutchValue_t &rightAxis,
            clutchValue_t &clutchAxis);
    }
}

/**
 * @brief Pixel control
 *
 */
namespace pixels
{
    /**
     * @brief Configure an LED strip for pixel control
     *
     * @param group Group of pixels to configure
     * @param dataPin GPIO number attached to `Din` (data input)
     *                in the LED strip. Must be output-capable.
     * @param pixelCount  Total count of pixels in the strip.
     * @param useLevelShift Set to `false` when using 3.3V logic.
     *                      Set to `true` when using the level
     *                      shifter in open-drain mode.
     * @param pixelType Pixel driver.
     * @param pixelFormat  Format of color data (byte order).
     *                    Set to `AUTO` for auto-detection.
     */
    void configure(
        pixelGroup_t group,
        gpio_num_t dataPin,
        uint8_t pixelCount,
        bool useLevelShift,
        pixel_driver_t pixelType = WS2812,
        pixel_format_t pixelFormat = pixel_format_t::AUTO);

    /**
     * @brief Set the color of a single pixel
     *
     * @note Not displayed immediately
     * @note Non-existing pixels will be ignored
     *
     * @param group The group to which the pixel is a member
     * @param pixelIndex Index of the pixel in the LED strip
     *                   (zero-based)
     * @param red Red component of the pixel color
     * @param green Green component of the pixel color
     * @param blue Blue component of the pixel color
     */
    void set(pixelGroup_t group,
             uint8_t pixelIndex,
             uint8_t red,
             uint8_t green,
             uint8_t blue);

    /**
     * @brief Display all pixels in all groups at once
     *
     */
    void show();

    /**
     * @brief Turn off all pixels in all groups
     *
     */
    void reset();

    /**
     * @brief Get the total number of pixels in a group
     *
     * @param group Group of pixels
     * @return byte Number of pixels in the given group
     */
    uint8_t getPixelCount(pixelGroup_t group);
}

#endif