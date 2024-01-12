# Unit test: Input hub

**NOTE**:: This is also an integration test for modules `inputHub` and `userSettings`.

## Purpose and summary

To test that:

- ALT buttons work as expected.
- Clutch paddles work the same as ALT buttons when set to that working mode.
- Working modes are selected in response to the proper input events.
- Bite point calibration works as expected.
- D-PAD input is properly reported.
- F1-style clutch position is properly computed in the corresponding working mode.
- When clutch working mode is set to "regular button":
  - Analog clutch position is properly translated into a button state.
  - Digital clutch state is properly reported as a button state.
- When clutch working mode is set to "individual axes":
  - Digital clutch state is properly translated into an axis position.
  - Analog clutch position is properly reported as an axis position.
- User-defined button map is applied.

## Hardware setup

Nothing required. This is an automated test.
Output through USB serial port at 115200 bauds. Connect before reset.

## Procedure and expected output

1. Reset. Ignore output from the operating system itself.
2. Output must match:

```text
-- READY --
-- GO --
- simulate ALT engagement with clutch paddles or buttons -
- simulate ALT button input when working mode is regular button -
- simulate POV operation (valid input) -
- simulate POV operation (invalid input) -
- simulate POV operation while ALT pushed -
- simulate POV operation in buttons mode -
- simulate cycle ALT working mode -
- simulate cycle clutch working mode -
- simulate cycle DPAD working mode -
- simulate explicit selection of clutch working mode -
- simulate non-mapped button combinations -
- simulate bite point calibration -
- simulate dual clutch operation -
- simulate analog clutch operation in buttons mode -
- simulate digital clutch operation in axis mode -
- simulate digital clutch operation in buttons mode -
- simulate analog clutch operation in axis mode -
- simulate repeated input without real change in inputs state -
- simulate input in user-defined buttons map -
-- END --
```
