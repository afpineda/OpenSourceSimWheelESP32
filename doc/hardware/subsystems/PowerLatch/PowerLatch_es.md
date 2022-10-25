# Subsistema "Power latch" (cerrojo de energía)

**Usted no construye este subsistema**, puede venir equipado con su módulo/escudo powerboost.

## Objetivo

El propósito de este subsistema es **cortar por completo** la energía del sistema cuando sea necesario, ahorrando así la carga de la batería. Este subsistema es **opcional**. Cuando no esté en su lugar, el sistema entrará en el modo [**sueño profundo**](https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/) en su lugar, que drena la corriente, pero a un ritmo menor.

- No es necesario un circuito de cerrojo de energía si el sistema se alimenta a través de la base del volante o cualquier otra fuente de alimentación externa que no sea a base de baterías.
- Este subsistema requiere un solo pin de salida dedicado (GPIO) en la placa DevKit, denominado `POWER_LATCH`. No es necesario conectar este pin a nada, por lo que puede reservarse para uso futuro.


## Circuito externo

Es posible que su módulo/escudo powerboost venga equipado con este subsistema. Lo sabrá porque hay un botón pulsador (**pero no un interruptor**) para encender/apagar el módulo. Uno de los dos terminales donde se suelda dicho pulsador funcionará como `POWER_LATCH`, por lo que se debe soldar un cable ahí. Para saber cuál, simplemente haga algunas pruebas con un cable. Conecte uno de los terminales a `GND` y espere unos segundos. Si la energía se enciende/apaga, ese terminal es `POWER_LATCH`.

## Personalización de firmware

La personalización se realiza en el archivo [CustomSetup.ino](../../../../src/Firmware/CustomSetup/CustomSetup.ino).

### Sin circuito de cerrojo de energía

Una "fuente de activación" (encendido) es obligatoria si no hay un circuito de cerrojo de energía, esto es, un conjunto de números GPIO. Se recomienda incluso si hay un circuito de cerrojo de energía, como medida alternativa en caso de que no esté cableado. Cualquier cosa conectada a la matriz de botones no puede actuar como una fuente de activación, ya que la matriz de botones no se activará mientras esté en sueño profundo. Hay varias opciones:

#### Fuente de activación: botón RESET

Con esta opción, un botón de reinicio es la única manera de despertarse del sueño profundo: un terminal conectado a "GND" y el otro conectado al pin "EN" en la placa DevKit. Esto **no se recomienda**, ya que el botón de reinicio puede ser presionado por accidente, arruinando así tu carrera. No se requiere personalización de firmware en este caso.

#### Fuente de activación: codificadores rotatorios o un botón pulsador dedicado

Con esta elección se pueden configurar uno o más codificadores rotatorios. Todos ellos tienen que activarse al mismo tiempo para despertar del sueño profundo. Esto es más fiable para sus botones integrados (pin `SW`) que para la rotación (`CLK` o `DT`), siempre que no estén conectados a la matriz de botones.

Busque la siguiente línea en *CustomSetup.ino*:

```c
const gpio_num_t WAKEUP_PINS[] = {};
```

Ponga dentro de los corchetes una lista separada por comas de números GPIO. Esos números GPIO deben conectarse a un pin `CLK`, `DT` o `SW`. **También se puede usar cualquier otra entrada en modo pull-up**. 

Ejemplo:

```c
const gpio_num_t WAKEUP_PINS[] = {GPIO_NUM_39, GPIO_NUM_32};
```

Localice la línea que comienza con `#define WAKEUP_ANYorALL`. Debe coincidir con lo siguiente:

```c
#define WAKEUP_ANYorALL falso
```

Si su configuración personalizada no utiliza codificadores rotatorios, debe proporcionar un botón pulsador exclusivo como fuente de activación. El firmware habilitará las resistencias pull-up internas si están disponibles. `GPIO_NUM_3` es perfecto para esto, ya que siempre está en modo pull-up.

### Con circuito externo de cerrojo de energía

Localice la línea que contiene `#define POWER_LATCH` y asegúrese de que no esté comentada:

```c
#define POWER_LATCH
```

Luego escriba el número GPIO asignado a `POWER_LATCH` en el lado derecho. Por ejemplo:

```c
#define POWER_LATCH GPIO_NUM_1
```