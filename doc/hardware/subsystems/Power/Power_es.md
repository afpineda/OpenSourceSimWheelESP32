# Subsistema de energía

**Usted no construye este subsistema**, solo debe comprar uno.

## Objetivo

El propósito de este subsistema es:

- Proporcionar una fuente de alimentación regulada de 3,3V al resto del sistema, ya sea alimentado por cable, baterías o ambos.
- Si el sistema se alimenta a través de baterías:
  - Cargar las baterías mientras el cable USB está enchufado, incluso si el sistema está en uso.
  - Evitar la sobrecarga de las baterías.
  - Evitar la descarga excesiva de las baterías.
  - **Evitar que las baterías se incendien o exploten**.
  - Maximizar la duración de la batería.

Hay **varias opciones alternativas** para este subsistema y **restricciones** para ellas:

- Usar el regulador de voltaje interno en la placa DevKit, de modo que la energía se tome de una fuente externa.
- Usar un _módulo/escudo powerboost_ (también conocido como _power bank_). Esto es **obligatorio** para un sistema que funciona con batería.
- Algunas placas ESP32 ya integran soporte para batería, lo que resulta en una buena alternativa.

**Advertencia:** No coloque dos o más fuentes de alimentación al mismo tiempo. La placa DevKit se dañará.


## Energía de una fuente externa

Una fuente de alimentación externa debe estar disponible a través de un conector micro-USB o cables Dupond (ambos son compatibles). Sin embargo, la fuente de alimentación externa debe cumplir uno de estos requisitos:

- Tensión estable y constante (`Vcc`) entre 3,0V y 3,3V (pero no más), y un suministro de corriente de 500 mA o más. En este caso, el pin `3V3` en la placa DevKit está conectado a `Vcc`, y `GND` como de costumbre, al igual que si fuera un módulo powerboost.
- Cualquier voltaje (`Vcc`) entre 5V y 12V, constante o no, y una corriente de suministro de 500mA o más. Hay dos opciones de cableado:
  - Cable micro-USB: simplemente conéctelo.
  - `Vcc` conectado al pin `5V0` en la placa DevKit y `GND` como de costumbre. Tenga en cuenta que, a pesar de la etiqueta "5V0", este pin puede soportar hasta 12V. Sin embargo, cuanto mayor sea el voltaje, mayor será el desperdicio de energía y mayor será el calor disipado. Se recomienda no más de 7V.

Si su fuente de alimentación externa no cumple con uno de estos requisitos, opte por un módulo powerboost incluso sin baterías, o busque un regulador de voltaje externo adecuado.

## Energía a través de un módulo powerboost

### Guía de compra

Asegúrese de comprar nada más que un _módulo/escudo powerboost_ o un _power bank_. Un simple cargador de batería o cualquier otro circuito **es peligroso** aunque funcione. Los requisitos son:

- Fuente de alimentación regulada (tensión constante)
- Suministro de corriente de 500mA o más (se recomienda 1A).
- Capaz de suministrar un voltaje de salida de 3V a 3,3V. Algunos módulos powerboost proporcionan tanto 5V como 3,3V.
  - Si su módulo powerboost proporciona 5V, pero no 3,3V, aún puede usarlo. Conecte la salida de 5V al pin `5V0` en la placa DevKit. Esto funciona, pero es **una pérdida de energía** (hay dos reguladores de voltaje en acción), por lo que no se recomienda.
- Paquete de baterías con un voltaje de 3,2V a 4,3V, que se denomina "1S". Es decir, si hay dos o más celdas de batería, se conectan en paralelo, no en serie.

Se recomienda encarecidamente, pero no es obligatorio, que el módulo Powerboost venga equipado con un [circuito de "power latch"](../PowerLatch/PowerLatch_es.md). Hay un circuito de este tipo si hay un _botón momentáneo de encendido/apagado_, pero no un interruptor (no momentáneo).

Compruebe qué tipo de batería es adecuada para el módulo powerboost elegido y compre una.
Tenga en cuenta cuánto espacio hay disponible dentro de la caja de la rueda para colocar todos los componentes, así que no compre un módulo powerboost de gran tamaño.

Algunos DevKits basados ​​en ESP32 ya integran un módulo powerboost, como [Adafruit HUZZAH32 – ESP32 Feather Board](https://www.adafruit.com/product/3405). Adafruit también ofrece dos módulos powerboost independientes (un poco caros, en mi opinión), pero no con capacidad para 3,3V.

Este es [un ejemplo](https://www.wareorigin.com/products/8650-v8-lithium-battery-shield-micro-usb-5v-3a-3v-1a-power-bank-battery-charging-module-for-arduino-esp32-esp8266-wifi) de un banco de energía popular con capacidad de 3,3V que se puede comprar de muchos proveedores.
![Powerboost LiPo 3V3](https://cdn.shopify.com/s/files/1/0258/7145/0157/products/1_d200f2a8-5564-4df9-a8de-b79283da38bd_1024x.jpg?v=1568552186)

Y este es el módulo powerboost utilizado para probar en este proyecto (observe que hay un circuito "power latch"):
![módulo utilizado para las pruebas](./BatteryShieldSpecs.png)

## Diseño del circuito

Si su placa ESP32 ya integra soporte para batería, no se necesita nada más.
No hay circuito involucrado, sin embargo, algunos cables deben soldarse. Si es posible, use cabezales de pines para mayor flexibilidad. Estas etiquetas se utilizarán de ahora en adelante a lo largo de este proyecto:

- **POWERBOOST_3V3**: denominado `3V3` o `3V` en el módulo powerboost.
- **POWERBOOST_GND**: denominado `GND`.
- **EXTERNAL_5V**: denominado `5V` o `5V0` en el módulo powerboost.
- **EXTERNAL_GND**: denominado `GND`.
- **Batería(+)**: el polo positivo de la batería. Simplemente suelde un cable a cualquiera de ellos. Lo requiere el [subsistema monitor de batería](./../BatteryMonitor/BatteryMonitor_es.md).

## Personalización de firmware

No se requiere nada.

## Otras lecturas

Para obtener una explicación sobre cómo alimentar la placa DevKit, consulte estos artículos:

- [Como alimentar tu Devkit ESP32](https://techexplorations.com/guides/esp32/begin/power/)
- [¿Cuál es la mejor batería para el ESP32?](https://diyi0t.com/best-battery-for-esp32/)
- [Fuente de alimentación para ESP32 con cargador de batería y powerboost](https://how2electronics.com/power-supply-for-esp32-with-boost-converter-battery-charger/)