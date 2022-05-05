> English

# Open source wireless steering wheel/Button box for ESP32 boards

This project provides open hardware and software for a sim-racing wheel or button box, featuring:

- **Wireless connection** through Bluetooth Low Energy with zero configuration effort
- **Several hardware setups** to choose from and detailed instructions about how to **customize** them with **minimum effort**
- Power through rechargeable **batteries** or through the wheel base
- **Alternate mode**. While activated, any input is reported as an alternate one, thus, duplicating the number of effective inputs. Much like the SHIFT key in your keyboard.
- **Clutch paddles** that can be mapped to one of three functions through user preferences:
  - **F1-style clutch** with bite point calibration
  - Alternate mode
  - Regular button
- **ALT buttons** that can be mapped to one of two functions through user preferences
  - Alternate mode
  - Regular button
- **Preferences menu** through OLED (optional), in English and Spanish
- 40 **save slots** to remember your preferences, including clutch's bite point and button mappings 
- Preferences may also be set through **[AT commands](https://www.twilio.com/docs/iot/supersim/introduction-at-commands)** using generic apps for PC or mobile phone (a specific PC app has to be developed yet).
- **Display basic simulator data** like gear and speed through OLED (a specific [SimHub](https://www.simhubdash.com/) plugin has to be developed yet).
- **Automatic power-off** when not in use
- **Quality controls** (you don't want your wheel to fail in the middle of a race)
- **Extensive documentation**
- **Budget friendly**

Please, start at the [documentation index (English)](./doc/Index_en.md).

> Español

# Volante inalámbrico de simulador/caja de botones para placas ESP32 de fuentes abiertas

Este proyecto proporciona hardware y software abierto para un volante de simulación o una caja de botones con las siguientes características:

- **Conexión inalámbrica** a través de Bluetooth de baja energía y sin esfuerzo de configuración
- **Varias configuraciones hardware** para elegir e instrucciones detalladas para **personalizarlas** con el **mínimo esfuerzo** 
- Alimentación mediante **baterías** recargables, o bien, mediante la base del volante
- **Modo alternativo**. Mientras está activado, cualquier entrada se reporta como una entrada alternativa, duplicando su número en consecuencia. Similar a la tecla SHIFT de un teclado.
- **Levas de embrague** que pueden ser configuradas en una de tres funciones:
  - **Embrague tipo F1** con calibración del punto de mordida
  - Modo alternativo
  - Botón normal y corriente
- **Botones "ALT"** que pueden ser configurados en una de dos funciones:
  - Modo alternativo
  - Botón normal y corriente
- **Menú de preferencias** mediante OLED (opcional), en inglés y español
- 40 **ranuras de salvado** para recordar sus preferencias, incluyendo el punto de mordida del embrague y las funciones seleccionadas
- Las preferencias de usuario también se pueden introducir mediante **[comandos AT](http://www.zonabot.com/index.php/en/11-categoria-es-es/electronica/11-comandos-at)** usando aplicaciones genéricas para PC o teléfono móvil (pendiente de desarrollo una aplicación específica para PC).
- **Visualización de datos básicos del simulador** como la marcha y la velocidad a través de la pantalla OLED (pendiente de desarrollo de un _plugin_ para [SimHub](https://www.simhubdash.com/))
- **Apagado automático** cuando no se usa
- **Controles de calidad** (no queremos que el volante falle en medio de una carrera)
- **Documentación extensa**
- **Costes contenidos**

Por favor, comience por el [índice de documentación (español)](./doc/Index_es.md).

**Nota**: la documentación específica de diseño e implementación del software se encuentra únicamente en inglés.

[![License: CC BY 4.0](https://img.shields.io/badge/License-CC%20BY%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by/4.0/)
