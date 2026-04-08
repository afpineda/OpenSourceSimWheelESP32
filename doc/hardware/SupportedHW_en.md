# Supported hardware quick reference

This is a summary of supported hardware.
You will find details later.

## Supported DevKits

Any DevKit based on the ESP32 architecture having BLE and/or USB-OTG.

## Supported input devices

- Switches:
  - Normally open or NO-NC.
  - Momentary or non-momentary.
  - Any shape (push button, roller lever, etc.)
- Discrete Hall-Effect sensors (working as a switch)
- Rotary encoders:
  - Bare-bone or KY-040
    (any brand using *Incremental Encoder Quadrature Output Waveform*).
  - 7-way funky switches from ALPS.
- Rotary switches (any brand).
- Rotary coded switches
  (any brand using "BCD" output code, complementary or not).
- Potentiometers.
- **Unipolar** ratiometric linear HE sensors
  (working as a potentiometer).
- 4-way directional pads or sticks.

> [!NOTE]
> Unlisted input devices should work if they behave as another supported device.

### Supported input circuitry for switches

- Button matrices:
  - Any size.
  - Diodes are mandatory.
- Multiplexers:
  - Any brand.
  - Analog or digital,
    but digital multiplexers are discouraged.
  - 8, 16 or 32 channels.
- PISO shift registers:
  - 74HC165N brand only (or clones).
- GPIO expanders:
  - PCF8574 (8 channels) or MCP23017 (16 channel) brands only.

> [!NOTE]
> Analog circuits are **not** supported,
> including voltage ladders and voltage dividers.

## Supported telemetry displays "out-of-the-box"

- Single-color LEDs:
  - Any forward voltage (any color),
    but depending on the available power supply.
- RGB LED strips:
  - Having a pixel driver in these families:
    WS2811, WS2812, WS2815, SK6812, UCS1903 or APA106.
  - Any power voltage.
- Monochrome OLED displays:
  - I2C interface only.
  - Having an SSD1306, SH1106 or SH1107 display controller.
  - Having 128x64 pixels (only).
    Note that 132x64 display controllers are supported,
    but having a 128x64 display panel.

> [!NOTE]
> The firmware is extensible to other telemetry displays,
> but requires custom code.

### Supported single-color LED drivers "out-of-the-box"

- Single LED or LED cluster attached to a single GPIO pin
  (in open drain mode)
- PCF8574 (8 LEDs).

## Supported batteries

- Any battery pack with a voltage of 3.2V to 4.3V,
  which is called "1S".
  This is, if there are two or more battery cells,
  they are connected in parallel, not in series.

### Supported state-of-charge monitors

- Voltage divider with or without a NPN-PNP pair
  (to enable/disable the circuit).
- Any fuel gauge based on the MAX1704x family,
  including *Sparkfun TOL-20680*, *Adafruit MAX17048* or clones.
  - Only one battery cell is supported.
  - Only Li-Ion and LiPo chemistries are supported.
