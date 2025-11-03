/**
 * @file hid_dummy.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-11
 * @brief Dummy HID implementation for testing
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheelInternals.hpp"
#include "SimWheel.hpp"

bool internals::hid::isConnected() { return true; }
bool internals::hid::supportsCustomHardwareID() { return true; }
void internals::hid::reportChangeInConfig() {}
void internals::hid::reportBatteryLevel(int batteryLevel) {}
void internals::hid::reportBatteryLevel(const BatteryStatus &status) {}
void internals::hid::reportInput(
    uint64_t inputsLow,
    uint64_t inputsHigh,
    uint8_t POVstate,
    uint8_t leftAxis,
    uint8_t rightAxis,
    uint8_t clutchAxis) {}

void internals::hid::reset() {}

void internals::hid::begin(
    std::string deviceName,
    std::string deviceManufacturer,
    bool enableAutoPowerOff,
    uint16_t vendorID,
    uint16_t productID) {}

