# Subsistema de visualización

**Usted no construye este subsistema**, solo debe comprar uno.

## Objetivo

El propósito de este subsistema es mostrar mensajes en una pequeña pantalla OLED, una parte de la interfaz de usuario requerida por el menú de configuración. Este subsistema es **opcional**. Si no está en su lugar, no estará disponible ningún menú de configuración, por lo que la única forma en que el usuario puede configurar su volante es a través de [**comandos AT**](./../../../firmware/UARTProtocol_es.md).

## Guía de compra

La pantalla debe cumplir con todos estos requisitos:

- Adecuado para una fuente de alimentación de 3,3V.
- Con interfaz a través del [protocolo I2C](https://es.wikipedia.org/wiki/I%C2%B2C).
- Compatible con la [biblioteca ss_oled](https://github.com/bitbank2/ss_oled). Eche un vistazo a su documentación.

Tenga en cuenta que **lo que compre puede no coincidir exactamente con lo que dice el vendedor**. Esto no debería ser un problema si sabe lo que sucede. Eche un vistazo a la sección [solución de problemas](#solución-de-problemas).

La resolución recomendada es de 128x64 y el tamaño recomendado es de 1,3 pulgadas. Este es el OLED utilizado para las pruebas en este proyecto:

![OLED utilizado para las pruebas](./OLEDdatasheet128x64.jpg)

## Cableado externo

- `VDD` o `Vcc` conectado a 3V3 en la placa DevKit o en el módulo/escudo powerboost.
- `GND` conectado a `GND` en la placa DevKit.
- `SCK` o `SCL` conectado a GPIO 22 (también llamado `WIRE_SCL`) en la placa DevKit.
- `SDA` conectado a GPIO 21 (también llamado `WIRE_SDA`) en la placa DevKit.


## Configuración del software

La personalización se realiza en el archivo [CustomSetup.ino](../../../../src/Firmware/CustomSetup/CustomSetup.ino).

Si no hay OLED, comente la línea `#define ENABLE_OLED` de la siguiente manera y deje de leer:

```c
//#definir ENABLE_OLED 0
```

De lo contrario, esa línea debe verse de la siguiente manera:

```c
#define ENABLE_OLED 0
```

Escriba el ancho y el alto de su pantalla, en píxeles, a la derecha de `#define OLED_SCREEN_WIDTH` y `#define OLED_SCREEN_HEIGHT`, respectivamente. Por ejemplo:

```c
#define OLED_SCREEN_WIDTH 128
#define OLED_SCREEN_HEIGHT 64
```

Escriba el controlador de su pantalla a la derecha de `#define OLED_TYPE`. Los valores válidos son `SSOLED_128x128`, `SSOLED_128x32`, `SSOLED_128x64`, `SSOLED_132x64`, `SSOLED_64x32`, `SSOLED_96x16` y `SSOLED_72x40`. Por ejemplo:

```c
#define OLED_TYPE SSOLED_132x64
```

Si su pantalla está montada al revés, la siguiente línea debe estar en su lugar:

```c
#define OLED_FLIP verdadero
```

De lo contrario, debe verse así:

```c
#define OLED_FLIP falso
```

### Solución de problemas

#### Píxeles desplazados

Este es un problema habitual. Pruebe con otro controlador (tipo de pantalla). Por ejemplo, `SSOLED_132x64` en lugar de `SSOLED_128x64`.

#### Sin pantalla

Primero revise su cableado. Si el cableado es correcto, la causa más probable es una dirección I2C incorrecta. Una forma de diagnosticarlo es cargar y ejecutar los ejemplos de la biblioteca `ss_oled`.

Esa biblioteca debería detectar la dirección correcta, pero podría fallar. Puede encontrar otro boceto de detección automática de direcciones aquí: [I2CAddressScanner.ino](../../../../src/Firmware/I2CAddressScanner/I2CAddressScanner.ino). En tal caso, debe modificar la implementación de `ui::begin()` en el archivo [ui_ssoled.cpp](../../../../src/common/ui_ssoled.cpp), configurando:

```c
ssoled.oled_addr = <<dirección correcta>>;
```

antes de la primera llamada a `oledInit()`.

## Otras lecturas

- [Pantalla OLED con ESP32](https://www.tutorialspoint.com/esp32_for_iot/interfacing_oled_display_with_esp32.htm)
- [OLED desplazado](https://forum.arduino.cc/t/oled-shifted/323480/2)
- [Pantalla OLED ESP32 con Arduino IDE](https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/)