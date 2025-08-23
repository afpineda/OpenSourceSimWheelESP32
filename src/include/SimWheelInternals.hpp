/**
 * @file SimWheelInternals.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-11
 * @brief System functionality not exposed to the end user.
 *
 * @copyright Licensed under the EUPL
 *
 */

#pragma once

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "InternalTypes.hpp"
#include "SimWheelTypes.hpp"

//-------------------------------------------------------------------
// GLOBALS
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// API
//-------------------------------------------------------------------

namespace internals
{
    namespace batteryMonitor
    {
        /**
         * @brief Prepare to run
         *
         */
        void getReady();

        /**
         * @brief Configure period, warning and power-off limits for testing.
         *
         */
        void configureForTesting();

        /**
         * @brief Configure a fake monitor for testing
         *
         * @param fakeStatus Pointer to variable holding the fake battery status
         */
        void configureFakeMonitor(BatteryStatus *fakeStatus);


        /**
         * @brief Get the Hardware Instance object
         *
         * @note For testing
         *
         * @return void* Untyped hardware instance (must be type-casted)
         */
        void *getHardwareInstance();

    } // namespace batteryMonitor

    namespace batteryCalibration
    {
        /**
         * @brief Prepare to run
         *
         */
        void getReady();

        /**
         * @brief Clear calibration data (but may persist in flash memory).
         *
         */
        void clear();

        /**
         * @brief Add an ADC reading to calibration data.
         *        The battery should get fully charged before first call.
         *
         * @param reading An ADC reading of current battery(+) voltage.
         */
        void addSample(int reading);

        /**
         * @brief Save calibration data to flash memory.
         *        The battery should get almost fully depleted before calling.
         *
         */
        // void save();
    }

    namespace pixels
    {
        /**
         * @brief Prepare to run
         *
         */
        void getReady();

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
        void set(PixelGroup group,
                 uint8_t pixelIndex,
                 uint8_t red,
                 uint8_t green,
                 uint8_t blue);

        /**
         * @brief Set the color of all pixels in a group
         *
         * @param group A group of pixels
         * @param red Red component of the pixel color
         * @param green Green component of the pixel color
         * @param blue Blue component of the pixel color
         */
        void setAll(PixelGroup group,
                    uint8_t red,
                    uint8_t green,
                    uint8_t blue);

        /**
         * @brief Shift all pixel colors to the next pixel index
         *
         * @param group A group of pixels
         */
        void shiftToNext(PixelGroup group);

        /**
         * @brief Shift all pixel colors to the previous pixel index
         *
         * @param group A group of pixels
         */
        void shiftToPrevious(PixelGroup group);

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
        uint8_t getCount(PixelGroup group);
    } // namespace pixels

    namespace ui
    {
        /**
         * @brief Prepare to run
         */
        void getReady();
    } // namespace ui

    namespace inputs
    {
        /**
         * @brief Prepare to run
         *
         */
        void getReady();

        /**
         * @brief Add a fake input instance for testing
         *
         * @param instance Fake iput instance
         */
        void addFakeInput(FakeInput *instance);

        /**
         * @brief Push an input event into the decoupling queue
         *        (for testing)
         *
         * @param input Event to push
         */
        inline void notifyInputEvent(const DecouplingEvent &input);
    } // namespace inputs

    namespace inputHub
    {
        /**
         * @brief Check that user configuration is correct
         *
         */
        void getReady();

        /**
         * @brief Process a single input event
         *
         * @param input Event to process
         */
        void onRawInput(DecouplingEvent &input);
    } // namespace inputHub

    namespace storage
    {
        /// @brief Prepare to run
        void getReady();
    } // namespace storage

    namespace inputMap
    {
        /**
         * @brief Prepare to run
         *
         */
        void getReady();

        /**
         * @brief Clear the whole map (for testing)
         *        and the custom defaults.
         *
         */
        void clear();

        /**
         * @brief Map a firmware-defined input bitmap
         *
         * @param isAltModeEngaged True if ALT mode is engaged,
         *                         false otherwise.
         * @param firmware_bitmap Firmware-defined input bitmap
         * @param low Least significant 64-bits of the user-defined map
         * @param high Most significant 64-bits of the user-defined map
         */
        void map(
            bool isAltModeEngaged,
            uint64_t firmware_bitmap,
            uint64_t &low,
            uint64_t &high);
    } // namespace inputMap

    namespace power
    {
        /**
         * @brief Prepare to run
         *
         */
        void getReady();
    } // namespace power

    namespace hid
    {

        /**
         * @brief Start BLE/Bluetooth HID
         *
         * @param deviceName Configured device name
         * @param deviceManufacturer Configured device manufacturer
         * @param enableAutoPowerOff Request for automatic shutdown (may be ignored)
         * @param VID Configured custom vendor ID (may be ignored)
         * @param PID Configured custom product ID (may be ignored)
         */
        void begin(
            std::string deviceName,
            std::string deviceManufacturer,
            bool enableAutoPowerOff,
            uint16_t VID,
            uint16_t PID);

        /**
         * @brief Support for a custom PID/VID
         *
         * @return true If VID and PID can be provided externally
         *              via begin()
         * @return false If the externally provided VID/PID are ignored
         */
        bool supportsCustomHardwareID();

        /**
         * @brief Tell if there is a host connection
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
         * @warning May be called before internals::hid::begin()
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
            uint64_t inputsLow,
            uint64_t inputsHigh,
            uint8_t POVstate,
            uint8_t leftAxis,
            uint8_t rightAxis,
            uint8_t clutchAxis);

        /**
         * @brief Report all inputs as not active
         *
         */
        void reset();

        /**
         * @brief Common functionality to all HID implementations
         *
         * @note Implemented at file `hidCommon.cpp`
         *
         */
        namespace common
        {
            /// @brief Prepare to run
            void getReady();

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
             * @brief  Sets data for the input report
             *
             * @param report Pointer to report buffer.
             *               Size is defined by GAMEPAD_REPORT_SIZE.
             * @param notifyConfigChanges True to notify changes in the device settings
             * @param inputsLow State of inputs (low-order bytes)
             * @param inputsHigh State of inputs (high-order bytes)
             * @param POVstate State of the DPAD (aka "point of view")
             * @param leftAxis State of the left axis
             * @param rightAxis State of the right axis
             * @param clutchAxis State of the clutch axis
             */
            void onReportInput(
                uint8_t *report,
                bool &notifyConfigChanges,
                uint64_t &inputsLow,
                uint64_t &inputsHigh,
                uint8_t &POVstate,
                uint8_t &leftAxis,
                uint8_t &rightAxis,
                uint8_t &clutchAxis);
        } // namespace common
    } // namespace hid
} // namespace internals

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------
