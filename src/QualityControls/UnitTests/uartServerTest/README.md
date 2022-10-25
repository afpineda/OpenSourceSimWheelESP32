# Unit test: UART input processing

## Purpose and summary

To test that received data and AT commands (through UART) is properly parsed and processed

## Hardware setup

Nothing required.
Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Reset.
2. Output must match the following:
   
   ```
   --GO--
   **TEST #1
   **TEST #2
   **TEST #3
   **TEST #4
   **TEST #5
   **TEST #6
   **TEST #7
   **TEST #8
   **TEST #9
   **TEST #10
   **TEST #11
   **TEST #12
   **TEST #13
   **TEST #14
   **TEST #15
   **TEST #16
   **TEST #17
   **TEST #18
   **TEST #19
   **TEST #20
   **TEST #21
   **TEST #22
   **TEST #23
   **TEST #24
   **TEST #25
   **TEST #26
   **TEST #27
   **TEST #28
   --END (reset to restart) --
   ```