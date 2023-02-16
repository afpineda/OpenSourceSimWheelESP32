# Pin-out plan #1 for hardware testing

DevKit board: ESP32-DevKit-C

Requirements:

| Subsystem            | Input pins    | output pins |
| -------------------- | ------------- | ----------- |
| KY-040 rotary        | 3             | 0           |
| ALPS rotary          | 3 pulled up   | 0           |
| Button matrix        | 3 pulled down | 2           |
| Battery monitor      | 1 ADC         | 1           |
| Analog clutch        | 2 ADC         | 0           |
| Multiplexed switches | 2 pulled up   | 3           |

Plan:

| **GPIO** | **Input**  | **Output** |     **Usage**     | **Notes**                              |
| -------- | ---------- | ---------- | :---------------: | -------------------------------------- |
| **36**   | OK         |            |     Pot 1 ADC     | input only (no internal pull resistor) |
| **39**   | OK         |            |     Pot 2 ADC     | input only (no internal pull resistor) |
| **34**   | OK         |            |     KY-040 SW     | input only (no internal pull resistor) |
| **35**   | OK         |            |    KY-040 CLK     | input only (no internal pull resistor) |
| **32**   | OK         | OK         |     KY-040 DT     |                                        |
| **33**   | OK         | OK         |    ALPS ENC A     |                                        |
| **25**   | OK         | OK         |    ALPS ENC B     |                                        |
| **26**   | OK         | OK         |  Matrix input 1   |                                        |
| **27**   | OK         | OK         |  Matrix input 2   |                                        |
| **14**   | OK         | OK         |  Matrix input 3   | outputs PWM signal at boot             |
| **12**   | OK         | OK         | Matrix selector 1 | boot fail if pulled high               |
| **13**   | OK         | OK         | Matrix selector 2 |                                        |
| **9**    | x          | x          |   **UNUSABLE**    | connected to the integrated SPI flash  |
| **10**   | x          | x          |   **UNUSABLE**    | connected to the integrated SPI flash  |
| **11**   | x          | x          |   **UNUSABLE**    | connected to the integrated SPI flash  |
| **6**    | x          | x          |   **UNUSABLE**    | connected to the integrated SPI flash  |
| **7**    | x          | x          |   **UNUSABLE**    | connected to the integrated SPI flash  |
| **8**    | x          | x          |   **UNUSABLE**    | connected to the integrated SPI flash  |
| **15**   | OK         | OK         |  BATT_READ (ADC)  | outputs PWM signal at boot             |
| **2**    | OK         | OK         |                   | connected to on-board LED              |
| **0**    | pulled up? | OK         |                   | outputs PWM signal at boot             |
| **4**    | OK         | OK         |   Mutex input 1   |                                        |
| **16**   | OK         | OK         |   Mutex input 2   |                                        |
| **17**   | OK         | OK         |    BATT_ENABLE    |                                        |
| **5**    | OK         | OK         | Mutex selector 1  | outputs PWM signal at boot             |
| **18**   | OK         | OK         | Mutex selector 2  |                                        |
| **19**   | OK         | OK         | Mutex selector 3  |                                        |
| **21**   | OK         | OK         |                   |                                        |
| **3**    | pulled up  | RX pin     |                   | HIGH at boot                           |
| **1**    | TX pin     | OK         |                   | debug output at boot                   |
| **22**   | OK         | OK         |                   |                                        |
| **23**   | OK         | OK         |                   |                                        |
