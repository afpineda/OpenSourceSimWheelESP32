# Quality controls (overview)

Starting with version 5.0.0, the
[ESP32S3-DevKitC V1.0](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitc-1.html)
is the main board for development and testing. However, all tests should work in a pure ESP32 board, too.

## Pin-out plan (ESP32S3)

| **GPIO** | **Usages**                                       |
| :------: | ------------------------------------------------ |
|    04    | Button matrix I2                                 |
|    05    | Button matrix I1                                 |
|    06    | Button matrix I0                                 |
|    07    | Button matrix S1                                 |
|    15    | Button matrix S0                                 |
|    16    | Multiplexer I1                                   |
|    17    | Multiplexer I2                                   |
|    18    | Power LATCH                                      |
|    08    | Primary I2C bus (SDA)                            |
|    03    | **Reserved (strap pin)**                         |
|    46    | **Reserved (strap pin)**                         |
|    09    | Primary I2C bus (SCL)                            |
|    10    | Rotary encoder SW, WAKE UP pin (power subsystem) |
|    11    | Rotary encoder DT                                |
|    12    | Rotary encoder CLK                               |
|    13    | Potentiometer ADC                                |
|    14    | Potentiometer ADC                                |
|   ---    | ---                                              |
|    19    | **Not usable (USB)**                             |
|    20    | **Not usable (USB)**                             |
|    21    | Multiplexer S0, Secondary SDA                    |
|    47    | Multiplexer S1, Secondary SCL                    |
|    48    | Multiplexer S2                                   |
|    45    | **Reserved (strap pin)**                         |
|    00    | Digital button (BOOT)                            |
|    35    | **Not usable (PSRAM)**                           |
|    36    | **Not usable (PSRAM)**                           |
|    37    | **Not usable (PSRAM)**                           |
|    38    | ALPS Encoder B                                   |
|    39    | Shift register LOAD, DOut                        |
|    40    | Shift register NEXT, Simple shift light pin      |
|    41    | Shift register SERIAL                            |
|    42    | BattEN (Battery monitor enable)                  |
|    02    | BattREAD (Battery monitor ADC)                   |
|    01    | ALPS Encoder A                                   |
|    RX    | **Not usable (serial port)**                     |
|    TX    | **Not usable (serial port)**                     |

**Note**: the onboard RGB led uses GPIO #38 in revision 1.1,
but GPIO #48 in revision 1.0.
In any case, there is a solder pad that needs to be in short-circuit
for the built-in RGB led to work.

![ESP32S3-DevKitC pin-out](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/_images/ESP32-S3_DevKitC-1_pinlayout_v1.1.jpg)

## Pin-out plan (ESP32)

| **GPIO** | **Usages**                                                  |
| :------: | ----------------------------------------------------------- |
|    36    | Potentiometer ADC, Shift register SERIAL                    |
|    39    | Potentiometer ADC,                                          |
|    34    | Rotary encoder SW, WAKE UP pin (power subsystem)            |
|    35    | Rotary encoder CLK                                          |
|    32    | Rotary encoder DT, Shift register LOAD                      |
|    33    | ALPS Encoder A, Shift register NEXT, Simple shift light pin |
|    25    | ALPS Encoder B                                              |
|    26    | Button matrix I0                                            |
|    27    | Button matrix I1                                            |
|    14    | Button matrix I2                                            |
|    12    | Button matrix S0                                            |
|    13    | Button matrix S1, DOut                                      |
|    9     | **Not usable (flash mem)**                                  |
|    10    | **Not usable (flash mem)**                                  |
|    11    | **Not usable (flash mem)**                                  |
|   ---    | ---                                                         |
|    6     | **Not usable (flash mem)**                                  |
|    7     | **Not usable (flash mem)**                                  |
|    8     | **Not usable (flash mem)**                                  |
|    15    | BattREAD (Battery monitor ADC)                              |
|    2     |                                                             |
|    0     | Digital button (BOOT)                                       |
|    4     | Multiplexer I0                                              |
|    16    | Multiplexer I1                                              |
|    17    | BattEN (Battery monitor enable)                             |
|    5     | Multiplexer S0                                              |
|    18    | Multiplexer S1                                              |
|    19    | Multiplexer S2                                              |
|    21    |                                                             |
|    3     | **Not usable (serial port)**                                |
|    1     | **Not usable (serial port)**                                |
|    22    |                                                             |
|    23    | Power LATCH                                                 |

![ESP32-DevKitC pin-out](https://docs.espressif.com/projects/esp-dev-kits/en/latest/_images/esp32_devkitC_v4_pinlayout.png)

## Integration tests for display hardware

These tests are placed in "UITest".
The test procedure is the same as for the
[telemetry integration test](./IntegrationTests/TelemetryIntegrationTest/README.md).
For that reason, there is no "README" file.
