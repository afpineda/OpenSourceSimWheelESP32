# Required skills and tools

## Software tools

Indispensable:

- [Arduino IDE](https://www.arduino.cc/en/software).
  Choose a stable release and follow instructions.
- Official [Arduino-ESP32 support package](https://docs.espressif.com/projects/arduino-esp32/en/latest/getting_started.html).
  Follow [this link](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html) for installation instructions.
  This project has been successfully tested with **version 3.0.1**, however, BLE implementation does not work (use *NimBLE* instead).
- Indispensable [Arduino libraries](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries) for this project.
  Install their dependencies as well.
  - [NimBLE-Arduino](https://www.arduino.cc/reference/en/libraries/nimble-arduino/).
    This project has been successfully tested with **versions 1.4.1 and 1.4.2**.
- [Powershell](https://docs.microsoft.com/en-us/powershell/scripting/install/installing-powershell?view=powershell-7.2).
  Already installed in Windows.

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
    Best choice for now.
- [Visual Studio Code](https://code.visualstudio.com/), as an alternative to Arduino IDE.
  Requires [Arduino extension for VSCode](https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino).
  Installable from VSCode itself.
  **Note**: That extension is officially deprecated, but it works.
- SimpleHIDWrite.exe: available at [http://janaxelson.com/hidpage.htm](http://janaxelson.com/hidpage.htm).

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
