/**
 * @file InputValidation.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-04
 * @brief Validation of input hardware specifications
 *
 * @copyright Licensed under the EUPL
 *
 */

#pragma once

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "InputSpecification.hpp"

//-------------------------------------------------------------------
// API
//-------------------------------------------------------------------

namespace internals
{
    namespace inputs
    {
        /**
         * @brief Validation of user-defined input hardware
         *
         */
        namespace validate
        {
            /**
             * @brief Reserve a collection of GPIO pins
             *
             * @tparam GPIOtype GPIO or subtype
             * @param collection Pins to reserve
             */
            template <typename GPIOtype>
            inline void reserve(const std::set<GPIOtype> &collection)
            {
                for (auto item : collection)
                    item.reserve();
            }

            /**
             * @brief Validate a button matrix
             *
             * @param matrix Button matrix to validate
             */
            inline void buttonMatrix(const ButtonMatrix &matrix)
            {
                OutputGPIOCollection selectors;
                InputGPIOCollection inputs;
                uint128_t previousInputNumbers = InputNumber::booked();
                for (ButtonMatrix::const_iterator row = matrix.cbegin();
                     row != matrix.cend();
                     row++)
                {
                    OutputGPIO selectorPin = row->first;
                    selectors.insert(selectorPin);
                    for (std::map<InputGPIO, InputNumber>::const_iterator col = row->second.begin(); col != row->second.end(); col++)
                    {
                        InputGPIO inputPin = col->first;
                        inputs.insert(inputPin);
                        InputNumber inputNumber = col->second;
                        inputNumber.book();
                    }
                }
                reserve<OutputGPIO>(selectors);
                reserve<InputGPIO>(inputs);
                if (previousInputNumbers == InputNumber::booked())
                    throw empty_input_number_set("button matrix");
            }

            /**
             * @brief Validate a group of analog multiplexers
             *
             * @tparam PinTags Pin tags
             * @param selectors Collection of selector pins
             * @param chips Collections of chips
             */
            template <typename PinTags>
            inline void analogMultiplexer(const OutputGPIOCollection &selectors, const AnalogMultiplexerGroup<PinTags> chips)
            {
                uint128_t previousInputNumbers = InputNumber::booked();
                reserve<OutputGPIO>(selectors);
                for (auto chip : chips)
                    chip.reserve_and_book();
                if (previousInputNumbers == InputNumber::booked())
                    throw empty_input_number_set("analog multiplexers");
            }

            /**
             * @brief Validate a chain of PISO shift registers
             *
             * @param loadPin Output GPIO attached to LOAD
             * @param nextPin Output GPIO attached to NEXT
             * @param inputPin Input GPIO pin
             * @param chain Collection of chips
             */
            inline void shiftRegisterChain(
                OutputGPIO loadPin,
                OutputGPIO nextPin,
                InputGPIO inputPin,
                const ShiftRegisterChain &chain)
            {
                uint128_t previousInputNumbers = InputNumber::booked();
                loadPin.reserve();
                nextPin.reserve();
                inputPin.reserve();
                for (auto chip : chain)
                {
                    for (ShiftRegisterChip::const_iterator i = chip.begin(); i != chip.end(); i++)
                        (i->second).book();
                }
                if (previousInputNumbers == InputNumber::booked())
                    throw empty_input_number_set("PISO shift registers");
            }

            /**
             * @brief Validate a GPIO expander
             *
             * @tparam PinTags Pin tags
             * @param chip Chip instance
             */
            template <typename PinTags>
            inline void GPIOExpander(const GPIOExpanderChip<PinTags> &chip)
            {
                uint128_t previousInputNumbers = InputNumber::booked();
                for (auto i = chip.begin(); i != chip.end(); i++)
                    (i->second).book();
                if (previousInputNumbers == InputNumber::booked())
                    throw empty_input_number_set("GPIO expander");
            }

            /**
             * @brief Validate a rotary encoder
             *
             * @param dtPin DT pin
             * @param clkPin CLK pin
             * @param cw Input number for clockwise rotation
             * @param ccw Input number for counter-clockwise rotation
             */
            inline void rotaryEncoder(
                InputGPIO dtPin,
                InputGPIO clkPin,
                InputNumber cw,
                InputNumber ccw)
            {
                uint128_t previousInputNumbers = InputNumber::booked();
                dtPin.reserve();
                clkPin.reserve();
                cw.book();
                ccw.book();
                if (previousInputNumbers == InputNumber::booked())
                    throw empty_input_number_set("rotary encoder");
                if (cw == UNSPECIFIED::VALUE)
                    throw std::runtime_error("Useless rotary encoder: no input number for clockwise rotation");
                if (ccw == UNSPECIFIED::VALUE)
                    throw std::runtime_error("Useless rotary encoder: no input number for counter-clockwise rotation");
                if (cw == ccw)
                    throw std::runtime_error("Useless rotary encoder: same input numbers for clockwise and counter-clockwise");
            }

            /**
             * @brief Validate a single button
             *
             * @param pin Input pin
             * @param inputNumber Input number
             */
            inline void button(InputGPIO pin, InputNumber inputNumber)
            {
                pin.reserve();
                if (inputNumber == UNSPECIFIED::VALUE)
                    throw empty_input_number_set("single button");
                inputNumber.book();
            }

            /**
             * @brief Validate a joystick
             *
             * @param xAxisPin ADC-capable pin for the horizontal axis
             * @param yAxisPin ADC-capable pin for the vertical axis
             * @param up Input number assigned to the "up" direction
             * @param down Input number assigned to the "down" direction
             * @param left Input number assigned to the "left" direction
             * @param right Input number assigned to the "right" direction
             */
            inline void joystick(
                ADC_GPIO xAxisPin,
                ADC_GPIO yAxisPin,
                InputNumber up,
                InputNumber down,
                InputNumber left,
                InputNumber right)
            {
                if (xAxisPin == yAxisPin)
                    throw std::runtime_error(
                        "inputs::addJoystick() is using the same pin for both axes");
                xAxisPin.reserve();
                yAxisPin.reserve();
                up.book();
                down.book();
                left.book();
                right.book();
            }

        } // namespace validate
    } // namespace inputs
} // namespace internals
