# Know your ESP32 DevKit board

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

## ESP-WROOM-32 DevKit

The following article explains which pins can be used and how:
[https://randomnerdtutorials.com/esp32-pinout-reference-gpios/](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/). However, we can get more specific:

- **GPIO #36, #39, #34 and #35**:
  
  - Input only.
  - Not suitable for bare bone rotary encoders, unless external pull-up resistors are in place.
  - Not suitable for a button matrix, unless external pull-down resistors are in place.

- **GPIO #12**:
  
  - Not suitable for rotary encoders (any kind).
  - Not suitable for any pulled-up input. Boot will fail.

- **GPIO #6, #7, #8, #9, #10 and #11**
  
  - **UNUSABLE**. Do not attach anything to those pins.

- **GPIO #0**:
  
  - Despite being described as pulled-up, it seems to work in other modes.

- **GPIO #3**:
  
  - Despite being described as input-capable, it works in pull-up mode only.
  - Not suitable for a button matrix.
  - Perfect for rotary encoders (any kind).

- **GPIO #1**:
  
  - Output only.

## The problem with boot strapping

In order to upload a firmware to a DevKit board, certain pins, called "bootstrap" pins are used. If some device is attached to those pins, it may get in the way. You should detach the board from the circuit before uploading.

## Some interesting ESP32 boards suitable for this project

- [Tinypico](https://www.tinypico.com/) and [related](https://unexpectedmaker.com/shop?category=Development+Boards).
- [Adafruit Feather 32u4 Bluefruit LE](https://www.adafruit.com/product/2829).
- [Wemos D32 boards](https://www.wemos.cc/en/latest/d32/d32.html). Also known as "Lolin32".
- [Sparkfun Thing Plus](https://www.sparkfun.com/products/17381)
- Most [LilyGO](http://www.lilygo.cn/) DevKits.
- [Seeed Studio XIAO ESP32C3](https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html). Note: this is a single core SoC.
