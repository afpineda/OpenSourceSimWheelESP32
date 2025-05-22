# Required skills and tools

## Software tools (and external dependencies)

Indispensable:

- [Arduino IDE](https://www.arduino.cc/en/software).
  Choose a stable release and follow instructions.
  This project has been successfully compiled with **version 2.3.6**.
- Official [Arduino-ESP32 support package](https://docs.espressif.com/projects/arduino-esp32/en/latest/getting_started.html).
  Follow [this link](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html) for installation instructions.
  This project has been successfully tested with **version 3.2.0**.
- Indispensable [Arduino libraries](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries) for this project.
  Install their dependencies as well.
  - [NimBLE-Arduino](https://www.arduino.cc/reference/en/libraries/nimble-arduino/).
    This project has been successfully tested with **version 2.3.0**.
    Any version prior to version 2.2.0 will not work.
- [Powershell](https://docs.microsoft.com/en-us/powershell/scripting/install/installing-powershell?view=powershell-7.2).
  Already installed in Windows.

> [!IMPORTANT]
> This project is not guaranteed to work with untested version dependencies.
> When troubleshooting, check this first.

May be needed:

- [ESP32 drivers](http://esp32.net/usb-uart/) on Windows.
  A tutorial: [Installing drivers for the ESP32](https://www.bromleysat.com/installing-drivers-for-the-esp32/).
  Depending on what DevKit board you choose, a different driver may be needed.
  Check all information available from the manufacturer.

Recommended:

- [DIY layout creator](https://bancika.github.io/diy-layout-creator/).
  [Alternate link](https://github.com/bancika/diy-layout-creator/releases).
- Any joystick test application able to display 128 buttons:
  - [Planet pointy's joystick test application](http://www.planetpointy.co.uk/joystick-test-application/) (for Windows 10).
    **Does not work in Windows 11**.
  - [pygame-joystick-test](https://github.com/denilsonsa/pygame-joystick-test). Requires Python.
  - [Multi-joystick tester](https://github.com/EDDiscovery/MultiJoyStickTest/releases/tag/Release_1_2_0).
    Works in Windows 11.
    Best choice for now, but be aware
    that this app may miss some rotation events from rotary encoders
    due to its own polling interval.

- [Visual Studio Code](https://code.visualstudio.com/), as an alternative to Arduino IDE.
  Requires [Arduino Community Edition for VSCode](https://marketplace.visualstudio.com/items?itemName=vscode-arduino.vscode-arduino-community).
  Installable from VSCode itself.
  **Note**: The official Arduino extension was deprecated.
- SimpleHIDWrite.exe: available at [http://janaxelson.com/hidpage.htm](http://janaxelson.com/hidpage.htm).
  There is a modern clone here: [benbaker76/SimpleHIDWrite](https://github.com/benbaker76/SimpleHIDWrite/tree/master/BIN).

## Hardware tools

Indispensable:

- [Soldering iron](https://en.wikipedia.org/wiki/Soldering_iron) for electronics
- Thin welding tin with flux
- Cutting pliers

Recommended:

- A ["third hand"](https://en.wikipedia.org/wiki/Helping_hand_(tool))
- A soldering stand
- A cheap [multimeter](https://en.wikipedia.org/wiki/Multimeter)
- A [protoboard](https://en.wikipedia.org/wiki/Breadboard)
- A kit of Dupond wires for protoboards

## Skills

There are lots of resources out in the Internet to learn those skills. Just use your favorite search engine.

Indispensable:

- Basic soldering for electronics.
- Compile, upload and run Arduino's *sketches* using *Arduino IDE*.
- Run powershell scripts.

Recommended:

- [Ohm's law](https://en.wikipedia.org/wiki/Ohm%27s_law).
- Basics of switches, resistors, diodes and transistors.
- Role of [pull-up](https://en.wikipedia.org/wiki/Pull-up_resistor) and pull-down resistors.
- Basics of C and C++ programming languages.
- [Digital circuits](https://en.wikipedia.org/wiki/Digital_electronics).
- [System on a chip](https://en.wikipedia.org/wiki/System_on_a_chip) development boards (Arduino-compatible).
