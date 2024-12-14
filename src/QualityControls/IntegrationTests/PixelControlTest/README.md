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

In the test computer:

- SimpleHIDWrite.exe: available at [http://janaxelson.com/hidpage.htm](http://janaxelson.com/hidpage.htm).
  There is a modern clone at [https://github.com/Robmaister/SimplerHidWrite](https://github.com/Robmaister/SimplerHidWrite).
- **Optional**: *SimHub* and the official plugin.
  No game is required. We are using pre-recorded telemetry data found in the
  [TestTelemetryData](../TelemetryIntegrationTest/TestTelemetryData/) folder.

## Procedure and expected output

### Primary test

*Note*: do not confuse `Set Report` with `Set Feature`.

1. Open "SimpleHidWriter.exe". Locate this test device in the top area, and click on it.
   Look for `Device VID= ... PID= ...`.
2. You should see continuous report lines starting with `RD 01`.
   Ignore them. Click on `Clear` from time to time.
3. Enter `1E` at field `ReportID`.
4. Enter `00 00 FF FF FF 00` at fields below `ReportID`.
5. Click on `Set Report`.
6. Enter `00 07 FF 00 00 00` at fields below `ReportID`.
7. Click on `Set Report`.
8. Enter `03` at field `ReportID`.
9. Enter `FF FF FF 07 FF FF` at fields below `ReportID`.
10. Click on `Set Feature`.
11. The LED strip must show the following pixels in order:
    white, off, off, off, off, off, off, blue.
12. Enter `FF FF FF 08 FF FF` at fields below `ReportID`.
13. Click on `Set Feature`.
14. All pixels must turn off.

### Secondary and **optional** test

**Important note**:
this test departs a lot from others because there is no way to predict
a "correct" behavior.

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
