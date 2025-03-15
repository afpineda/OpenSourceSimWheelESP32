# Unit test: user settings

## Purpose and summary

To test that user settings are properly stored in flash memory then retrieved at start up.

> [!WARNING]
> This test is destructive.
> Any previous user settings will be erased.

## Hardware setup

Nothing required. Serial output at 115200 bauds.

## Procedure and expected output

1. Reset. Ignore output from the operating system itself.
   The message `All settings should be loaded` and
   a text menu will show up.
   You may follow on-screen instructions.
   To select an option type a character in the Serial Monitor
   and hit `Enter`.
2. Select option `X`.
3. Reset.
4. Select option `1`.
5. **Before 20 seconds elapse**, reset.
6. Select option `3`
7. Output must match (this is **OK**):

    ```text
    Checking current settings against test case (delayed settings)
    NO MATCH: axis calibration data (0,0,0,0)
    NO MATCH: axis polarity
    NO MATCH: pulse width
    NO MATCH: Alt buttons working mode
    NO MATCH: Bite point (0)
    NO MATCH: Clutch working mode
    NO MATCH: DPAD working mode
    NO MATCH: security lock
    Done
    ```

8. Reset.
9. Select option `1` again.
10. **Wait for 20 seconds** until the message `Saved!` show up.
    Such a message must show up. Otherwise, the system does not
    pass this test.
11. Reset.
12. Select option `3`.
13. The message `NO MATCH ...` must **not** show up.
14. Reset.
15. Select option `2`.
16. The message  `Saved!` must show up inmediately.
17. Reset.
18. Select option `4`.
19. The message `NO MATCH ...` must **not** show up.
