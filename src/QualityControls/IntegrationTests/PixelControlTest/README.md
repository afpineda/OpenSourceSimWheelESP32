# Integration test: Pixel control via HID output reports

## Purpose and summary

To test that pixel data is received and properly displayed in an LED strip.

## Hardware setup

An LED strip is required:

- No level shifter (3.3V logic).
- Eight pixels.
- Pixel driver: WS2812 family.
- `Din` attached to the GPIO pin aliased as [TEST_D_OUT](../../../include/debugUtils.h)

Output through USB serial port at 115200 bauds.

## Software setup

SimHub must be installed in the test machine.
The official plugin must be installed and enabled.
However, no game is required.
We are using pre-recorded telemetry data found in the
[TestTelemetryData](../TelemetryIntegrationTest/TestTelemetryData/) folder.

1. Ensure the device is connected to the host computer through BLE.
2. Run SimHub. Enable the official plugin if asked to.
3. Choose **Automobilista 2** as active game (click on `change active game`).
   **This is a must**.
4. Click `Refresh` if the device does not show up.
5. Select the `LEDs` tab page.
6. In `Telemetry Leds` select `Test profile for rev lights (8 pixels)`.
   If that profile does not show up, click on `Import profile` and
   select the file
   [Any Game - Test profile for rev lights (8 pixels).ledsprofile](<./LedProfiles/Any Game - Test profile for rev lights (8 pixels).ledsprofile>)
   which is located in the [LedProfiles subfolder](./LedProfiles/).
7. Place the pre-recorded files in the SimHub folder if not done yet:
   - Click on `Open replay folder`.
   - Copy pre-recorded files to that folder.
   - Once done, close the window and click on `Replay` again.
8. Choose the recording and hit the play button.

## Procedure and expected output

**Important note**:
this test departs a lot from others because there is no way to predict
a "correct" behavior.

The LED strip should reflect the engine speed
as configured in the LED profile.
