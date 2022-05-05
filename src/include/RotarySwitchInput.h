/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-30
 * @brief Use an absolute rotary switch as input for a sim racing wheel or button box.
 * @note **Warning**: This code has not been tested. It is not used since the underlying
 *       circuit is not suitable for this project.
 *   
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *  
 */

#ifndef __ROTARY_SWITCH_INPUT_H__
#define __ROTARY_SWITCH_INPUT_H__

#include "PolledInput.h"
#include "SimWheelTypes.h"

class RotarySwitchInput : public AnalogInput
{
private:
    bool debouncing;
    int lastIndex;
private:
    int getClosedSwitchIndex();
public:
    RotarySwitchInput(
        gpio_num_t pinNumber,
        inputNumber_t firstInputNumber,
        uint8_t inputCount,
        analogReading_t *minReading,
        analogReading_t *maxReading,
        PolledInput *nextInChain = nullptr);

    /**
     * @brief Read the current state of the switch
     *
     * @param lastState Returned state of the previous call. This is required for debouncing.
     *        Set to zero at first call.
     * @return inputBitmap_t Current state of the switch.
     */
    virtual inputBitmap_t read(inputBitmap_t lastState) override;
};

#endif