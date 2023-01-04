# Subsistema de levas analógicas de embrague

Este subsistema es opcional. Tenga en cuenta que no es muy gentil con las baterías.

## Propósito

Proporcionar dos **potenciometros** como entradas analógicas al sistema. La posición de cada potenciometro es traducida a una posición de un eje lógico, que el usuario puede mapear a varias funciones:

- Botones normales y corrientes: encendido y apagado.
- Dos ejes independientes: de este modo, cada leva puede funcionar como un embrague normal, acelerador, freno o cualquier otra entrada.
- Embrague estilo F1: la posición de ambos ejes se combina en un único eje.
- Botones "ALT".

El subsistema necesita dos pines ADC. Si no están disponibles, los potenciometros aún pueden ser utilizados como se muestra en el [subsistema de interruptores](../Switches/Switches_es.md).

## Diseño del circuito

No hay ningún circuito, solamente cableado:

![Cableado de embrague analógico](./AnalogClutchWiring.png)

Notas:

- Se necesita un pin ADC para cada leva.
- `3V3` y `GND` son intercambiables.
- Se recomiendan potenciometros de alta impedancia (10 K-ohms o más). Los potenciometros dreanarán corriente en todo momento, lo que es malo para las baterías.

## Personalización del Firmware

La personalización tiene lugar en el fichero [CustomSetup.ino](../../../../src/Firmware/CustomSetup/CustomSetup.ino).
Sitúe una única llamada a `inputs::setAnalogClutchPaddles()` dentro de `simWheelSetup()`:

- El primer parámetro es el número de pin ADC para la leva izquierda.
- El segundo parámetro es el número de pin ADC para la leva derecha.
- El tercer parámetro es un número de botón cualquiera para la leva izquierda, en el rango de 0 a 63.
- El cuarto parámetro es un número de botón cualquiera para la leva derecha, en el rango de 0 a 63.

Esos números de botón serán reportados en el modo "botón normal". No tienen ningún otro significado.

Por ejemplo:

```c
void simWheelSetup()
{
    ...
    inputs::setAnalogClutchPaddles(GPIO_NUM_12, GPIO_NUM_13, 40, 41);
    ...
}
```

Observe que `inputs::setDigitalClutchPaddles()` es incompatible y no debe ser ejecutada.

## Autocalibración

Por defecto, se presupone que ambos potenciometros trabajan en todo el rango de voltaje. A menudo, este no es el caso debido a topes físicos en la rotación de un potenciometro. En tal caso, el usuario debería solicitar una "recalibración". Una vez que ambos potenciometros se han movido de extremo a extremo, el rango efectivo de voltaje será tenido en cuenta y salvado a la memoria flash tras un breve retardo.
