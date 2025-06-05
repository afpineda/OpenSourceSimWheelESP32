/**
 * @file SimWheel.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-04
 * @brief Public API for firmware customization
 *
 * @copyright Licensed under the EUPL
 *
 */

#pragma once

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "InputSpecification.hpp"

#include <type_traits>

//-------------------------------------------------------------------
// Inputs
//-------------------------------------------------------------------

/**
 * @brief Everything related to hardware inputs and their events
 *
 */
namespace inputs
{
    /**
     * @brief Add a button attached to a single pin to the hardware inputs
     *
     * @note The button works in negative logic
     *
     * @param pin GPIO attached to the button
     * @param inputNumber Assigned input number
     */
    void addButton(InputGPIO pin, InputNumber inputNumber);

    /**
     * @brief Add incremental rotary encoder inputs bound to specific input numbers.
     *
     * @param clkPin GPIO attached to CLK or A
     * @param dtPin GPIO attached to DT or B
     * @param[in] cwInputNumber A number for the "virtual button" of a clockwise rotation event.
     * @param[in] ccwInputNumber A number for the "virtual button" of a counter-clockwise rotation event.
     * @param[in] useAlternateEncoding Set to true in order to use the signal encoding of
     *                                 ALPS RKJX series of rotary encoders, and the alike.
     *
     * @note Only rotation events are considered for input.
     *       Rotary's push button must be added with addButton()
     */
    void addRotaryEncoder(
        InputGPIO clkPin,
        InputGPIO dtPin,
        InputNumber cwInputNumber,
        InputNumber ccwInputNumber,
        bool useAlternateEncoding = false);

    /**
     * @brief Add a button matrix to the hardware inputs
     *
     * @param matrix Specification of input numbers and GPIO pins
     * @param negativeLogic If true, selector pins are enabled on HIGH.
     *                      If false, selector pins are enabled on LOW.
     */
    void addButtonMatrix(
        const ButtonMatrix &matrix,
        bool negativeLogic = false);

    /**
     * @brief Add a group of 8-channel multiplexers to the hardware inputs
     *
     * @note All buttons are assumed to work in negative logic
     *
     * @param selectorPin1 Least-significant selector pin
     * @param selectorPin2 Second selector pin
     * @param selectorPin3 Third selector pin
     * @param chips Array of chips sharing the given selector pins
     */
    void addAnalogMultiplexerGroup(
        OutputGPIO selectorPin1,
        OutputGPIO selectorPin2,
        OutputGPIO selectorPin3,
        const AnalogMultiplexerGroup<Mux8Pin> &chips);

    /**
     * @brief Add a group of 16-channel multiplexers to the hardware inputs
     *
     * @note All buttons are assumed to work in negative logic
     *
     * @param selectorPin1 Least-significant selector pin
     * @param selectorPin2 Second selector pin
     * @param selectorPin3 Third selector pin
     * @param selectorPin4 Fourth selector pin
     * @param chips Array of chips sharing the given selector pins
     */
    void addAnalogMultiplexerGroup(
        OutputGPIO selectorPin1,
        OutputGPIO selectorPin2,
        OutputGPIO selectorPin3,
        OutputGPIO selectorPin4,
        const AnalogMultiplexerGroup<Mux16Pin> &chips);

    /**
     * @brief Add a group of 32-channel multiplexers to the hardware inputs
     *
     * @note All buttons are assumed to work in negative logic
     *
     * @param selectorPin1 Least-significant selector pin
     * @param selectorPin2 Second selector pin
     * @param selectorPin3 Third selector pin
     * @param selectorPin4 Fourth selector pin
     * @param selectorPin5 Fifth selector pin
     * @param chips Array of chips sharing the given selector pins
     */
    void addAnalogMultiplexerGroup(
        OutputGPIO selectorPin1,
        OutputGPIO selectorPin2,
        OutputGPIO selectorPin3,
        OutputGPIO selectorPin4,
        OutputGPIO selectorPin5,
        const AnalogMultiplexerGroup<Mux32Pin> &chips);

    /**
     * @brief Add a MCP23017 GPIO expander to the hardware inputs
     *
     * @note All buttons are assumed to work in negative logic
     *
     * @param chip Specification of input numbers
     * @param address Full (7-bit) or hardware (3-bit) I2C address
     * @param isFullAddress If true, @p address is a full address,
     *                      If false, @p address is a hardware address.
     * @param bus I2C bus to which the chip is connected.
     *            If the secondary bus is used, manual initialization
     *            is required using inputs::initializeI2C()
     */
    void addMCP23017Expander(
        const MCP23017Expander &chip,
        uint8_t address,
        bool isFullAddress = false,
        I2CBus bus = I2CBus::PRIMARY);

    /**
     * @brief Add a PCF8574 GPIO expander to the hardware inputs
     *
     * @note All buttons are assumed to work in negative logic
     *
     * @param chip Specification of input numbers
     * @param address Full (7-bit) or hardware (3-bit) I2C address
     * @param isFullAddress If true, @p address is a full address,
     *                      If false, @p address is a hardware address.
     * @param bus I2C bus to which the chip is connected.
     *            If the secondary bus is used, manual initialization
     *            is required using inputs::initializeI2C()
     */
    void addPCF8574Expander(
        const PCF8574Expander &chip,
        uint8_t address,
        bool isFullAddress = false,
        I2CBus bus = I2CBus::PRIMARY);

    /**
     * @brief Add a chain of 74HC165N PISO shift registers to the hardware inputs
     *
     * @note All buttons are assumed to work in negative logic
     *
     * @param loadPin Pin attached to LOAD in the fist chip in the chain
     * @param nextPin Pin attached to NEXT in the first chip in the chain
     * @param inputPin Pin attached to INPUT in the fist chip in the chain
     * @param chain Chain of specifications of input numbers
     * @param SER_inputNumber Input number for the switch attached to the SER
     *                        pin in the last chip. Set to UNSPECIFIED::VALUE
     *                        to ignore.
     * @param negativeLogic If true, all switches must be pulled down (the default),
     *                      If false, all switches must be pulled up (positive logic).
     */
    void add74HC165NChain(
        OutputGPIO loadPin,
        OutputGPIO nextPin,
        InputGPIO inputPin,
        const ShiftRegisterChain &chain,
        InputNumber SER_inputNumber = UNSPECIFIED::VALUE,
        const bool negativeLogic = true);

    /**
     * @brief Add a binary coded rotary switch up to 8 positions
     *
     * @param spec Specification of input numbers for each switch position
     * @param pin0 Input pin for the least-significant bit
     * @param pin1 Input pin
     * @param pin2 Input pin for the most-significant bit
     * @param complementaryCode Set to true if your rotary switch uses complementary binary code
     */
    void addRotaryCodedSwitch(
        const RotaryCodedSwitch &spec,
        InputGPIO pin0,
        InputGPIO pin1,
        InputGPIO pin2,
        bool complementaryCode = true);

    /**
     * @brief Add a binary coded rotary switch up to 16 positions
     *
     * @param spec Specification of input numbers for each switch position
     * @param pin0 Input pin for the least-significant bit
     * @param pin1 Input pin
     * @param pin2 Input pin
     * @param pin3 Input pin for the most-significant bit
     * @param complementaryCode Set to true if your rotary switch uses complementary binary code
     */
    void addRotaryCodedSwitch(
        const RotaryCodedSwitch &spec,
        InputGPIO pin0,
        InputGPIO pin1,
        InputGPIO pin2,
        InputGPIO pin3,
        bool complementaryCode = true);

    /**
     * @brief Add a binary coded rotary switch up to 32 positions
     *
     * @param spec Specification of input numbers for each switch position
     * @param pin0 Input pin for the least-significant bit
     * @param pin1 Input pin
     * @param pin2 Input pin
     * @param pin3 Input pin
     * @param pin4 Input pin for the most-significant bit
     * @param complementaryCode Set to true if your rotary switch uses complementary binary code
     */
    void addRotaryCodedSwitch(
        const RotaryCodedSwitch &spec,
        InputGPIO pin0,
        InputGPIO pin1,
        InputGPIO pin2,
        InputGPIO pin3,
        InputGPIO pin4,
        bool complementaryCode = true);

    /**
     * @brief Initialize an I2C bus to certain pins
     *
     * @param sclPin SCL or SCK pin
     * @param sdaPin SDA pin
     * @param bus I2C bus to initialize
     */
    void initializeI2C(
        GPIO sclPin,
        GPIO sdaPin,
        I2CBus bus = I2CBus::PRIMARY);

    /**
     * @brief Set two potentiometers as clutch paddles.
     *        Each one will work as an analog axis.
     *
     * @param leftClutchPin ADC pin for the left clutch paddle
     * @param rightClutchPin ADC pin for the right clutch paddle.
     *        Must differ from @p leftClutchPin.
     */
    void setAnalogClutchPaddles(
        ADC_GPIO leftClutchPin,
        ADC_GPIO rightClutchPin);

} // namespace inputs

//-------------------------------------------------------------------
// Input Hub
//-------------------------------------------------------------------

/**
 * @brief Everything related to the combined state of all inputs and their treatment
 *
 */
namespace inputHub
{
    /**
     * @brief Clutch operation
     *
     */
    namespace clutch
    {
        /**
         * @brief Select two input numbers for clutch operation
         *
         * @param leftInputNumber Input number assigned to the left clutch paddle
         * @param rightInputNumber Input number assigned to the right clutch paddle
         */
        void inputs(
            InputNumber leftInputNumber,
            InputNumber rightInputNumber);

        /**
         * @brief Set inputs for clutch calibration while
         *        one and only one clutch paddle is pressed
         *
         * @param increase Input number to increase the bite point
         * @param decrease Input number to decrease then bite point
         */
        void bitePointInputs(
            InputNumber increase,
            InputNumber decrease);

        /**
         * @brief Set a combination of inputs to cycle the
         *        working mode of clutch paddles.
         *        All inputs must be activated at the same time and none of the others.
         *
         * @note Make sure all inputs can be activated at the same time.
         *
         * @param inputNumbers Array of input numbers
         */
        void cycleWorkingModeInputs(InputNumberCombination inputNumbers);

        /**
         * @brief Set a combination of inputs to command a
         *        recalibration of the analog clutch paddles.
         *
         * @note Make sure all inputs can be activated at the same time.
         *
         * @param inputNumbers Array of input numbers
         */
        void cmdRecalibrateAxisInputs(InputNumberCombination inputNumbers);

    } // namespace Clutch

    /**
     * @brief Directional pad operation
     *
     */
    namespace dpad
    {
        /**
         * @brief Configure directional pad buttons
         *
         * @param padUpNumber Input number assigned to the "up" direction
         * @param padDownNumber Input number assigned to the "down" direction
         * @param padLeftNumber Input number assigned to the "left" direction
         * @param padRightNumber Input number assigned to the "right" direction
         *
         * @note Just one pad button can be pressed at a time.
         *       The firmware will combine pressed buttons from different axes.
         *       If two buttons in the same axis are pressed,
         *       both will be ignored.
         */
        void inputs(
            InputNumber padUpNumber,
            InputNumber padDownNumber,
            InputNumber padLeftNumber,
            InputNumber padRightNumber);

        /**
         * @brief Set a combination of inputs to cycle the
         *        working mode of the DPAD.
         *        All inputs must be activated at the same time and none of the others.
         *
         * @note Make sure all inputs can be activated at the same time.
         *
         * @param inputNumbers Array of input numbers
         */
        void cycleWorkingModeInputs(InputNumberCombination inputNumbers);
    } // namespace DPAD

    /**
     * @brief ALT buttons operation
     *
     */
    namespace altButtons
    {
        /**
         * @brief Set a list of input numbers as ALT buttons.
         *        Any of them will engage the alternate mode.
         *
         * @param inputNumbers Array of input numbers
         */
        void inputs(InputNumberCombination inputNumbers);

        /**
         * @brief Set a combination of inputs to cycle the working mode of ALT buttons.
         *        All inputs must be activated at the same time and none of the others.
         *
         * @note Make sure all inputs can be activated at the same time.
         *
         * @param inputNumbers Array of input numbers
         */
        void cycleWorkingModeInputs(InputNumberCombination inputNumbers);
    } // namespace ALTButtons

    /**
     * @brief Security lock operation
     *
     */
    namespace securityLock
    {
        /**
         * @brief Set a combination of inputs to cycle the working mode of the
         *        security lock.
         *        All inputs must be activated at the same time and none of the others.
         *
         * @note Make sure all inputs can be activated at the same time.
         *
         * @param inputNumbers Array of input numbers
         */
        void cycleWorkingModeInputs(InputNumberCombination inputNumbers);
    } // namespace SecurityLock

    /**
     * @brief Neutral gear operation
     *
     */
    namespace neutralGear
    {
        /**
         * @brief Set a "virtual" button for the neutral gear
         *
         * @param neutral Input number assigned to the neutral gear
         *                "virtual" button. Should not be
         *                assigned to a hardware input.
         * @param combination A combination of input numbers to
         *                    trigger the neutral gear button.
         *                    Must be assigned to two or more
         *                    hardware inputs.
         */
        void set(
            InputNumber neutral,
            InputNumberCombination combination =
                {JOY_LSHIFT_PADDLE, JOY_RSHIFT_PADDLE});
    } // namespace neutral

} // namespace inputHub

//-------------------------------------------------------------------
// Input map
//-------------------------------------------------------------------

/**
 * @brief Translates firmware-defined input numbers to user-defined input numbers
 *
 */
namespace inputMap
{
    /**
     * @brief Set a default mapping for an input number
     *
     * @note This is just a default mapping.
     *       The user can override this setting.
     *
     * @param firmware_defined Firmware-defined input number
     * @param user_defined User-defined input number when ALT mode is not engaged
     * @param user_defined_alt_engaged User-defined input number when ALT mode is engaged
     */
    void set(
        InputNumber firmware_defined,
        UserInputNumber user_defined,
        UserInputNumber user_defined_alt_engaged);

    /**
     * @brief Set a default mapping for an input number when alternate mode is engaged
     *
     * @note This is just a default mapping.
     *       The user can override this setting.
     *
     * @param firmware_defined Firmware-defined input number
     * @param user_defined_alt_engaged User-defined input number when ALT mode is engaged
     */
    inline void set(
        InputNumber firmware_defined,
        UserInputNumber user_defined_alt_engaged)
    {
        inputMap::set(
            firmware_defined,
            static_cast<UserInputNumber>(firmware_defined),
            user_defined_alt_engaged);
    }

    /**
     * @brief Set an "optimal" default input map
     *
     * @note When ALT mode is not engaged, the user-defined input number
     *       is the firmware-defined input number.
     *       When ALT mode is engaged, the user-defined input number
     *       is the firmware-defined input number plus the highest
     *       firmware-defined input number plus one.
     */
    void setOptimal();

} // namespace inputMap

/**
 * @brief Everything related to the HID protocol
 *
 */
namespace hid
{
    /**
     * @brief Initialize Bluetooth/USB device
     *
     * @param deviceName Name of this device shown to the host computer
     * @param deviceManufacturer Name of the manufacturer of this device
     * @param enableAutoPowerOff True to shutdown when not connected within a certain time lapse.
     *                           Set to false if there is no battery or for testing.
     * @param vendorID Factory VID. Set to zero to use a default product ID.
     *                 Value 0xFFFF is reserved for testing.
     *                 Ignored in the USB implementation.
     * @param productID Factory PID. Set to zero to use a default product ID.
     *                  Value 0xFFFF is reserved for testing.
     *                  Ignored in the USB implementation.
     */
    void configure(
        std::string deviceName,
        std::string deviceManufacturer,
        bool enableAutoPowerOff = true,
        uint16_t vendorID = 0,
        uint16_t productID = 0);
} // namespace hid

//-------------------------------------------------------------------
// Power management
//-------------------------------------------------------------------

/**
 * @brief Everything related to power management
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
    void configureWakeUp(RTC_GPIO wakeUpPin);

    /**
     * @brief Configure an external latch circuit for power on and off
     *
     * @param latchPin Output-capable GPIO that drives power on/off.
     * @param mode Expected behaviour of @p latchPin to keep the power on or shutdown.
     * @param waitMs A delay to wait for the shutdown to happen, in milliseconds.
     */
    void configurePowerLatch(
        OutputGPIO latchPin,
        PowerLatchMode mode = PowerLatchMode::POWER_OPEN_DRAIN,
        uint32_t waitMs = 5000);
}

//-------------------------------------------------------------------
// Battery
//-------------------------------------------------------------------

/**
 * @brief Everything related to the measurement of available battery charge
 *
 */
namespace batteryMonitor
{
    /**
     * @brief Monitor battery charge using a battery monitor circuit
     *        or a voltage divider
     *
     * @param battREADPin ADC pin used to read battery voltage
     * @param battENPin Output pin to enable/disable the battery monitor circuit.
     *                  Set to UNSPECIFIED::VALUE if @p battREADPin
     *                  is attached to a simple voltage divider.
     *                  This is the case for most battery-enabled DevKits.
     */
    void configure(ADC_GPIO battREADPin, OutputGPIO battENPin = UNSPECIFIED::VALUE);

    /**
     * @brief Monitor battery charge using a "fuel gauge"
     *
     * @note For MAX17043 chips or compatible
     *
     * @param bus I2C bus to which the chip is connected.
     *            If the secondary bus is used, manual initialization
     *            is required using inputs::initializeI2C()
     * @param i2c_address Full I2C address of the fuel gauge chip (7 bits).
     *                    Set to 0xFF to use a default address.
     */
    void configure(I2CBus bus = I2CBus::PRIMARY, uint8_t i2c_address = 0xFF);

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
     * @param[in] percentage Value in the range from 0% (disable) to 100%.
     *                       Invalid values are ignored.
     */
    void setWarningSoC(uint8_t percentage);

    /**
     * @brief Set a battery level to shutdown the system
     *
     * @param[in] percentage Value in the range from 0% (disable) to 100%.
     *                       Invalid values are ignored.
     */
    void setPowerOffSoC(uint8_t percentage);
} // namespace batteryMonitor

//-------------------------------------------------------------------
// Pixel control
//-------------------------------------------------------------------

/**
 * @brief Everything related to pixel control
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
     * @param pixelFormat Format of color data (byte order).
     *                    Set to `AUTO` for auto-detection.
     * @param globalBrightness Global brightness for all pixels.
     *                         By default, the maximum brightness.
     *                         Set to 16 when using a 3.3V power supply
     *                         with a non-3.3V LED strip and no level shifter.
     */
    void configure(
        PixelGroup group,
        OutputGPIO dataPin,
        uint8_t pixelCount,
        bool useLevelShift,
        PixelDriver pixelType = PixelDriver::WS2812,
        PixelFormat pixelFormat = PixelFormat::AUTO,
        uint8_t globalBrightness = 0xFF);
} // namespace pixels

//-------------------------------------------------------------------
// Telemetry
//-------------------------------------------------------------------

/**
 * @brief Holds received telemetry data
 *
 */
namespace telemetry
{
    /// @brief Received telemetry data
    extern TelemetryData data;
}

//-------------------------------------------------------------------
// User interface
//-------------------------------------------------------------------

/**
 * @brief Everything related to the user interface, if available.
 *
 */
namespace ui
{
    /**
     * @brief Add a user interface instance
     *
     * @param instance Pointer to a singleton instance of the
     *                 class implementing the user interface.
     *                 Must be global (created with `new`).
     *                 Do not delete.
     */
    void add(AbstractUserInterface *instance);

    /**
     * @brief Add an user interface class
     *
     * @tparam UIClass Class implementing AbstractUserInterface
     * @tparam _Args Constructor parameter types
     * @param __args Constructor parameters
     */
    template <typename UIClass, typename... _Args>
    inline void add(_Args... __args)
    {
        static_assert(
            std::is_base_of<AbstractUserInterface, UIClass>::value,
            "Given user interface class is not derived from AbstractUserInterface");
        ui::add(new UIClass(std::forward<_Args>(__args)...));
    }

    /**
     * @brief Add UI notifications using pixel control
     *
     * @note No effect unless pixel control is configured
     */
    inline void addPixelControlNotifications()
    {
        ui::add(PixelControlNotification::getInstance());
    }
}

//-------------------------------------------------------------------
// Firmware
//-------------------------------------------------------------------

/**
 * @brief Firmware operation
 *
 */
namespace firmware
{
    /// @brief Run the custom firmware (non blocking)
    void run();
    /// @brief Run the custom firmware and show errors (non blocking)
    /// @param func Main firmware configuration body
    void run(void (*func)());
}
