# Volcar y (opcionalmente) borrar los datos de calibración de la batería

## Objetivo

Utilice este procedimiento para volcar los datos actuales de calibración de la batería en el puerto serie y, opcionalmente, borrarlos.

## Procedimiento

1. Cargue el [firmware](./DumpAndClearBatteryCalibration.ino).
2. Abra el monitor serie **rápidamente**.
3. Reinicie.
4. Se mostrarán los datos de calibración actuales.
5. Después de una cuenta atrás de 60 segundos, dichos datos se borrarán. Si esa no es su intención, desconecte el cable USB.