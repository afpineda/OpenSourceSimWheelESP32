/**
 * @file HidServiceTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-10-30
 * @brief Unit test
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "InternalServices.hpp"
#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"

#include <cassert>
#include <iostream>
#include <cstdint>

#define FACTORY_VID 0x10
#define FACTORY_PID 0x11

int main()
{
    // Note: this a regression test for a bug not registered in GitHub

    hid::configure("none", "none", false, FACTORY_VID, FACTORY_PID);
    internals::hid::common::getReady();
    uint16_t vid, pid;

    HidService::call::getCustomHardwareID(vid, pid);
    assert(vid == 0);
    assert(pid == 0);
    HidService::call::setCustomHardwareID(0x5050, 0xABAB);
    HidService::call::getCustomHardwareID(vid, pid);
    assert(vid == 0x5050);
    assert(pid == 0xABAB);
    HidService::call::setCustomHardwareID(0, 0);
    HidService::call::getCustomHardwareID(vid, pid);
    assert(vid == 0);
    assert(pid == 0);

    return 0;
}