# Cómo personalizar el firmware para crear su propia configuración de hardware y firmware

Todo el sistema de volante/caja de botones se ha dividido en piezas llamadas **subsistemas**. Cada subsistema realiza una función particular y está compuesto tanto por hardware como (a veces) por software. Una descripción de cada subsistema está disponible a continuación.
Para crear su propia configuración, siga unos sencillos pasos:

1. Elija lo que se requiere de cada subsistema
2. Diseñe su hardware personalizado
3. Configure su firmware personalizado
4. _Opcional pero recomendado_: construya el circuito resultante en una placa de prototipado, cargue su firmware personalizado y pruébelo.
5. Cree su diseño en una [placa perforada](https://en.wikipedia.org/wiki/Perfboard).

## Elija lo que se requiere de cada subsistema

Siga los enlaces para obtener una descripción detallada de cada subsistema:

- [Subsistema de energía](./Power/Power_es.md): proporciona una fuente de alimentación al sistema.
- [Subsistema Power Latch](./PowerLatch/PowerLatch_es.md): proporciona funcionalidad de encendido/apagado.
- [Subsistema monitor de batería](./BatteryMonitor/BatteryMonitor_es.md): proporciona una estimación de la carga de la batería.
- [Subsistema de codificador rotatorio relativo](./RelativeRotaryEncoder/RelativeRotaryEncoder_es.md): proporciona información de la rotación de los codificadores rotatorios.
- [Subsistema de interruptores](./Switches/Switches_es.md): proporciona entrada desde interruptores NO momentáneos (pulsadores, levas, etc.) y potenciómetros (algunos tipos de levas de embrague).
- [Subsistema de embrague analógico](./AnalogClutchPaddles/AnalogClutchPaddles_es.md): proporciona entrada desde dos potenciometros anclados a las levas de embrague. Los embragues digitales están soportados mediante el subsistema de interruptores.

## Diseñe su hardware personalizado

Dependiendo de cuánto espacio esté disponible en la carcasa del volante, tiene la opción de construir todos los subsistemas en una sola placa perforada o dividirlos en varias placas a costa de cableado adicional.

1. Abra [DIY layout creator](https://bancika.github.io/diy-layout-creator/) y cree un nuevo archivo de proyecto.
2. Abra el archivo `.diy` proporcionado para cada subsistema. Copie y pegue lo que necesite en su propio archivo de proyecto. Use el menú "editar" ya que los atajos de teclado no parecen funcionar. También puede comenzar desde una configuración predefinida.
3. Reorganice los componentes para que se ajusten a sus necesidades.

## Configure su firmware personalizado

El boceto de Arduino llamado [**CustomSetup**](../../../src/Firmware/CustomSetup/CustomSetup.ino) es el lugar donde crear su propio firmware modificando algunas líneas específicas de código. Hay algunos pasos:

1. Configure cada subsistema como se describe en su documentación.

2. Configurar entradas:

   Como se muestra para cada subsistema, una llamada a una función en el espacio de nombres `inputs` las habilitará. Debe asignar un "número de entrada" a cada una de ellas, en el rango desde 0 a 63. Cada número de entrada también está asociada a una posición concreta en el cabezal de pines del circuito. Algunos números de entrada tienen un significado concreto en el PC.

3. Asigne ciertos números de entrada a funciones específicas, como se explica a continuación. Edite el cuerpo de `simWheelSetup()` y coloque las llamadas necesarias al final. Todas esas asignaciones son opcionales, pero tenga cuidado de no crear un firmware disfuncional. No asigne dos funciones al mismo número de entrada. Donde esté disponible, no use una combinación de números de entrada que no puedan ser activados al mismo tiempo. No asigne una función específica a números de entrada que no existen.

*Nota:* "..." significa código que no se muestra explícitamente.

### DPAD

A pesar de que esta función está diseñada para interruptores funky y pads direccionales, puede asignarse a _cualquier_ entrada, incluidos los codificadores rotatorios y los botones pulsadores. Haga una llamada a `inputHub::setDPADControls()`:

- El primer parámetro es el número de entrada para el botón "arriba"
- El segundo parámetro es el número de entrada para el botón "abajo"
- El tercer parámetro es el número de entrada para el botón "izquierdo"
- El cuarto parámetro es el número de entrada para el botón "derecho"

Por ejemplo, supongamos que la matriz de botones contiene los números de entrada 20, 22, 25 y 28:

```c
void simWheelSetup()
{
   ...
   inputs::addButtonMatrix(...);
   ...
   inputHub::setDPADControls(20, 22, 25, 28);
   ...
}
```

### Levas de embrague

Las levas de embrague son opcionales. Puede tener levas *analógicas* o *digitales*, pero no ambas. Funcionan igual indistintamente. Las levas analógicas se configuran en el espacio de nombres `inputs` como se relata en el [subsistema correspondiente](./AnalogClutchPaddles/AnalogClutchPaddles_es.md) (no se necesita nada más).

Las levas digitales necesitan exactamente dos números de entrada. Ponga una llamada a `inputs::setDigitalClutchPaddles()`. Cada parámetro (hay dos) es el número de entrada asignado a una leva. Por ejemplo, supongamos que la matriz de botones contiene los números de entrada 45 and 46:

```c
void simWheelSetup()
{
   inputNumber_t btnMatrixNumbers = [ ..., 45, 46, ...];
   ...
   inputs::addButtonMatrix(... , btnMatrixNumbers);
   ...
   inputs::setDigitalClutchPaddles(45, 46);
   ...
}
```

### Controles de calibración del punto de mordida del embrague

Haga una llamada a `inputHub::setClutchCalibrationButtons()`:

- El primer parámetro es un número de entrada para aumentar el punto de mordida.
- El segundo parámetro es un número de entrada para disminuir el punto de mordida.

Por ejemplo, supongamos que esta función está asignada a un codificador rotatorio (números de entrada 34 y 35):

```c
void simWheelSetup()
{
   ...
   inputs::addRotaryEncoder(..., 34, 35, false);
   ...
   inputHub::setClutchCalibrationButtons(34, 35);
   ...
}
```

### Botones ALT

Puede asignar esta función a cualquier número de botones (o a ninguno). Haga una llamada a `inputHub::setALTBitmap()`.
Hay un parámetro: una secuencia de llamadas a `BITMAP(<número de entrada>)` separadas por `|`.

Por ejemplo, supongamos que esta función está asignada a dos entradas determinadas en la matriz de botones:

```c
void simWheelSetup()
{
   inputNumber_t btnMatrixNumbers = [ ..., 45, 46, ...];
   ...
   inputs::addButtonMatrix(..., btnMatrixNumbers);
   ...
   inputHub::setALTBitmap(
      BITMAP(45) | BITMAP(46)
   );
   ...
}
```

### Rotar el modo de funcionamiento de las levas de embrague

Cada vez que eta función es activada, el modo de funcionamiento de las levas de embrague pasa al siguiente disponible: embrague tipo F1, ejes independientes, modo alternativo, botones normales y vuelta al principio. Esto no tiene sentido si no hay levas de embrague.

Asigne una combinación de números de entrada para activar esta función mediante una llamada a `inputHub::setCycleClutchFunctionBitmap()`. Tiene un parámetro: una secuencia de llamadas a `BITMAP(<número de entrada>)` separadas por `|`. Todas las entradas deben estar activas al mismo tiempo y ninguna de las otras.
Por ejemplo:

```c
void simWheelSetup()
{
   inputNumber_t btnMatrixNumbers = [ ..., 60, 61, ...];
   ...
   inputs::addButtonMatrix(... , btnMatrixNumbers);
   ...
   inputHub::setCycleClutchFunctionBitmap(BITMAP(60)|BITMAP(61));
   ...
}
```

#### Seleccionar un modo de funcionamiento específico para las levas de embrague

Como alternativa, puede asignar una combinación de botones a modos de funcionamiento específicos. Situe una llamada a `inputHub::setSelectClutchFunctionBitmaps()`. Hay cuatro parámetros. Cada uno debería contener una secuencia de llamadas a  `BITMAP(<número de entrada>)` como se vio en las llamadas anteriores:

- Primer parámetro: combinación de botones para seleccionar el embrague tipo F1.
- Segundo parámetro: combinación de botones para seleccionar ejes independientes.
- Tercer parámetro: combinación de botones para seleccionar el modo alternativo.
- Cuarto parámetro: combinación de botones para seleccionar el modo de "botones normales".

Por ejemplo:

```c
void simWheelSetup()
{
   inputNumber_t btnMatrixNumbers = [ ..., 59, 60, 61, 62, 63];
   ...
   inputs::addButtonMatrix(... , btnMatrixNumbers);
   ...
   inputHub::setSelectClutchFunctionBitmaps(
      BITMAP(59)|BITMAP(60),
      BITMAP(59)|BITMAP(61),
      BITMAP(59)|BITMAP(62),
      BITMAP(59)|BITMAP(63) );
   ...
}
```

### Rotar el modo de funcionamiento de los botones "ALT"

Cada vez que se active esta función, el modo de funcionamiento de los botones "ALT pasará al siguiente: modo alternativo, botones normales y vuelta al principio. Eso no tiene sentido si no hay botones "ALT".

Asigne una combinación de números de entrada para activar esta función mediante una llamada a  `inputHub::setCycleALTFunctionBitmap()`. Tiene un parámetro: una secuencia de llamadas a `BITMAP(<número de entrada>)` separadas por `|`. Todas las entradas deben estar activas al mismo tiempo y ninguna de las otras.

### Otros controles de gamepad

Tenga en cuenta que los siguientes números de entrada tienen un significado especial en Windows:

- _00_: "A"
- _01_: "B"
- _02_: "X"
- _03_: "Y"
- _04_: "LB" (debe reservarse para la leva de cambio izquierda)
- _05_: "RB" (debe reservarse para la leva de cambio derecha)
- _06_: "Atrás"
- _07_: "Inicio"

## Cree su diseño en una placa perforada

Algunos de los diseños de circuitos pueden mostrar resistencias y diodos muy pequeños que no se ajustan a los reales. Esto no es un error. Deben colocarse en disposición "vertical", de modo que queden en una superficie mínima de la placa perforada.

<img src="../pictures/VerticalLayout.png" alt="Diseño vertical" width="40%" margin-lefet="auto" />

Tenga en cuenta que algunos componentes se pueden colocar encima de otros para ahorrar más espacio.
