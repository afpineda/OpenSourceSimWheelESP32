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
- [Subsistema de pantalla](./Display/Display_es.md): proporciona una interfaz de usuario.

## Diseñe su hardware personalizado

Dependiendo de cuánto espacio esté disponible en la carcasa del volante, tiene la opción de construir todos los subsistemas en una sola placa perforada o dividirlos en varias placas a costa de cableado adicional.

1. Abra [DIY layout creator](https://bancika.github.io/diy-layout-creator/) y cree un nuevo archivo de proyecto.
2. Abra el archivo `.diy` proporcionado para cada subsistema. Copie y pegue lo que necesite en su propio archivo de proyecto. Use el menú "editar" ya que los atajos de teclado no parecen funcionar. También puede comenzar desde una configuración predefinida.
3. Reorganice los componentes para que se ajusten a sus necesidades.

## Configure su firmware personalizado

El boceto de Arduino llamado [**CustomSetup**](../../../src/Firmware/CustomSetup/CustomSetup.ino) es el lugar donde crear su propio firmware modificando algunas líneas específicas de código. Hay algunos pasos:

1. Configure cada subsistema como se describe en su documentación.

2. Configurar entradas:
   
   Como se muestra para cada subsistema, una llamada a una función en el espacio de nombres `inputs` las habilitará. Esas funciones devolverán el número asignado para la primera entrada, pero puede imaginárselo de antemano. El número asignado a ciertas entradas debe conocerse para el siguiente paso. Cada número de entrada también se asocia a una determinada posición en un cabezal de pines en su diseño hardware.

3. Asigne ciertos números de entrada a funciones específicas, como se explica a continuación. Edite el cuerpo de `simWheelSetup()` y coloque las llamadas necesarias al final. Todas esas asignaciones son opcionales, pero tenga cuidado de no crear un firmware disfuncional. No asigne dos funciones al mismo número de entrada (excepto para la navegación del menú).

*Nota:* "..." significa código que no se muestra explícitamente.

### DPAD

A pesar de que esta función está diseñada para interruptores funky y pads direccionales, puede asignarse a _cualquier_ entrada, incluidos los codificadores rotatorios y los botones pulsadores. Haga una llamada a `inputHub::setDPADControls()`:

- El primer parámetro es el número de entrada para el botón "arriba"
- El segundo parámetro es el número de entrada para el botón "abajo"
- El tercer parámetro es el número de entrada para el botón "izquierdo"
- El cuarto parámetro es el número de entrada para el botón "derecho"

Por ejemplo, supongamos que esta función está asignada a ciertas entradas en la matriz de botones:

```c
void simWheelSetup()
{
   ...
   inputNumber_t mtxFirstBtn = inputs::setButtonMatrix(...);
   ...
   inputHub::setDPADControls(
      mtxFirstBtn+10,
      mtxFirstBtn+12,
      mtxFirstBtn+16,
      mtxFirstBtn+18);
   ...
}
```

### Levas de embrague

Haga una llamada a `inputHub::setClutchPaddles()`. Cada parámetro (hay dos) es el número de entrada asignado a una leva de embrague.
Por ejemplo, supongamos que esta función está asignada a ciertas entradas en la matriz de botones:

```c
void simWheelSetup()
{
   ...
   inputNumber_t mtxFirstBtn = inputs::setButtonMatrix(...);
   ...
   inputHub::setClutchPaddles(
      mtxFirstBtn+14,
      mtxFirstBtn+15);
   ...
}
```

### Controles de calibración del punto de mordida del embrague

Haga una llamada a `inputHub::setClutchCalibrationButtons()`:

- El primer parámetro es un número de entrada para aumentar el punto de mordida.
- El segundo parámetro es un número de entrada para disminuir el punto de mordida.

Por ejemplo, supongamos que esta función está asignada a un codificador rotatorio:

```c
void simWheelSetup()
{
   ...
   inputNumber_t rotary1Clockwise = inputs::addRotaryEncoder(...);
   ...
   inputHub::setClutchCalibrationButtons(
      rotary1Clockwise,
      rotary1Clockwise+1);
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
   ...
   inputNumber_t mtxFirstBtn = inputs::setButtonMatrix(...);
   ...
   inputHub::setALTBitmap(
      BITMAP(mtxFirstBtn) | BITMAP(mtxFirstBtn+1)
   );
   ...
}
```

### Entrar/salir del menú

Esta función es inútil si no hay un subsistema OLED. Puede asignar esta función a cualquier número de botones pulsadores (la rotación de un codificador giratorio no funcionará). Todos esos botones, y ninguno de los otros, deben presionarse al mismo tiempo, mantenerse presionados durante dos segundos y luego soltarse para entrar/salir del menú de configuración. Si esos botones se presionan de otra manera, **se informarán al PC anfitrión** como cualquier otra entrada. Para evitar la activación accidental, asigne dos números de botón.

Realice una llamada a `inputHub::setMenuBitmap()`. Hay un parámetro: una secuencia de llamadas a `BITMAP(<número de entrada>)` separadas por `|`. Por ejemplo, supongamos que esta función está asignada a dos botones determinados en la matriz de botones:

```c
void simWheelSetup()
{
   ...
   inputNumber_t mtxFirstBtn = inputs::setButtonMatrix(...);
   ...
   inputHub::setMenuBitmap(
      BITMAP(mtxFirstBtn+20) | BITMAP(mtxFirstBtn+22)
   );
   ...
}
```

### Menú de navegación

Esta función es inútil si no hay un subsistema OLED.

En este caso, los números de entrada asignados se pueden compartir con otras funciones. Haga una llamada a `configMenu::setNavButtons()`:

- El 1er parámetro es el número de entrada asignado para navegar a la opción anterior.
- El segundo parámetro es el número de entrada asignado para navegar a la siguiente opción.
- El tercer parámetro es el número de entrada asignado para seleccionar una opción del menú.
- El cuarto parámetro es el número de entrada asignado para cancelar.

Por ejemplo, digamos que la navegación está asignada a un codificador rotatorio (igual que en el ejemplo anterior), "seleccionar" está asignada a su botón integrado y "cancelar" está asignada a una leva de embrague (igual que en el ejemplo anterior) :

```c
void simWheelSetup()
{
   ...
   inputNumber_t mtxFirstBtn = inputs::setButtonMatrix(...);
   inputNumber_t rotary1Clockwise = inputs::addRotaryEncoder(...);
   inputNumber_t rotary1Button = inputs::addDigital(...);
   ...
   inputHub::setClutchPaddles(
      mtxFirstBtn+14,
      mtxFirstBtn+15);
   inputHub::setClutchCalibrationButtons(
      rotary1Clockwise,
      rotary1Clockwise+1);
   ...
   configMenu::setNavButtons(
      rotary1Clockwise,
      rotary1Clockwise+1,
      rotary1Button,
      mtxFirstBtn+14
   );
   ...
}
```

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