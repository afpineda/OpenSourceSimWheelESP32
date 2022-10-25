# Integration test: OLED with message priority

## Purpose and summary

To test that messages are shown to the OLED (or hidden) in a coordinated way depending on its priority.

## Hardware setup

This test is designed for 128x64 pixels with a 132x64 display controller. When using another setup, the sketch must be tweaked.  OLED must be powered and wired to the corresponding `SCL` and `SCA` pins in the DevKit board.

Output through USB serial port at 115200 bauds, but not strictly required for testing purposes.

## Procedure and expected output

1. Reset and ignore output until `-- GO --` is shown. If `OLED not detected!!` is show, check the wiring and restart.
2. A "Welcome" message (splash screen) must show up for a second.
3. The following sequence must be shown at the OLED, in strict order. The sequence is repeated in a loop. Check that no other message is displayed.

| Header  | Message    | Other                     |
| ------- | ---------- | ------------------------- |
| Menu    | Option 1   |                           |
| Menu    | Option 2   |                           |
| Menu    | Option 3   |                           |
| (none)  | (none)     | Screen cleared            |
| Info    | 1234567890 | Sized to fit screen width |
| Info    | 123456     | Sized to fit screen width |
| (none)  | (none)     | Screen cleared            |
| Modal 1 | Value 1    | Flashing edge             |
| Modal 2 | Value 2    | Flashing edge             |
| Menu    | Option 3   |                           |
| Modal 2 | Value 4    | Flashing edge             |
| (none)  | (none)     | Screen cleared            |
| Battery | (icon)     | Shown for 6 seconds       |

4. Check that all messages are not flipped upside-down.