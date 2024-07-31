# Know your ESP32 DevKit board

You may choose any devkit board you want as long as it is based on Expressif's "ESP32" architecture and features BLE support.
To be more precise, "ESP32" or "ESP32S3" boards, since they are supported by [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino).
Expressif's "ESP32C3" boards are not recommended since they have a single CPU core.
They work, but this project takes advantage of dual core architectures.

Alternatively, you may use any "ESP32S3" board for a purely wired implementation, since those are *USB-capable*.
"USB-capable" means your devkit is able to work as a fully-featured USB device.
Note that most USB ports found in devkit boards are just serial port devices, not fully featured ones, so check a data sheet before purchasing.
Right now, this project does not support both USB and BLE connectivity at the same time in the same device.

Pure "ESP32" boards are widely available and 100% functional, however, they are a bit outdated and may drop support soon.
My advice is to go with the latest boards. Some interesting DevKit boards for this project are:

- Those designed and sold by [Unexpected Maker](https://unexpectedmaker.com/shop?category=Development+Boards),
  like "Feather S3", "Tiny S3" or "TinyPico". Those are open hardware, as well.
- [Adafruit Feather 32u4 Bluefruit LE](https://www.adafruit.com/product/2829).
- [Wemos boards](https://www.wemos.cc/). *Note*:
  - The "D32" series are also known as "Lolin32" and found by that name at many retailers.
  - Wemos "Lolin32 Lite" is an outdated and deprecated board. However, very cheap clones are still sold at some retailers.
  - "S3 mini" is a very interesting board if you don't need built-in battery support.
- [Sparkfun Thing Plus](https://www.sparkfun.com/products/17381).
- [Some LilyGO boards](http://www.lilygo.cc/) even if their built-in display is not used.
  - "T7 S3" is a very interesting board, close to "Wemos S3 mini", but features built-in battery support
    (credits to user [@WallK](https://github.com/WallK) for pointing this out).

You need to balance two key aspects: **size and pin availability**. The larger the board, the higher the count of available pins, so you can fit more buttons, paddles, etc, but the greater is the space required inside the sim wheel's housing.

In order to reduce size and circuit complexity, it is recommended to use GPIO expanders (will be explained later) and small devkit boards like Unexpected Maker's "TinyPico" or LilyGO's "T-QT".

This project makes extensive use of the official "ESP-WROOM-32" board (aka "ESP32-DevKit-C") for testing and development purposes, but this is not the best choice due to its excessive size.

## Flash memory requirements

At the time of writing, this project uses 85%-90% of the storage capacity of a "standard" ESP32 DevKit, which is **4 MB** of flash memory.
This is very close to the limit and firmware size may continue to grow.
In case of extreme need, you may still reconfigure the partition table to make more room for the firmware:

- In Arduino IDE, go to "tools" ("Arduino: board configuration" in VSCode), then "partition scheme".
- Select "Huge APP (3MB No OTA/1MB SPIFFS)".

However, my advice is to go for a 8 MB DevKit board, just in case.

## The two lies

Technically, DevKit boards are interfaced through General Purpose Input/Output (GPIO) pins. However, there are two lies in this sentence:

- General Purpose: Some  pins are *specific purpose*, to name some: I2C and UART pins. Some of those pins can not be used as general-purpose pins.
- Input/Output: some pins can not be used for input, others can not be used for output.

This is not the only annoyance:

- Boot will fail if certain pins are set to high voltage.
- Some input pins lack internal pull-up or pull-down resistors that others have.
- Some pins can be used as inputs, but in pull-up mode only.
- Not all pins can wake up the system from deep sleep.
- Not all pins can be used for analog input.

As a result, some devices can not be connected to any arbitrary pin, so a **pin-out plan** is needed. To develop such a plan, you need to know which constraints apply to your DevKit board. Look for a data sheet.

### ESP-WROOM-32 DevKit

The following article explains which pins can be used and how:
[https://randomnerdtutorials.com/esp32-pinout-reference-gpios/](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/). However, we can get more specific:

- **GPIO #36, #39, #34 and #35**:

  - Input only.
  - Not suitable for bare bone rotary encoders, unless external pull-up resistors are in place.
  - Not suitable for a button matrix, unless external pull-down resistors are in place (inputs only).
  - Not suitable for an analog multiplexer, unless external pull-up resistors are in place (inputs only).

- **GPIO #12**:

  - Not suitable for rotary encoders (any kind).
  - Not suitable for any pulled-up input. Boot will fail.

- **GPIO #6, #7, #8, #9, #10 and #11**

  - **UNUSABLE**. Do not attach anything to those pins.

- **GPIO #0**:

  - Despite being described as pulled-up, it seems to work in other modes.
  - The board will enter "bootloader mode" if this pin is set to low voltage at startup.

- **GPIO #3**:

  - Despite being described as input-capable, it works in pull-up mode only.
  - Not suitable for a button matrix.
  - Perfect for rotary encoders (any kind).

- **GPIO #1**:

  - Output only.

## Uploading firmware to your DevKit board

In order to upload a firmware to a DevKit board, certain pins, called "bootstrap" pins are used. If some device is attached to those pins, it may get in the way. You should detach the board from the circuit before uploading, just in case.

In Arduino IDE, you need to configure the "board manager" with the proper parameters. Most times, you only need to select the correct board and go with the default parameters. Most times, those boards are "ESP32 Dev Module" or "ESP32S3 Dev Module". However, check the manufacturer's data sheet.
When using the USB implementation, set USB-Mode to "USB-OTG (TinyUSB)".

### Entering bootloader mode

Some DevKit boards refuses to upload firmware unless you put them in "bootloader mode".
To enter boot mode you have to either:

- press the "reset" button while pressing the "boot" button, then release both.
- keep the "boot" button pressed while Arduino IDE is uploading the firmware.

If your DevKit board does not feature a "boot" button, drive `GPIO #0` to `GND` instead.

If your DevKit board does not feature a "reset" button, drive `EN` to `GND` instead.

It is said that some boards require a small capacitor between `EN` and `GND` pins to avoid this annoyance.
I can not say if that is true.

See [Expressif's documentation on "Select Bootloader Mode"](https://docs.espressif.com/projects/esptool/en/latest/esp32/advanced-topics/boot-mode-selection.html)
for further information.

### Troubleshooting firmware bugs

If you go for a custom firmware and it does not work,
your only chance to know what is happening is the *serial monitor* at Arduino IDE.

However, most debug messages are **not shown by default**.
In order to enable them, go to the "board manager" and find the "Core debug level" option.
Configure that option to "error" or "verbose". Then, upload the firmware again.

Check the guide on [firmware-defined error messages](./../firmware/ErrorMessages_en.md).
