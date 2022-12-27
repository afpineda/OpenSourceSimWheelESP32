# Unit test: clutch beaviour

## Purpouse and summary

To test correct behaviour of clutch paddles depending on each available function.

## Harware setup

Nothing required. This is an automated test.
Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Reset. Ignore output from the operating system itself.
2. Output must match:

   ```text
   -- READY --
   -- GO --
   -- Set bite point
   -- CF_BUTTON
   -- CF_ALT
   -- CF_ALT: left on, right off
   -- CF_ALT: left off, right on
   -- CF_ALT: left on, right on
   -- CF_ALT: left off, right off
   -- CF_AXIS
   -- CF_AXIS: new bite point
   -- CF_CLUTCH
   -- CF_CLUTCH: left off, right off
   -- CF_CLUTCH: left on, right off
   -- CF_CLUTCH: left on, right off, new bite point
   -- CF_CLUTCH: left off, right on
   -- CF_CLUTCH: left off, right on, new bite point
   -- CF_CLUTCH: left on, right on, new bite point
   -- END --
   ```
