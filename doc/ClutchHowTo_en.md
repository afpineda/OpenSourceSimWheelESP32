# How the clutch paddles work

There are a number of operating modes for the clutch paddles in this firmware.
There are several ways to select a particular mode:

- Using the companion app.
- Using the SimHub plugin.
- By pressing a specific combination of buttons.

This document explains the specifics of each mode.

"Digital" clutch paddles work in the same way as "analog" clutch paddles.
However, "digital" clutch paddles are fully depressed or fully released
and there is no intermediate state.

When launch control or F1-style clutch mode is selected,
the bite point calibration is performed as follows:

- Using the companion app or the SimHub plugin.
- By fully depressing one clutch paddle,
  fully releasing the other
  and operating the bite point calibration input
  (typically a rotary encoder).

If the "Alternate Mode" or "Buttons Mode" is selected,
the clutch paddle must be pressed down 3/4 of the way
for the input to be detected as engaged.
On the other hand, the clutch paddle must be released 1/4 of the way up
for the input to be detected as disengaged.

## F1-Style clutch

In this mode the position of both clutch paddles is combined into a single clutch position.
There is no *master* or *slave* paddle.
The paddle that is pressed hardest will move the clutch position up to the bite point
and the other paddle will add up to the full clutch position.

### How to take the race start (F1-style)

1. Depress both clutch paddles.
2. Use the accelerator to achieve the optimum engine speed.
3. Release one paddle when the lights go out.
4. When the car is moving at the optimum speed, release the other paddle.

## Launch control

In this mode the user designates one of the clutch paddles
as the *master* and the other as the *slave*.

The *slave* clutch paddle will move the clutch position
either to the bite point or fully disengaged,
but no intermediate position will be reached.
The *master* clutch paddle will move the clutch position
to any position from fully disengaged to fully engaged,
ignoring the bite point.
However, when the slave paddle is fully depressed,
the master paddle will move the clutch
to any position from the bite point to fully engaged.

### Bite point calibration

For bite point calibration to work in this mode (using the hardware inputs),
you must fully depress the slave paddle and release the master paddle.
It doesn't work the other way round.

### How to take the race start (launch control)

1. Depress both clutch paddles.
2. Use the accelerator to achieve the optimum engine speed.
3. Release the **master** paddle when the lights go out.
4. When the car is moving at the optimum speed,
   release the slave paddle.

## Alternate mode

In this mode, both clutch paddles will work as "ALT" buttons (mode selectors).
Use the "alternate mode" when no "ALT buttons" are available
and the clutch is not needed (automatic clutch).

## Buttons mode

In this mode, both clutch paddles will work as regular buttons.
Use the "buttons mode" when the clutch is not needed (automatic clutch).
