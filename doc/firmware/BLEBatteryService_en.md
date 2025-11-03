# Bluetooth battery service

This is a compilation of relevant information about the
*battery service specification* for Bluetooth Low Energy.
That specification comes from the following documents:

- [Battery Service 1.1](https://www.bluetooth.com/specifications/specs/bas-1-1/)
- [GATT Specification supplement](https://www.bluetooth.com/specifications/gss/)
  - 3.23 Battery Critical Status (page 28)
  - 3.27 Battery Information (page 32)
  - 3.28 Battery Level (page 35)
  - 3.29 Battery Level Status (page 35)
- [Assigned Numbers](https://www.bluetooth.com/specifications/assigned-numbers/)
- [Bluetooth Core Specification](https://www.bluetooth.com/specifications/specs/core60-html/)
- [GATT Namespace descriptors](https://github.com/dano-liu/bluetooth-spec/blob/main/GATT%20Namespace%20Descriptors.pdf)

The purpose is to enable an implementation.

Other known implementations:

- [Zephir](https://docs.zephyrproject.org/apidoc/latest/group__bt__bas.html)
- [Nordic SDK](https://developer.nordicsemi.com/nRF5_SDK/nRF51_SDK_v4.x.x/doc/html/group__ble__sdk__srv__bas.html)
- [Microcontrollerslab.com](https://microcontrollerslab.com/esp32-ble-server-gatt-service-battery-level-indication/)
- [Circuitdigest](https://circuitdigest.com/microcontroller-projects/esp32-ble-server-how-to-use-gatt-services-for-battery-level-indication)

## Conventions

Data structures are described starting with the least significant byte.
Bit fields are described starting with the least significant bit.

## Related Assigned Numbers (UUID)

Service / Characteristic / Descriptor:

- Battery Service: `0x180F`.
  - Battery Level: `0x2A19`
    - Characteristic presentation format: `0x2904`
  - Battery Level Status: `0x2BED`
  - Battery Critical Status: `0x2BE9`
  - Battery Information: `0x2BEC`

## Available features

### Two or more batteries

«If a device has several batteries to be monitored,
then multiple instances of the Battery Service should be declared.»

Two device models are supported:

- Devices implementing several GATT servers, one battery service each.
- Devices implementing a single GATT server with several battery services.
  **This is the use case covered here**.

#### Aggregation groups

«A Battery Aggregation Group represents a collection
of one or more batteries that provide power
to a device concurrently and in parallel.»

A device has one or more *aggregation groups* (one by default).
Each *aggregation group* can contain one or more *battery services*.

An application chooses to aggregate or not the
individual information form each individual battery service in the same group.
There are no other implications.

### Removable batteries

«It is possible that the batteries are removable.»

## Characteristics

### Battery Level

Mandatory. Read. Notify.

«This characteristic is used to describe the level of the battery
represented by this instance of the Battery Service.
...
The value of the battery level is represented as a percentage from 0 to 100,
where 0 percent represents a battery that is fully discharged,
and 100 percent represents a battery that is fully charged.
...
If the battery is not present,
then the value of the Battery Level characteristic should be set to zero.»

#### Characteristic Presentation Format descriptor

Mandatory if there are two or more battery services.

«When a device has more than one instance of the Battery Service,
each Battery Level characteristic **shall** include a
Characteristic Presentation Format descriptor
...
and the Description field set to a valid value from the GATT Namespace Descriptors
and that is **unique** among all instances of the Battery Service exposed by the GATT Server.
»

Read only, no authentication, no authorization.

| Field       | Byte | Value                |
| ----------- | ---- | -------------------- |
| Format      | 1    | 0x04 (uint8)         |
| Exponent    | 1    | 0x00                 |
| Unit        | 2    | 0x27AD (percentage)  |
| Name space  | 1    | 0x01 (Bluetooth SIG) |
| Description | 2    | See below            |

«If the service that instantiates the Battery Level characteristic
represents the battery status for the device,
the Description field should be set to “main”.»

- Main = 0x106
- Other values are found in the "GATT namespace descriptors"
  document, section "GATT Namespace Descriptors", page 2.
  **Requires deeper investigation**

### Battery Level Status

Optional. Read, Notify.

«This characteristic may be used to describe the battery level and power state
of the battery represented by this instance of the Battery Service.»

| Field             | Data type   | bytes size | Specification |
| ----------------- | ----------- | ---------- | ------------- |
| Flags             | boolean[8]  | 1          | Mandatory     |
| Power state       | boolean[16] | 2          | Mandatory     |
| Identifier        | uint16      | 0 or 2     | Optional      |
| Battery level     | uint8       | 0 or 1     | Optional      |
| Additional status | boolean[8]  | 0 or 1     | Optional      |

- Flags field:

  «The bits of this field represent the presence of optional fields.»

  | Bit index | Meaning                   |
  | --------- | ------------------------- |
  | 0         | Identifier present        |
  | 1         | Battery level present     |
  | 2         | Additional status present |
  | 3 to 7    | Reserved                  |

- Power state field:

  | Bit index | Meaning                                  |
  | --------- | ---------------------------------------- |
  | 0         | Battery present                          |
  | 1-2       | Wired External Power Source Connected    |
  | 3-4       | Wireless External Power Source Connected |
  | 5-6       | Battery Charge State                     |
  | 7-8       | Battery Charge Level                     |
  | 9-11      | Charging Type                            |
  | 12-14     | Charging fault reason                    |
  | 15        | reserved                                 |

  - Wired/wireless external power source:
    - 0 = No
    - 1 = Yes
    - 2 = Unknown
    - 3 = Reserved
  - Battery charge state:
    - 0 = Unknown
    - 1 = Charging
    - 2 = Discharging: active
    - 3 = Discharging: inactive

    «If the battery is present and self-discharging,
    or discharging only because of leakage,
    the Battery Charge State bits shall be set to the value
    Discharging: Inactive.
    This also applies when self-discharging or leakage is being compensated.»

  - Battery charge level:
    - 0 = Unknown
    - 1 = Good
    - 2 = Low
    - 3 = Critical

    «If the battery is not present or
    if the monitoring of low and critical battery levels is not supported,
    then the Battery Charge Level bits shall be set to the value Unknown.»

  - Charging type
    - 0 = Unknown or not charging
    - 1 = Constant current
    - 2 = Constant voltage
    - 3 = Trickle
    - 4 = Float
    - 5-7 = Reserved

    «If the battery is not present or
    if the monitoring of charging type is not supported,
    then the value of the Charging Type bits shall be set
    to the value Unknown or Not Charging.»

    - Charging fault reason (boolean[3]):
      - Bit 12 = battery
      - Bit 13 = External power source
      - Bit 14 = Other reason

- Identifier field:

  «This field is used as a service identifier and
  shall be included in the Battery Level Status characteristic
  when multiple instances of the Battery Service are present on a GATT Server
  and the characteristic is broadcast.
  This field is optional otherwise.

  When the Characteristic Presentation Format descriptor
  of the Battery Level characteristic is present,
  the value of this field shall be identical
  to the Description field of that descriptor.
  When the descriptor is not present,
  then this field shall have the value representing “main”.»

  Main = 0x106

- Battery level field:

  «The value of this field shall be identical to the Battery Level characteristic.»

- Additional status field:

  | Bit index | Meaning          |
  | --------- | ---------------- |
  | 0-1       | Service required |
  | 2         | Battery fault    |
  | 3-7       | Reserved         |

  - Service required:
    - 0 = False
    - 1 = True
    - 2 = Unknown
    - 3 = Reserved
  - Battery fault:
    - 0 = False or unknown
    - 1 = Yes
