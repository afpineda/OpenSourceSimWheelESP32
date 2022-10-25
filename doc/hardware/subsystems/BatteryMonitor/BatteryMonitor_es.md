# Subsistema monitor de batería

Este subsistema es para una configuración **que funcione con batería**. No tiene sentido si hay una fuente de alimentación externa instalada.

## Objetivo

El propósito de este subsistema es proporcionar una estimación de cuánta carga queda en la batería, para que el usuario sepa cuándo enchufar el cable de carga. El nivel de la batería es un porcentaje de carga restante en el rango de 0% a 100%. EL PC anfitrión conoce el nivel de la batería a través de Bluetooth Low Energy.

## Circuito

Este circuito está diseñado para baterías de polímero de litio (LiPo) "1S". Es posible que se requieran algunos ajustes para otros tipos de baterías.

### Principios de funcionamiento

Un monitor de batería necesita leer el voltaje de salida de la batería (*no* el voltaje de salida del módulo powerboost), es decir, el voltaje en el terminal positivo (el terminal negativo está conectada a tierra ("GND") en el módulo powerboost). A medida que la batería se descarga, ese voltaje bajará. El voltaje de salida de una batería LiPo está en este rango (más o menos):

- 4V a 5V cuando el módulo powerboost está cargando la batería.
- 3,7V cuando la batería está completamente cargada.
- Alrededor de 2,4V o menos cuando la batería está descargada. El módulo powerboost cortará la energía para evitar una descarga excesiva.

De alguna manera, este voltaje de salida debe leerse a través de un pin ADC, sin embargo, existen dos restricciones:

- El voltaje de salida de la batería supera los 3,3V y dañaría la placa DevKit, por lo que el voltaje debe caer a un rango adecuado.
- Leer el voltaje de salida podría descargar la batería, por lo que el monitor de batería no debería consumir ninguna corriente relevante. El firmware leerá el nivel de la batería cada pocos minutos y solo toma unos pocos milisegundos. De esta manera, se conserva la carga de la batería. El circuito se enciende y apaga por medio de un par NPN-PNP.


### Diseño del circuito

El circuito tiene las siguientes entradas:

- Pin **Battery (+)** (terminal positivo de la batería): se debe soldar un cable a ese terminal en el módulo powerboost.
- pin **battEN**: habilita o deshabilita el circuito. Conectado a un pin GPIO con capacidad de salida en la placa DevKit.

Y una salida:

- Pin **battREAD**: proporciona el nivel actual de la batería. Conectado a un pin GPIO compatible con ADC en la placa DevKit.

El pin `GND` se comparte con el circuito principal, conectado a `GND` en el módulo powerboost.

![Circuito del monitor de batería](./BatteryMonitor_falstad.png)

[Esta simulación en falstad.com](https://falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWcMBMcUHYMGZIA4UA2ATmIxAUgoqoQFMBaMMAKABcQmwVxieHCAFl48eEGJTAE8g4pDQJshLFGiKEmfJG4oEgsJGLIqAEzoAzAIYBXADZsWAc07YeuYQ1chXVXywDunHhUwUG+UCwATlAgoVRgGIQxghhwUTEJSVSYPFQpadFMiSKczGJ8GfAFnELg3DXCmZVVAS5ukB5eOeCt2Rj8Xf0RgQLC3aMgaHgRHBNlDbFU2KWcEgZ4OPJggpAIxNgeEgTYlMRSBpDYUo1wIGZWdg4AMqXFYBUMuknvueAgVrYAM50bzQZaQVoMQTTOKLCIAeQyb0EjTewzakyGE26EIAHqUOi5OBgjHkQEkAEaWNhsOiRACeAAoADoABwAlCx8Z8EOQwIQkJ95OBCNNGtMqTSAKIAOS55KM-KQ20VopA4pAkrYACUpQBBAAiLAAymEYkxCOEqEkAcCYhgWNrSpbONDSh83eEdtQrWpHc6qJ5+CgUJ1fnkyb6EP6LYH+e7+PGvUtoOQozH+fEkkwKknkksYr4-U7Y+aQ8IreqU+DC8WA95g6GG4Wq5NUy2JBmXW6czxPfm2zWi9HueNBNndBAocsbiAALJ6gAa8s8ggGU90gZQYuQIAAwgALSyRRwASwAdo5mYCAPTXvfHxyWc8mAD2K+wJ1KwconCMs8PR86BMa870BB8T0sExLA-A4aE4EMeEUdVdwNU9AQAYyPE9gNA68DToTDHygmCKRcccajEODajxSjXTqaYmBQ+NjVPABbOxqVwwEtTpelrwAN1fexLEcOgVwwCAEnAENJjJeMADVhLYSwACs6GvMxr0lOkAFvLGvQF2M4t8WCAA) muestra cómo funciona, pero tenga en cuenta que el rectángulo es no es parte del circuito.

Piezas necesarias:

- Resistencia de 100K-ohmios (x2)
- Resistencias de 4,7K-ohmios (x2) **1% de tolerancia**. Cualquier impedancia en el rango de 1K-100K ohmios funcionará, siempre que ambas sean idénticas.
- Un transistor de unión bipolar (x1), tipo NPN: cualquiera debería funcionar (por ejemplo: [BC637](https://www.onsemi.com/pdf/datasheet/bc637-d.pdf)).
- Un transistor de unión bipolar (x1), tipo PNP: cualquiera debería funcionar (por ejemplo: [BC640](https://www.onsemi.com/pdf/datasheet/bc640-d.pdf)).

Preste atención al patillaje de sus transistores. *Puede no coincidir* con el que se muestra aquí.

Mire esta [disposición del circuito](./BatteryMonitor.diy) usando [DIY Layout Creator](https://github.com/bancika/diy-layout-creator).

![Diseño de placa](./BatteryMonitor_diy.png)


## Personalización de firmware

La personalización se realiza en el archivo [CustomSetup.ino](../../../../src/Firmware/CustomSetup/CustomSetup.ino).
Asegúrese de que la siguiente línea de código esté en su lugar:

```c
#define ENABLE_BATTERY_MONITOR
```

Localice la línea `#define BATTERY_ENABLE_READ_GPIO` y escriba un número GPIO a la derecha donde se conecta `battEN`. Localice la línea `#define BATTERY_READ_GPIO` y escriba un número GPIO a la derecha donde se conecta `battREAD`. Por ejemplo:

```c
#define BATTERY_ENABLE_READ_GPIO 0
#define BATTERY_READ_GPIO 36
```

Solo se puede proporcionar una estimación aproximada de la carga disponible de la batería. El nivel de la batería no será fiable hasta que la batería esté completamente cargada por primera vez.

Para obtener niveles precisos de la batería se puede seguir un procedimiento de calibración, que está ampliamente documentado [aquí](../../../../src/Firmware/BatteryTools/BatteryCalibration/README_es.md) junto con el firmware requerido.