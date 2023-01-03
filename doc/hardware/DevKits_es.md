# Conozca su placa ESP32 DevKit

Técnicamente, las placas DevKit interoperan a través de pines de entrada/salida de propósito general (GPIO). Sin embargo, hay dos mentiras en esta frase:

- Propósito general: algunos pines son de *propósito específico*, por nombrar algunos: pines I2C y UART. Algunos de esos pines no se pueden usar como pines de propósito general.
- Entrada/Salida: algunos pines no se pueden usar para entrada, otros no se pueden usar para salida.

Esta no es la única molestia:

- El arranque fallará si ciertos pines están configurados en alto voltaje.
- Algunos pines de entrada carecen de resistencias pull-up o pull-down internas que otros tienen.
- Algunos pines se pueden usar como entradas, pero solo en modo pull-up.
- No todos los pines pueden activar el sistema desde el modo de suspensión profunda.
- No todos los pines se pueden usar para entrada analógica.

Como resultado, algunos dispositivos no se pueden conectar a cualquier pin arbitrario, por lo que se necesita un **plan de asignación de pines**. Para desarrollar un plan de este tipo, necesita saber qué restricciones se aplican a su placa DevKit. Busque una hoja de datos.

## Devkit ESP-WROOM-32

El siguiente artículo explica qué pines se pueden usar y cómo:
[https://randomnerdtutorials.com/esp32-pinout-reference-gpios/](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/). Sin embargo, podemos ser más específicos:

- **GPIO #36, #39, #34 y #35**:
  
  - Solo entrada.
  - No apto para codificadores rotatorios desnudos, a menos que se hayan instalado resistencias pull-up externas.
  - No apto para la matriz de botones, a menos que haya resistencias pull-down externas.

- **GPIO n.º 12**:
  
  - No apto para codificadores rotatorios (cualquier tipo).
  - No apto para ninguna entrada pull-up. El arranque fallará.

- **GPIO #6, #7, #8, #9, #10 y #11**
  
  - **INUTILIZABLE**. No conecte nada a esos pines.

- **GPIO #0**:
  
  - A pesar de que se describe como pull-up, parece funcionar en otros modos.

- **GPIO #3**:
  
  - A pesar de que se describe como de entrada, solo funciona en modo pull-up.
  - No apto para la matriz de botones.
  - Perfecto para codificadores rotatorios (cualquier tipo).

- **GPIO #1**:
  
  - Solo salida.

## El problema del bootstrap

Para cargar un firmware en una placa DevKit se utilizan ciertos pines, llamados pines "bootstrap". Si algún dispositivo está conectado a esos pines, puede interferir. Debe separar la placa del circuito antes de cargar el firmware.