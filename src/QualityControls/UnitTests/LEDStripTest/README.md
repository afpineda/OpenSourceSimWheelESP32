# Unit test: LED strip

## Purpose and summary

To test that an LED strip is properly interfaced using a single GPIO pin.

## Hardware setup

A WS2812B 8 pixel LED strip is required without a "lever shifter".
Output via USB serial port at 115200 baud. Connect before reset.

## Procedure and expected output

1. Reset. Ignore output from the operating system itself.
2. Output must show:

   ```text
   -- GO --
   ```

3. All LEDs must go red. Output must show `Go red`.
4. All LEDs must go green. Output must show `Go green`.
5. All LEDs must go blue. Output must show `Go blue`.
6. All LEDs must go white. Output must show `Go white`.
7. All LEDs must go purple. Output must show `Go purple`.
8. All LEDs must go orange. Output must show `Go orange`.
9. All LEDs must go dimmer. Output must show `Go orange dimmer`.
10. All LEDs must go off. Output must show `Go off`.
11. LEDs must show rainbow colors. Output must show `rainbow`.
12. LEDs must show rainbow colors displaced one pixel to the right. Output must show `Shift to next`.
13. LEDs must show rainbow colors again. Output must show `Shift to previous`.
14. Back to 3.
