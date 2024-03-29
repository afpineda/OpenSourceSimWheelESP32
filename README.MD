<p align="center">
    <img width="250" src="./logo/ESP32SimWheel_logo_400x400.png" alt="Project logo"/>
</p>

> English

# Open-source wireless steering wheel or button box for ESP32 boards

This project provides open hardware and software for a sim-racing wheel or button box, featuring:

- **Wireless connectivity** through **Bluetooth Low Energy**, with zero configuration effort
- **Wired connectivity** is also available through **USB**.
- There are **several hardware setups** to choose from and detailed instructions about how to **customize** them with **minimum effort**.
- Power through rechargeable **batteries** or through the wheel's base
- **Alternate mode**. While engaged, any input is reported as an alternate one, thus duplicating the number of effective inputs. Much like the SHIFT key on your keyboard.
- **Clutch paddles** that can be mapped to one of four functions through user preferences:
  - **F1-style clutch** with bite-point calibration
  - Alternate mode
  - Regular button
  - Individual analog axes.
- **ALT buttons** that can be mapped to one of two functions through user preferences:
  - Alternate mode
  - Regular button.
- One **directional pad or funky switch** can be mapped to one of two functions through user preferences:
  - Navigation controls
  - Regular buttons. Navigation controls cannot be assigned to car functions in some simulators. This solves that problem.
- **User-defined button mapping**. The user may adjust the button numbering on this steering wheel to match any other steering wheel. In this way, the simulator does not force the user to reconfigure inputs each time those wheels are swapped.
- **Automatic power-off** when not in use
- **Quality controls** (you don't want your wheel to fail in the middle of a race)
- **Extensive documentation**
- **Low budget**.

Please start at the [documentation index](./doc/Index_en.md).

There is a [companion app](https://github.com/afpineda/SimWheelESP32Config) for user preferences, not mandatory but required for user-defined button mapping.

This project is **fully operational**.

> Español

# Volante inalámbrico de simulador o caja de botones, de fuentes abiertas, para placas ESP32

Este proyecto proporciona hardware y software abierto para un volante de simulación o una caja de botones con las siguientes características:

- **Conectividad inalámbrica** a través de **Bluetooth de baja energía** y sin esfuerzo de configuración.
- **Conectividad por cable** también disponible mediante **USB**
- **Varias configuraciones hardware** para elegir e instrucciones detalladas para **personalizarlas** con el **mínimo esfuerzo**
- Alimentación mediante **baterías** recargables, o bien, mediante la base del volante
- **Modo alternativo**. Mientras está activado, cualquier entrada se reporta como una entrada alternativa, duplicando su número en consecuencia. Similar a la tecla SHIFT de un teclado.
- **Levas de embrague** que pueden ser configuradas en una de cuatro funciones:
  - **Embrague tipo F1** con calibración del punto de mordida
  - Modo alternativo
  - Botón normal y corriente
  - Ejes analógicos independientes.
- **Botones "ALT"** que pueden ser configurados en una de dos funciones:
  - Modo alternativo
  - Botón normal y corriente.
- Una **cruceta direccional o funky switch** puede ser configurada en una de dos funciones:
  - Controles de navegación
  - Botones normales y corrientes. Los controles de navegación no se pueden asignar a funciones del coche en algunos simuladores. Esto solventa dicho problema.
- **Mapa de botones definido por el usuario**. El usuario puede ajustar la numeración de los botones de manera que coincida con cualquier otro volante. De esta manera, el simulador no fuerza al usuario a configurar las entradas cada vez que intercambia los volantes.
- **Apagado automático** cuando no se usa
- **Controles de calidad** (no queremos que el volante falle en medio de una carrera)
- **Documentación extensa**
- **Costes contenidos**.

Por favor, comience por el [índice de documentación (inglés)](./doc/Index_en.md).

Existe una [aplicación auxiliar](https://github.com/afpineda/SimWheelESP32Config) para las preferencias de usuario, no obligatoria,
pero necesaria para establecer el mapa de botones definido por el usuario.

Este proyecto está **totalmente operativo**.
