# Unit test: connectivity

## Purpose and summary

To test that every connectivity option behaves as expected
regarding advertising, connection and disconnection events.

## Hardware setup

- **A DevKit board with two USB ports is required**
  (USB-TO-UART and USB-OTG). ESP32S3-DevKit-C was tested.
- Two USB cables to the host computer.

Output through USB serial port at 115200 bauds using the USB-TO-UART port.

## Arduino IDE setup

Press the `reset` button while holding the `Boot` (or `IO0`) button
to enter *bootloader mode*.

- *Upload mode*: UART0 / Hardware CDC.
- *USB mode*: Hardware CDC and JTAG.
- *USB CDC On Boot*: disabled.

## Software setup

Computer:

- Windows 10 or later
- Bluetooth 4.2 or later

## Procedure and expected output

This procedure must be executed four times by selecting
a different connectivity option on each run.
The device is named "ConnTest".

The sketch waits for user input in the serial monitor before running.
Type "?" to print all the options.
Type a character to select a connectivity option.

Use the Windows "Bluetooth & devices" control panel to check
the connection status. From now on, all references to the "USB cable"
refers to the USB-OTG port.

The test procedure starts once the `--GO--` message is printed.

### Before each run

- Unplug the USB cable from the USB-OTG port.
- If the device is paired because of a previous test,
  unpair it first (delete from the Bluetooth control panel).
- Hit `reset`.

### BLE-only connectivity

1. The message `*** DISCONNECTED ***` must appear.
2. Check that the device is advertised
3. Connect.
4. The message `*** CONNECTED ***` must appear.
5. Plug the USB cable.
6. Check the BLE connection is alive and there is no USB connection.
7. Unpair the BLE device (disconnect).
8. The message `*** DISCONNECTED ***` must appear.
9. Check that the device is advertised again.

### USB-only connectivity

1. The message `*** DISCONNECTED ***` must appear.
2. Check that the device is **not** advertised.
3. Plug the USB cable.
4. The message `*** CONNECTED ***` must appear.
5. Check there is USB HID connection in the host computer.
6. Unplug the USB cable.
7. The message `*** DISCONNECTED ***` must appear.

### USB & BLE

Part 1:

1. The message `*** DISCONNECTED ***` must appear.
2. Check that the BLE device is advertised, but do not connect yet.
3. Plug the USB cable.
4. The message `*** CONNECTED ***` must appear.
5. Check that the device is still advertised and connect.
6. No message must appear.
7. Check that both BLE and USB devices are connected
   (in the host computer).
8. Unplug the USB cable.
9. No message must appear.
10. Check the BLE connection is alive and there is no USB connection.
11. Unpair the BLE device (disconnect).
12. The message `*** DISCONNECTED ***` must appear.

Part 2:

1. Check that the BLE device is advertised and connect again.
2. The message `*** CONNECTED ***` must appear.
3. Plug the USB cable.
4. No message must appear.
5. Check that both BLE and USB devices are connected
   (in the host computer).
6. Unpair the BLE device (disconnect).
7. No message must appear.
8. Check the USB connection is alive.
9. Check that the device is still advertised.
10. Unplug the USB cable.
11. The message `*** DISCONNECTED ***` must appear.

### USB & BLE (EXCLUSIVE)

1. The message `*** DISCONNECTED ***` must appear.
2. Check that the BLE device is advertised, but do not connect yet.
3. Plug the USB cable.
4. The message `*** CONNECTED ***` must appear.
5. Check that the BLE device is **not** advertised.
6. Unplug the USB cable.
7. The message `*** DISCONNECTED ***` must appear.
8. Check that the BLE device is advertised again and connect.
9. The message `*** CONNECTED ***` must appear.
10. Plug the USB cable.
11. No message must appear.
12. Check that the BLE device is **not** connected.
13. Unplug the USB cable.
14. The message `*** DISCONNECTED ***` must appear followed
    by the `*** CONNECTED ***` message.
15. Check that the BLE device is connected.
16. Unpair the BLE device (disconnect).
17. The message `*** DISCONNECTED ***` must appear.
