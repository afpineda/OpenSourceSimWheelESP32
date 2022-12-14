# Diseño listo para implementar #2

Lea este documento de principio a fin antes de construir cualquier cosa. Asegúrese de entenderlo todo.

**Esta configuración no ha sido probada a nivel de sistema**. Si lo intenta, por favor, hágamelo saber.

## Características del hardware

- Bluetooth de bajo consumo

- Alimentado a través de USB (opcionalmente, fuente de alimentación externa)

- Botonera (64 entradas):
  - Hasta 52 pulsadores de uso general
  - Hasta 4 codificadores rotativos relativos (con pulsador)

## Piezas necesarias

|                      **Artículo**                      | **Cantidad** | Notas                                                      |
| :----------------------------------------------------: | :----------: | ---------------------------------------------------------- |
|             Codificador rotatorio desnudo              |   hasta 4    |                                                            |
|       Placa perforada estándar de 24x18 agujeros       |      1       |                                                            |
|                       Pulsadores                       |   hasta 52   | De propósito general (a su elección)                       |
|     Cabezal de pines (macho/hembra a su elección)      |     136      | Para cableado externo                                      |
|                    Diodos Schottky                     |      56      | 1N4148 recomendado                                         |
| Resistencia (cualquier impedancia de 4K a 100K-ohmios) |      4       |                                                            |
|             ESP32-WROOM-32UE/E (DevKit-C)              |      1       | Pines macho ya soldados. Elija antena incorporada/externa. |
|     Antena Externa con conector U.FL, MHF I o AMC      |      1       | Solo se requiere si se elige ESP32-WROOM-32UE              |
|        Cable micro-USB lo suficientemente largo        |      1       | Para alimentación                                          |

Otras partes (cantidad desconocida):

- Cable fino.
- Cable con terminales Dupond (para cableado externo). Algunos kits de cables para placas de prototipado harán el trabajo. ¿Masculino o femenino? lo contrario a los encabezados de pines.
- Estaño para soldadura.


## Plan de patillaje para la placa ESP32-DevKit-C

| **GPIO** | **Entrada** | **Salida** |      **Uso**      | **Notas**                                         |
| -------- | ----------- | ---------- | :---------------: | ------------------------------------------------- |
| **36**   | OK          |            | _Matrix input_ 1  | solo entrada (sin resistencia de pull-up interna) |
| **39**   | OK          |            | _Matrix input_ 2  | solo entrada (sin resistencia de pull-up interna) |
| **34**   | OK          |            | _Matrix input_ 3  | solo entrada (sin resistencia de pull-up interna) |
| **35**   | OK          |            | _Matrix input_ 4  | solo entrada (sin resistencia de pull-up interna) |
| **32**   | OK          | OK         |  Matrix input 5   |                                                   |
| **33**   | OK          | OK         |  Matrix input 6   |                                                   |
| **25**   | OK          | OK         |  Matrix input 7   |                                                   |
| **26**   | OK          | OK         |  Matrix input 8   |                                                   |
| **27**   | OK          | OK         |     ROTARY1_A     |                                                   |
| **14**   | OK          | OK         |     ROTARY1_B     | emite señal PWM en el arranque                    |
| **12**   | OK          | OK         |     ROTARY2_A     | el arranque falla a voltaje alto                  |
| **13**   | OK          | OK         |     ROTARY2_B     |                                                   |
| **9**    | x           | x          | **INUTILIZABLE**  | conectado al flash SPI integrado                  |
| **10**   | x           | x          | **INUTILIZABLE**  | conectado al flash SPI integrado                  |
| **11**   | x           | x          | **INUTILIZABLE**  | conectado al flash SPI integrado                  |
| **6**    | x           | x          | **INUTILIZABLE**  | conectado al flash SPI integrado                  |
| **7**    | x           | x          | **INUTILIZABLE**  | conectado al flash SPI integrado                  |
| **8**    | x           | x          | **INUTILIZABLE**  | conectado al flash SPI integrado                  |
| **15**   | OK          | OK         | Matrix selector 1 | emite señal PWM en el arranque                    |
| **2**    | OK          | OK         | Matrix selector 2 | conectado al LED integrado                        |
| **0**    | pull-up?    | OK         | Matrix selector 3 | emite señal PWM en el arranque                    |
| **4**    | OK          | OK         | Matrix selector 4 |                                                   |
| **16**   | OK          | OK         | Matrix selector 5 |                                                   |
| **17**   | OK          | OK         | Matrix selector 6 |                                                   |
| **5**    | OK          | OK         | Matrix selector 7 | emite señal PWM en el arranque                    |
| **18**   | OK          | OK         |     ROTARY3_A     |                                                   |
| **19**   | OK          | OK         |     ROTARY3_B     |                                                   |
| **21**   | OK          | OK         |     _SIN USO_     |                                                   |
| **3**    | pull-up     | RX pin     |     _SIN USO_     | ALTO en el arranque                               |
| **1**    | TX pin      | OK         |     _SIN USO_     | salida de depuración en el arranque               |
| **22**   | OK          | OK         |     ROTARY4_A     |                                                   |
| **23**   | OK          | OK         |     ROTARY4_B     |                                                   |

## Disposición del circuito

Abra el [diseño del circuito](./setup2.diy) usando [DIY Layout Creator](https://github.com/bancika/diy-layout-creator).

![Diseño de circuito de configuración #2](./setup2.png)

Este diseño incluye los siguientes subsistemas (lea para obtener una explicación detallada):

- [Alimentación](../../subsystems/Power/Power_es.md) a través de una fuente de alimentación externa.
- [Switches](../../subsystems/Switches/Switches_es.md) a través de una matriz de botones.
- [Codificador rotatorio relativo](../../subsystems/RelativeRotaryEncoder/RelativeRotaryEncoder_es.md) tipo desnudo.

Notas y consejos de construcción:

- Algunos componentes pueden parecer muy pequeños y no coincidir con su tamaño real. Esto no es un error. Deben colocarse en posición vertical, de modo que queden en una superficie mínima de la placa perforada. Todas las resistencias y diodos deben caber en agujeros de 1x4 cuando estén en posición horizontal.
- Hay mucho cableado, que es propenso a errores humanos. Verifique el cableado y las trazas dos veces antes de soldar.
- Elija una y solamente una fuente de alimentación (USB, `EXTERNAL_5V0` or `EXTERNAL_3V3`). Su placa puede resultar dañada si se usan dos fuentes de alimentación al mismo tiempo. `EXTERNAL_GND` debe usarse junto a `EXTERNAL_5V0` o `EXTERNAL_3V3` (pero no ambos).

### Cableado externo

- Los pulsadores integrados de los codificadores rotatorios están cableados a la matriz de botones (terminales `SW` y `SW GND`) como cualquier otro botón pulsador.
- Cada entrada tiene asignado un número en el esquema del circuito. Ciertas entradas tienen una función particular, así que adjúntelas correctamente.

## Subida del firmware

1. Desconecte la placa DevKit del circuito antes de continuar.
2. Conecte el cable USB a la placa Devkit y cargue el [firmware](../../../../src/Firmware/Setup2/Setup2.ino) con Arduino IDE.
3. Conecte la placa DevKit al circuito. Mantenga el cable USB enchufado.
4. Abra el monitor serie (IDE de Arduino).
5. Reinicie.
6. Verifique que no haya mensajes de error.