# Pin-out plan #2 for hardware testing

DevKit board: ESP32-DevKit-C

Requirements:

| Subsystem            | Pins                 |
| -------------------- | -------------------- |
| PISO shift registers | LOAD / INPUT / CLOCK |
| GPIO Expanders       | SDA / SCL            |

Plan:

| **GPIO** | **Input**  | **Output** |      **Usage**       | **Notes**                              |
| -------- | ---------- | ---------- | :------------------: | -------------------------------------- |
| **36**   | OK         |            |      PISO Input      | input only (no internal pull resistor) |
| **39**   | OK         |            |                      | input only (no internal pull resistor) |
| **34**   | OK         |            |                      | input only (no internal pull resistor) |
| **35**   | OK         |            |                      | input only (no internal pull resistor) |
| **32**   | OK         | OK         |                      |                                        |
| **33**   | OK         | OK         |      PISO Clock      |                                        |
| **25**   | OK         | OK         |                      |                                        |
| **26**   | OK         | OK         |                      |                                        |
| **27**   | OK         | OK         |                      |                                        |
| **14**   | OK         | OK         |                      | outputs PWM signal at boot             |
| **12**   | OK         | OK         |                      | boot fail if pulled high               |
| **13**   | OK         | OK         |                      |                                        |
| **9**    | x          | x          |     **UNUSABLE**     | connected to the integrated SPI flash  |
| **10**   | x          | x          |     **UNUSABLE**     | connected to the integrated SPI flash  |
| **11**   | x          | x          |     **UNUSABLE**     | connected to the integrated SPI flash  |
| **6**    | x          | x          |     **UNUSABLE**     | connected to the integrated SPI flash  |
| **7**    | x          | x          |     **UNUSABLE**     | connected to the integrated SPI flash  |
| **8**    | x          | x          |     **UNUSABLE**     | connected to the integrated SPI flash  |
| **15**   | OK         | OK         |                      | outputs PWM signal at boot             |
| **2**    | OK         | OK         |                      | connected to on-board LED              |
| **0**    | pulled up? | OK         |                      | outputs PWM signal at boot             |
| **4**    | OK         | OK         |                      |                                        |
| **16**   | OK         | OK         |                      |                                        |
| **17**   | OK         | OK         |      PISO Load       |                                        |
| **5**    | OK         | OK         |                      | outputs PWM signal at boot             |
| **18**   | OK         | OK         |                      |                                        |
| **19**   | OK         | OK         |                      |                                        |
| **21**   | OK         | OK         | GPIO Expanders (SDA) |                                        |
| **3**    | pulled up  | RX pin     |                      | HIGH at boot                           |
| **1**    | TX pin     | OK         |                      | debug output at boot                   |
| **22**   | OK         | OK         | GPIO Expanders (SCL) |                                        |
| **23**   | OK         | OK         |                      |                                        |
