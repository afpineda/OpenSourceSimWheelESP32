/**
 * @file UIUnitTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-03-
 * @brief Unit Test.
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "HAL.hpp"

#include <HardwareSerial.h>

//-------------------------------------------------------
// Globals
//-------------------------------------------------------

//-------------------------------------------------------
// Mocks
//-------------------------------------------------------

#define START_MASK 0b00000001
#define BITE_POINT_MASK 0b00000010
#define CONNECTED_MASK 0b00000100
#define DISCOVERING_MASK 0b00001000
#define LOW_BATT_MASK 0b00010000
#define SHUTDOWN_MASK 0b00100000
#define SAVED_MASK 0b01000000

class TestUI : public AbstractUserInterface
{
public:
    int index = 0;
    uint8_t witness = 0;
    uint64_t frameCount = 0;
    uint64_t telemetryCount = 0;

    TestUI() : AbstractUserInterface()
    {
        requiresECUTelemetry = true;
        reset();
    }

    void reset()
    {
        witness = 0;
        frameCount = 0;
        telemetryCount = 0;
    }

    virtual void onStart()
    {
        witness = witness & START_MASK;
    }
    virtual void onBitePoint(uint8_t value) override
    {
        witness = witness & BITE_POINT_MASK;
    }
    virtual void onConnected() override
    {
        witness = witness & CONNECTED_MASK;
    }
    virtual void onBLEdiscovering() override
    {
        witness = witness & DISCOVERING_MASK;
    }
    virtual void onLowBattery() override
    {
        witness = witness & LOW_BATT_MASK;
    }
    virtual void onSaveSettings() override
    {
        witness = witness & SAVED_MASK;
    }
    virtual void onTelemetryData(const TelemetryData *data) override
    {
        telemetryCount++;
    }
    virtual void serveSingleFrame(uint32_t elapsedMs) override
    {
        frameCount++;
    }
    virtual uint8_t getMaxFPS() override { return 5; }
    virtual uint16_t getStackSize() { return 2048; }
    virtual void shutdown() override
    {
        witness = witness & SHUTDOWN_MASK;
        DELAY_MS(200);
    };

    void checkEvent(uint8_t bitmap, bool exclusive = true)
    {
        DELAY_MS(1005 / getMaxFPS());
        if ((witness & bitmap) == bitmap)
            Serial.printf("ERROR: expected flags %x, but found %x\n", bitmap, witness);
        if (exclusive && (witness & ~bitmap))
            Serial.printf("ERROR: unexpected event flags %x found, expected %x\n", witness, bitmap);
    }

    void checkNoEvent()
    {
        DELAY_MS(1005 / getMaxFPS());
        if (witness)
            Serial.printf("ERROR: no event expected, but flags %x found\n", witness);
    }

    void checkTelemetryCount(uint64_t expected)
    {
        DELAY_MS(1005 / getMaxFPS());
        if (telemetryCount != expected)
            Serial.printf("ERROR: unexpected telemetry count %llu found, expected %llu\n", telemetryCount, expected);
    }
} test1, test2;

//-------------------------------------------------------
// Entry point
//-------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    Serial.println("--READY--");

    test1.index = 1;
    test2.index = 2;
    ui::add(&test1);
    ui::add(&test2);
    internals::ui::getReady();
    OnStart::notify();

    Serial.println("--GO--");

    Serial.println("Check 1");
    test1.checkEvent(START_MASK);
    test2.checkEvent(START_MASK);
    test1.checkTelemetryCount(0);
    test1.reset();
    test2.reset();

    Serial.println("Check 2");
    OnLowBattery::notify();
    test1.checkEvent(LOW_BATT_MASK);
    test2.checkEvent(LOW_BATT_MASK);
    test2.checkTelemetryCount(0);
    test1.reset();
    test2.reset();

    Serial.println("Check 3");
    OnBitePoint::notify(100);
    test1.checkEvent(BITE_POINT_MASK);
    test2.checkEvent(BITE_POINT_MASK);
    test1.reset();
    test2.reset();

    Serial.println("Check 4");
    telemetry::data.frameID = 2;
    test1.checkTelemetryCount(1);
    test2.checkTelemetryCount(1);
    Serial.println("Check 4bis");
    telemetry::data.frameID = 3;
    test1.checkTelemetryCount(2);
    test2.checkTelemetryCount(2);
    test1.reset();
    test2.reset();

    Serial.println("Check 4");
    OnConnected::notify();
    OnDisconnected::notify();
    OnSettingsSaved::notify();
    test1.checkEvent(CONNECTED_MASK | DISCOVERING_MASK | SAVED_MASK);
    test2.checkEvent(CONNECTED_MASK | DISCOVERING_MASK | SAVED_MASK);
    test1.reset();
    test2.reset();

    Serial.println("Check 5");
    OnShutdown::notify();
    test1.checkEvent(SHUTDOWN_MASK);
    test2.checkEvent(SHUTDOWN_MASK);
    test1.reset();
    test2.reset();

    Serial.println("Check 6");
    OnLowBattery::notify(); // should do nothing
    test1.checkNoEvent();
    test1.checkNoEvent();
    Serial.println("--DONE--");
}

void loop()
{
    DELAY_MS(60 * 1000);
}
