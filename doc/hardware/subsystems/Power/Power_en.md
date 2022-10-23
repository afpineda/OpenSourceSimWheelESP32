# Power subsystem

**You don't build this subsystem**, just buy one.

## Purpose

The purpose of this subsystem is:

- To provide a regulated power source of 3.3V to the rest of the system, no matter if it is powered through cable, batteries or both.
- If the system is powered through batteries:
  - To charge batteries while the USB cable is plugged in, even if the system is in use.
  - To prevent over charge of the batteries.
  - To prevent over discharge of the batteries.
  - To **prevent your batteries from catching fire or blowing up**.
  - To maximize battery life.

There are **two alternate choices** for this subsystem and **restrictions** to them:

- Use the internal voltage regulator at the DevKit board, so power is taken from an external source.
- Use a _powerboost module/shield_ (also known as _power bank_). This is **mandatory** for a battery-operated system.

**Warning:** Do not place two or more power sources at the same time. The DevKit board will get damaged.

## Power from an external source

An external power source must be available through a micro-USB connector or Dupond wires (both are supported). However, the external power source must meet one of these requirements:

- Steady and constant voltage (`Vcc`) between 3.0V and 3.3V (but no more), and a current supply of 500mA or more. In this case, the `3V3` pin at the DevKit board is wired to `Vcc` and `GND` as usual, just the same as a powerboost module.
- Any voltage (`Vcc`) between 5V and 12V, steady or not, and a current supply of 500mA or more. There are two wiring options:
  - Micro-USB cable: just plug in.
  - `Vcc` wired to `5V0` pin at the DevKit board and `GND` as usual. Note that despite the "5V0" label, this pin can hold up to 12V. However, the higher the voltage, the higher the waste of power and the higher the heat dissipated. No more than 7V is recommended.

If your external power source does not meet one of these requirements, go for a powerboost module even without batteries, or find a suitable external voltage regulator.

## Power through a powerboost module

### Buying guide

Ensure you buy nothing but a _powerboost module/shield_ or a _power bank_. A simple battery charger or any other circuit **is dangerous** even if it works. The requirements are:

- Regulated power source (steady voltage)
- Current supply of 500mA or more (1A is recommended). 
- Capable of 3.0V to 3.3V output voltage. Some powerboost modules provide both 5.0V and 3.3V. 
  - If your powerboost module is 5.0V-capable, but not 3.3V-capable, you can still use it. Plug the 5.0V output to the 5V0 pin at the DevKit board. This works, but it is **a waste of power** (there are two voltage regulators in action), thus not recommended.
- Battery pack with a voltage of 3.2V to 4.3V, which is called "1S". This is, if there are two or more battery cells, they are connected in parallel, not in series.

It is highly recommended, but not mandatory, for the powerboost module to come equipped with a [power latch circuit](../PowerLatch/PowerLatch_en.md). You now there is a power latch circuit if there is a power on/off _momentary push button_, but not a (non-momentary) switch.

Check which kind of battery is suitable for the chosen powerboost module and buy one. 
Keep in mind how much space is available inside the wheel case to fit all the components, so don't buy an oversize powerboost module.

Some ESP32-based DevKits already integrate a powerboost module, such as the [Adafruit HUZZAH32 – ESP32 Feather Board](https://www.adafruit.com/product/3405). Adafruit also offers two stand-alone powerboost modules (a bit overpriced, in my opinion), but not 3.3V-capable.

This is [an example](https://www.wareorigin.com/products/8650-v8-lithium-battery-shield-micro-usb-5v-3a-3v-1a-power-bank-battery-charging-module-for-arduino-esp32-esp8266-wifi) of a popular 3.3V-capable power bank that can be purchased from many suppliers.
![3V3 LiPo powerboost](https://cdn.shopify.com/s/files/1/0258/7145/0157/products/1_d200f2a8-5564-4df9-a8de-b79283da38bd_1024x.jpg?v=1568552186)

And this is the powerboost module used for testing in this project (note there is a latch circuit):
![module used for testing](./BatteryShieldSpecs.png)

## Circuit design

There is no circuit involved, however, some wires have to be soldered. If possible, use pin headers instead, for flexibility. These labels will be used from now on along this project:

- **POWERBOOST_3V3**: named as `3V3` or `3V` at the powerboost module.
- **POWERBOOST_GND**: named `GND`.
- **EXTERNAL_5V**: named as `5V` or `5V0` at the powerboost module.
- **EXTERNAL_GND**: named `GND`.
- **Battery(+)**: the positive pole of the battery pack. Just solder a wire to any of them. It is required by the [battery monitor subsystem](./../BatteryMonitor/BatteryMonitor_en.md).

## Firmware customization

Nothing is required.

## Further reading

For an explanation on how how to power the DevKit board, take a look at these articles:

- [How to power your ESP32 dev kit](https://techexplorations.com/guides/esp32/begin/power/)
- [What is the best battery for the ESP32?](https://diyi0t.com/best-battery-for-esp32/)
- [Power Supply for ESP32 with Battery Charger & Boost Converter](https://how2electronics.com/power-supply-for-esp32-with-boost-converter-battery-charger/)