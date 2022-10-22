# Relative rotary encoder subsystem

**There is no circuit involved here**, just software and wiring.

## Purpose

The purpose of this subsystem is to provide one or more relative rotary encoders as inputs to the system. Since a funky switch is a kind of rotary encoder, this subsystem also applies to them. However, **only rotation is part of this subsystem**. Any built in push button is not part of this subsystem, refer to the [Switches subsystem](../Switches/Switches_en.md) for that.

## Wiring

Each rotary encoder requires two dedicated pins at the DevKit board, no matter if they are bare bone or KY-040 rotary encoders. Not counting any built in push button, this is the wiring of a rotary encoder:

- KY-040 type:
  
  - Pin labeled `Vcc` must be wired to `3V3` pin at the board.
  - Pin labeled `GND` must be wired to `GND` pin at the board.
  - Pin labeled `CLK` or `A`, must be wired to any input-capable GPIO pin.
  - Pin labeled `DT` or `B` must be wired to any input-capable GPIO pin.

- Bare bone and funky switch type:
  
  - There are two pins labeled `GND`. The one located between `A` (or `CLK`) and `B` (or `DT`) must be wired to `GND` pin at the board. The other one, sometimes labeled `SW GND`, is not related to this subsystem.
  - Pin labeled `CLK` or `A`, must be wired to any input-capable GPIO pin **with internal pull-up resistors**. Otherwise, a external pull-up resistor must be placed.
  - Pin labeled `DT` or `B` must be wired to any input-capable GPIO pin **with internal pull-up resistors**. Otherwise, a external pull-up resistor must be placed.
  - Most GPIOs can enable an internal pull-up resistor by software, but GPIOs 34, 35, 36 and 39 lack them. An external pull-up resistor must be placed at those pins as shown in the following image. Any resistor will suffice, but high impedance is recommended.

![External pullup resistors](./ExternalPullupEncoder.png)

## Signal encoding

Some encoders use ["incremental Encoder Quadrature Output Waveform"](https://www.allaboutcircuits.com/projects/how-to-use-a-rotary-encoder-in-a-mcu-based-project/). This is the case of KY-040. 
Others seems to use a different signal encoding. **This is the case of [ALPS RKJX](https://docs.rs-online.com/5b4c/0900766b8152c2e9.pdf) series of funky switches**. Such encoding is called "alterante encoding" in this project.
The firmware is ready to use both.

## Firmware customization

Customization takes place at file [CustomSetup.ino](../../../../src/Firmware/CustomSetup/CustomSetup.ino).
Edit the body of `simWheelSetup()` and place a call to `inputs::addRotaryEncoder()` for each rotary encoder as shown below.

- First parameter is the GPIO assigned to `CLK` or `A`.
- Second parameter is the GPIO assigned to `DT` or `B`.
- Third paramater must be set to `true` when "alternate encoding" is in place (optional parameter, defaults to `false`). Give a try to this parameter if your encoder does not work properly.

For example, let's say that a bare bone rotary encoder has `A` attached to GPIO 33 and `B` attached to GPIO 25:

```c
void simWheelSetup()
{
   ...
   inputs::addRotaryEncoder(GPIO_NUM_33,GPIO_NUM_25);
   ...
}
```

For example, let's say that an ALPS funky switch encoder has `A` attached to GPIO 33 and `B` attached to GPIO 25:

```c
void simWheelSetup()
{
   ...
   inputs::addRotaryEncoder(GPIO_NUM_33,GPIO_NUM_25,true);
   ...
}
```

`inputs::addRotaryEncoder()` returns the input number for clockwise rotation, being the next one the input number number for counter-clockwise rotation.