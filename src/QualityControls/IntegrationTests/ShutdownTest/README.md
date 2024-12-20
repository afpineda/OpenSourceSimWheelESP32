# Integration test: System shutdown

## Purpose and summary

Test that all display peripherals cut power when the system is shut down.

## Hardware setup

An LED strip is required:

- No level shifter (3.3V logic).
- Eight pixels.
- Pixel driver: WS2812 family.
- `Din` attached to the GPIO pin aliased as [TEST_D_OUT](../../../include/debugUtils.h)

Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Reset.
2. The LED strip must light up white.
3. Output in the serial monitor must show the following lines and nothing else:

   ```text
   -- READY --
   -- GO --
   ```

4. Type anything in the serial monitor and press `enter`.
5. The following text must appear:

   ```text
    Command received: shutting down
    Device 1: shutdown.
    Device 2: shutdown.
   ```

6. The LED strip must go out.
