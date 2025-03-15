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
            void reserve(const std::vector<GPIOtype> &collection)
            {
                for (auto item : collection)
                    item.reserve();
            }

            /**
             * @brief Validate a button matrix
             *
             * @param matrix Button matrix to validate
             */
            void buttonMatrix(const ButtonMatrix &matrix)
            {
                OutputGPIOCollection selectors;
                InputGPIOCollection inputs;
                uint64_t previousInputNumbers = InputNumber::booked();
                for (ButtonMatrix::const_iterator row = matrix.begin(); row != matrix.end(); row++)
                {
                    OutputGPIO selectorPin = row->first;
                    addIfNotExists<OutputGPIO>(selectorPin, selectors);
                    for (std::map<InputGPIO, InputNumber>::const_iterator col = row->second.begin(); col != row->second.end(); col++)
                    {
                        InputGPIO inputPin = col->first;
                        addIfNotExists<InputGPIO>(inputPin, inputs);
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
            void analogMultiplexer(const OutputGPIOCollection &selectors, const AnalogMultiplexerGroup<PinTags> chips)
            {
                uint64_t previousInputNumbers = InputNumber::booked();
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
            void shiftRegisterChain(
                OutputGPIO loadPin,
                OutputGPIO nextPin,
                InputGPIO inputPin,
                const ShiftRegisterChain &chain)
            {
                uint64_t previousInputNumbers = InputNumber::booked();
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
            void GPIOExpander(const GPIOExpanderChip<PinTags> &chip)
            {
                uint64_t previousInputNumbers = InputNumber::booked();
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
            void rotaryEncoder(InputGPIO dtPin, InputGPIO clkPin, InputNumber cw, InputNumber ccw)
            {
                uint64_t previousInputNumbers = InputNumber::booked();
                dtPin.reserve();
                clkPin.reserve();
                cw.book();
                ccw.book();
                if (previousInputNumbers == InputNumber::booked())
                    throw empty_input_number_set("rotary encoder");
                if (cw==UNSPECIFIED::VALUE)
                    throw std::runtime_error("Useless rotary encoder: no input number for clockwise rotation");
                if (ccw==UNSPECIFIED::VALUE)
                    throw std::runtime_error("Useless rotary encoder: no input number for counter-clockwise rotation");
                if (cw==ccw)
                    throw std::runtime_error("Useless rotary encoder: same input numbers for clockwise and counter-clockwise");
            }

            /**
             * @brief Validate a single button
             *
             * @param pin Input pin
             * @param inputNumber Input number
             */
            void button(InputGPIO pin, InputNumber inputNumber)
            {
                pin.reserve();
                if (inputNumber == UNSPECIFIED::VALUE)
                    throw empty_input_number_set("single button");
                inputNumber.book();
            }
        } // namespace validate
    } // namespace inputs
} // namespace internals
