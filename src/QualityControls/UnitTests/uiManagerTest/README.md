# Unit test: user interface manager

## Purpose and summary

To test that:

1. A screen with lower priority does not overlap a screen with higher priority.
2. A screen with higher priority overlaps a screen with lower priority.
3. When a screen is hidden, the screen with next priority is displayed.
4. When a screen is hidden and an higher priority screen is being displayed, nothing happens.
5. Auto hide works.

## Hardware setup

Nothing required.
Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Reset. Ignore output from the operating system itself.
2. Output must match the following:
   
   ```
   --READY--
   --GO--
   #1
   -- show and then hide
   info
   ***CLS***
   menu
   ***CLS***
   comm
   ***CLS***
   #2
   -- increasing priority show
   comm
   menu
   info
   -- decreasing priority hide
   menu
   comm
   ***CLS***
   #3
   -- decreasing priority show
   info
   -- decreasing priority hide
   menu
   comm
   ***CLS***
   #4
   -- decreasing priority show
   info
   -- increasing priority hide
   ***CLS***
   #5
   -- increasing priority show with autohide
   comm
   menu
   info
   ***CLS***
   #6
   -- autohide non visible screen
   info
   ***CLS***
   -- END --
   ```