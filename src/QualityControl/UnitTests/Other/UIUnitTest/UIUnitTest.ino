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

class TestUI : public AbstractUserInterface
{
public:
    int index = 0;
    bool start = false;
    bool bitePoint = false;
    bool connected = false;
    bool BLEdiscovering = false;
    bool lowBatt = false;
    bool shutdownWitness = false;
    bool saved = false;
    uint64_t frameCount = 0;
    uint64_t telemetryCount = 0;

    TestUI() : AbstractUserInterface()
    {
        requiresECUTelemetry = true;
    }

    virtual void onStart()
    {
        start = true;
    }
    virtual void onBitePoint(uint8_t value) override
    {
        bitePoint = true;
    }
    virtual void onConnected() override
    {
        connected = true;
    }
    virtual void onBLEdiscovering() override
    {
        BLEdiscovering = true;
    }
    virtual void onLowBattery() override
    {
        lowBatt = true;
    }
    virtual void onSaveSettings() override
    {
        saved = true;
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
        shutdownWitness = true;
        DELAY_MS(200);
    };
} test1, test2;

//-------------------------------------------------------
// Auxiliary
//-------------------------------------------------------

void check_instance(const TestUI &instance)
{
    Serial.printf("Checking instance %d\n", instance.index);
    if (!instance.bitePoint)
        Serial.println("ERROR: bitepoint event failed");
    if (!instance.start)
        Serial.println("ERROR: start event failed");
    if (!instance.BLEdiscovering)
        Serial.println("ERROR: disconnection event failed");
    if (!instance.connected)
        Serial.println("ERROR: connection event failed");
    if (!instance.lowBatt)
        Serial.println("ERROR: low battery event failed");
    if (!instance.saved)
        Serial.println("ERROR: save settings event failed");
    if (instance.frameCount == 0)
        Serial.println("ERROR: frame server failed");
    if (instance.telemetryCount != 1)
        Serial.println("ERROR: telemetry event failed");
}

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
    OnLowBattery::notify();
    OnBitePoint::notify(100);
    telemetry::data.frameID = 2;
    OnConnected::notify();
    OnDisconnected::notify();
    OnSettingsSaved::notify();
    DELAY_MS(2000);
    check_instance(test1);
    check_instance(test2);
    OnShutdown::notify();
    if (!test1.shutdownWitness)
        Serial.println("ERROR: instance 1 failed to shutdown");
    if (!test2.shutdownWitness)
        Serial.println("ERROR: instance 2 failed to shutdown");
    test1.lowBatt = false;
    test2.lowBatt = false;
    OnLowBattery::notify(); // should do nothing
    if (test1.lowBatt)
        Serial.println("ERROR: instance 1 still running");
    if (test2.lowBatt)
        Serial.println("ERROR: instance 2 still running");
    Serial.println("--DONE--");
}

void loop()
{
    DELAY_MS(60 * 1000);
}
