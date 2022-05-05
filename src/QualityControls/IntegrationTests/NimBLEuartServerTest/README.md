# Unit test: AT command processing via UaS

## Purpose and summary

To test that AT commands are received and processed using NuS.

## Hardware setup

Nothing required, except for an external antenna on some devices.
Output through USB serial port at 115200 bauds. This will be called "serial output" from now on.

## Software setup

A mobile device or computer is required, equipped with an UaS-capable terminal app ("terminal app" for later reference). For example:

- _Android_: [Serial Bluetooth Terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal)

Output from the terminal app will be called "terminal output" from now on. Note that input text is case-sensitive.

## Procedure and expected output

1. Reset. Ignore serial output from the operating system itself.
2. Serial output must match:
   
```
--START--
*** DISCOVERING ***
--GO--
```

3. Enter the terminal app and connect to the device. Failing to do so will reset the device after a minute. In such a case, restart this test.
4. Serial output must match:
   
```
*** CONNECTED ***
```

5. Type `AT`, then press "send". Terminal output must match:
   
```

ERROR

```

6. Type `AT+C?;;+C?`, then press "send". Terminal output must match:

```

00

OK

00

OK

```

7. Type `AT+C=;+C=`, then press "send". Terminal output must match:

```

ERROR

ERROR

```

8. Type `AT+C=0x15;+C?`, then press "send". Terminal output must match:

```

OK

15

OK

```

9. Type `AT+C=16;+C?`, then press "send". Terminal output must match:

```

OK

10

OK

```

10. Type `AT+A=1;+A?;` then press "send". Terminal output must match:

```

OK

1

OK

```

11. Type `AT+R=Y` then press "send". Terminal output must match:

```

OK

```

12. Serial output must match:

```

Battery auto calibration restarted

```

13. Type `!5553101010203` then press "send". Wait for a second. Terminal output must match:


```

GEAR: 5 RPM%: 85 SPEED: 305 MAP: 1 ABS: 2 TC: 3

```

14. Type `!` then press "send". Nothing must happen.
15. Type `!n` then press "send". Wait for a second. Terminal output must match:


```

GEAR:  RPM%: 0 SPEED: 0 MAP: 0 ABS: 0 TC: 0

```

16. Type `!10A` then press "send". Wait for a second. Terminal output must match:


```

GEAR: 1 RPM%: 10 SPEED: 0 MAP: 0 ABS: 0 TC: 0

```

17.  Type `!2zz000A` then press "send". Wait for a second. Terminal output must match:


```

GEAR: 2 RPM%: 0 SPEED: 0 MAP: 0 ABS: 0 TC: 0
```