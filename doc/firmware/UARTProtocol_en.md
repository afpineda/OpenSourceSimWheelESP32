# Bluetooth UART client software

Your hosting device (PC, phone, etc.) must be connected to the system in order to send/receive data. Windows also requires the system to be paired. These are some free apps:

- **Android**: 
  
  - [nRF Connect for Mobile by Nordic Semiconductors](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp&gl=US)
  - [Serial bluetooth terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal)

- **Apple**: 
  
  - [nRF Connect for Mobile by Nordic Semiconductors](https://apps.apple.com/us/app/nrf-connect-for-mobile/id1054362403)

- **Windows**:
  
  - [Bluetooth LE Explorer](https://www.microsoft.com/en-us/p/bluetooth-le-explorer/9n0ztkf1qd98?msclkid=3e571cc3c48711ec86a9bd20456650f0&activetab=pivot:overviewtab)

# UART protocol

The UART protocol allows any application in the host device to send and receive simple lines of text to the system. Each text line must be terminated with a newline character. All strings are case-sensitive.

## Available AT COMMANDS

| Command       | Format of parameter or response | Action                                              |
| ------------- | ------------------------------- | --------------------------------------------------- |
| AT+A?         | 0, 1 or 2                       | Get current function of clutch paddles (see below)  |
| AT+A=`number` | 0, 1 or 2                       | Set function of clutch paddles (see below)          |
| AT+B?         | "Y" or "N" (without quotes)     | Check if ALT buttons are mapped to the ALT function |
| AT+B=Y        | Upper case                      | Use ALT buttons for ALT function                    |
| AT+B=N        | Upper case                      | Use ALT buttons as regular buttons                  |
| AT+C?         | signed short hex number         | Get current clutch bite point                       |
| AT+C=`number` | signed short hex number         | Set current clutch bite point                       |
| AT+R=Y        | Upper case                      | Restart battery auto-calibration                    |

Clutch functions are:

- 0 --> F1-style clutch
- 1 --> Alternate mode
- 2 --> Regular buttons

Bite point is a signed byte number in the range -127 to 127 where -127 means "released" and 127 means "full engaged".

The host computer should read responses to AT commands and act accordingly.
See [AT command Syntax and responses](https://www.developershome.com/sms/atCommandsIntro2.asp).

# Send and display car simulation data

Each frame of simulation data may be written into a single line with this format (not an AT command):

> !`<Gear>` `<RPMpercent>` `<Speed>` `<engineMap>` `<absLevel>` `<TCLevel>`

Where `<Gear>` is any displayable single character. 
Other fields must be written as **hexadecimal digits** in **upper case** (do not prefix with "0x"):

- `<Speed>` must be written as **four** hexadecimal digits for a 2-bytes unsigned integer starting by the least significant byte (2 digits), left to right.
- Other fields must be written as **two** hexadecimal digits for a single byte unsigned integer.

`<RPMpercent>` is expected to express a percentage in the range 0-100 (decimal).
If `<Gear>` is a **blank space**, no data will be displayed. 

For example:

> !N552D01050403

means:

- `<Gear>` = 'N'
- `<RPMpercent>` = 55 (hex) = 85 (dec)
- `<Speed>` = 2D01 (word-aligned hex) = 012D (hex number) = 301 (dec)
- `<engineMap>` = 05 (hex) = 5 (dec)
- `<absLevel>` = 04 (hex) = 4 (dec)
- `<TCLevel>` = 03 (hex) = 3 (dec)

Fields can be omitted starting from the right side. Ill-formed messages will be ignored.
