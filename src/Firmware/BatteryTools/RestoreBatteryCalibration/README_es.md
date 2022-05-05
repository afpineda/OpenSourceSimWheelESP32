# Restauración de los datos de calibración de la batería

Se necesita una copia de los datos de calibración. Los datos de calibración son un conjunto de 32 números encerrados entre llaves y separados por comas.

## Objetivo

Restaurar los datos de calibración de la batería en una variedad de circunstancias:
- Datos de calibración obtenidos de otro usuario con el mismo modelo de batería.
- Pasar de una placa DevKit a otra.
- Pasar de un modelo de batería a otro.
- Memoria flash sobrescrita debido a otro firmware.
- Procedimiento de calibración fallido.
- Otras.

## Procedimiento

1. Edite [RestoreBatteryCalibration.ino](RestoreBatteryCalibration.ino) y localice la siguiente línea:
```c
//const uint16_t customCalibrationData[] =
```

2. Descomente esa línea:
```c
const uint16_t customCalibrationData[] =
```

1. Y escriba los datos de calibración, de llave a llave, después de `=`. Terminar con `;`. Por ejemplo:

```c
const uint16_t customCalibrationData[] = { 0, 0, 0, 0, 0, 0, 0, 0, 122, 254 , 250, 112, 52, 25, , 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
```

4. Guarde el archivo, compile y cargue el firmware en la placa DevKit.
5. Abra el monitor serie.
6. Reinice.
7. La salida debe ser:
```
[ES] Los datos de calibracion de bateria han sido restaurados
--END--FIN--
```

No es necesario ejecutar este firmware más de una vez.