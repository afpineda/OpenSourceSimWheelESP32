# Unit test: deep sleep and wake up

## Purpose and summary

To test deep sleep and wake up.

## Hardware setup

Use this [test circuit](../../../Protoboards/MainTestBoard.diy):

![Test circuit image](../../../Protoboards/MainTestBoard.png)

However, you have to run this test using two configurations:

- The first one will use a single button as wake-up source.
  In particular, the built-in push button found in the KY-040
  rotary encoder. Do **not** wire `WAKE_UP`.
- The second one will use any expanded GPIO pin as wake-up source.
  You should check that both GPIO expander chips work.
  Do not forget to wire `WAKE_UP`.

All of them work in negative logic.

Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Reset. Ignore all output.
2. Output must show:

   ```text
   Entering deep sleep mode
   ```

3. Wait a few seconds. **No other output** must appear.
4. Drive the wake-up source to low voltage (or press the button).
5. Output must match:

   ```text
   --START--
   Wake up caused by external signal using RTC_CNTL
   ```

6. When using GPIO expanders,
   the previous output could be repeated twice.
   This is OK.
