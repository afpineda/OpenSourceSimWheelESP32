/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Detect input events in a polling (or sampling) loop.
 *
 * @section DESCRIPTION
 *
 * Events are detected by a polling loop that runs in a separate thread. Note that rotary encoder events
 * (rotation) are not polled, but interrupt-driven.
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#ifndef __POLLEDINPUT_H__
#define __POLLEDINPUT_H__

#include "SimWheelTypes.h"
#include "esp32-hal.h"
#include <Arduino.h> // for debug

/**
 * @brief Base class for all polled inputs
 *
 */
class PolledInput
{
private:
    PolledInput *nextInChain;

public:
    // for read only
    inputBitmap_t mask;

protected:
    /**
     * @brief Compute a proper mask for consecutive inputs. Will abort execution if the resulting mask
     *        exceeds 64 bits
     *
     * @param inputsCount Number of inputs that will be reported by `read()`
     * @param firstButtonNumber Assigned number to the first input. Inputs are assumed to be numbered
     *                          in ascending order.
     */
    void updateMask(uint8_t inputsCount, inputNumber_t firstInputNumber);

    /**
     * @brief Compute a mask for an array of input numbers.
     * 
     * @param inputNumbersArray A pointer to an array of input numbers.
     * @param inputsCount Number of items (input numbers) in the previous array.
     */
    void updateMask(inputNumber_t *inputNumbersArray, uint8_t inputsCount);

public:
    /**
     * @brief Construct a new Polled Input object
     *
     * @param[in] firstInputNumber A number for the first input
     * @param[in] nextInChain Another instance to build a chain, or nullptr
     */
    PolledInput(PolledInput *nextInChain = nullptr);

    /**
     * @brief Read the current state of the inputs (pressed or released)
     *
     * @param lastState State of the same inputs as recorded in the previous iteration.
     *                  Whether the current state is unknown, `lastState` should be returned.
     *                  Should be set to zero at first call.
     * @return inputBitmap_t Current state of the inputs (a bit set to 1 means a pressed button).
     */
    virtual inputBitmap_t read(inputBitmap_t lastState) = 0;

    /**
     * @brief Get the next InputBase object in a chain
     *
     * @return PolledInput* Next object in the chain or nullptr if none
     */
    PolledInput *getNextInChain();

public:
    /**
     * @brief Check if an instance is alredy inserted in a chain
     *
     * @param instance Any instance
     * @param firstInChain Pointer to the first item in the chain
     * @return true if `instance` is chained or null
     * @return false if `instance` is not chained
     */
    static bool contains(PolledInput *instance, PolledInput *firstInChain);

    /**
     * @brief Read the combined state of all inputs in a chain
     *
     * @param lastState State of all inputs as recorded in the previous iteration.
     * @param firstInChain Pointer to the first item in the chain
     * @return inputBitmap_t Current state of all inputs
     */
    static inputBitmap_t readInChain(inputBitmap_t lastState, PolledInput *firstInChain);

    /**
     * @brief Get the combined mask of all instances in a chain
     *
     * @param firstInChain Pointer to the first item in the chain
     * @return inputBitmap_t Combined mask
     */
    static inputBitmap_t getChainMask(PolledInput *firstInChain);
};

/**
 * @brief Class for buttons attached to a single digital pin
 *
 */
class DigitalButton : public PolledInput
{
protected:
    gpio_num_t pinNumber;
    bool pullupOrPulldown;
    bool debouncing;
    inputBitmap_t bitmap;

public:
    /**
     * @brief Construct a new Digital Button object
     *
     * @param[in] pinNumber GPIO pin where the button is attached to
     * @param[in] buttonNumber Assigned number for this button
     * @param[in] pullupOrPulldown TRUE if a pullup resistor is used (LOW signal when pressed).
     *                             FALSE if a pulldown resistor is used (HIGH signal when pressed).
     * @param[in] enableInternalPull TRUE if the internal pullup or pulldown resistor must be enabled.
     *                               Ignored if the GPIO pin does not provide a pull resistor.
     * @param[in] nextInChain Another instance to build a chain or nullptr
     */
    DigitalButton(
        gpio_num_t pinNumber,
        inputNumber_t buttonNumber,
        bool pullupOrPulldown = true,
        bool enableInternalPull = true,
        PolledInput *nextInChain = nullptr);

    virtual inputBitmap_t read(inputBitmap_t lastState) override;
};

/**
 * @brief Base class for all inputs attached to an ADC pin. Note that this class is not
 *        needed. Provided for further development, if required.
 *
 * @note This class is not tested. It is not used since the underlying circuit
 *       is not suitable for this project.
 */
class AnalogInput : public PolledInput
{
protected:
    inputNumber_t firstInputNumber;
    gpio_num_t pinNumber;
    uint8_t arrayLength;
    analogReading_t *minReading;
    analogReading_t *maxReading;
protected:
    /**
     * @brief Get a reading using a software filer to reduce noise.
     *
     * @param pin ADC-capable pin number
     * @return int Analog reading
     */
    int filteredAnalogRead();
    
    /**
     * @brief Read an ADC value and return an index in `minReading`/`maxReading`
     * 
     * @return int Index of current reading in the arrays, or -1 if no match
     */
    int getReadingIndex();

public:
    /**
     * @brief Construct a new Analog Input object
     *
     * @param[in] pinNumber ADC-capable GPIO number
     * @param[in] firstInputNumber Assigned number for the first input attached to this pin.
     * @param[in] minReading Array of numbers used to translate an analog reading into a button bitmap
     * @param[in] maxReading Array of numbers used to translate an analog reading into a button bitmap
     * @param[in] arrayLength length of `minReading`/`maxReading` arrays
     * @param[in] nextInChain Another instance to build a chain or nullptr
     */
    AnalogInput(
        gpio_num_t pinNumber,
        inputNumber_t firstInputNumber,
        analogReading_t *minReading,
        analogReading_t *maxReading,
        uint8_t arrayLength,
        PolledInput *nextInChain = nullptr);
};

#endif
