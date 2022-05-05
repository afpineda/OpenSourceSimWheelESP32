# Unit test: current language and translated strings

## Purpose and summary

To test that:

1. Language preference is properly stored and then retrieved at startup
2. All string messages are properly translated to every language preference.

## Hardware setup

Nothing required.
Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Reset.
2. Ignore all output and wait for `--END--` to show up.
3. Reset **again**.
4. Output must match the following:
   
```
--START--
Current language: OK
--GO--
Clutch paddles
Clutch
ALT
Button
Bite point
Language
Slot %02.2d
Load preset
Save preset
Exit
Saved!
Connected!
ALT buttons
Power off
Battery
Recalibrate
On
Off
Wellcome
English
Espanol
Menu
Frameserver
----
Levas embrague
Embragar
ALT
Boton
P. mordida
Idioma
Ranura %02.2d
Cargar ajustes
Salvar ajustes
Salir
Guardado!
Conectado!
Botones ALT
Apagar
Bateria
Recalibrar
Si
No
Bienvenido
English
Espanol
Menu
Frameserver
--END--
```