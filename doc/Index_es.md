# índice

## Manual de construcción

- [Visión de conjunto](./overview_es.md)
- [Habilidades y herramientas requeridas](./skills_es.md)
- [Preparar el código fuente para la compilación y carga](./firmware/sourcesSetup_es.md)

## Electrónica

- [Conoce tu placa ESP32 DevKit](./hardware/DevKits_es.md)

- [Hardware de entrada adecuado (o no) para un volante o caja de botones](./hardware/InputHW_es.md)

- Diseños listos para implementar:

|                         Nombre                          |       Uso       | Kit de desarrollo  | Resumen de entradas                                                                                           | Fuente de energía | OLED  | Otro |
| :-----------------------------------------------------: | :-------------: | :----------------: | :------------------------------------------------------------------------------------------------------------ | :---------------: | :---: | ---- |
| [Configuración1](./hardware/setups/setup1/Setup1_es.md) |     Volante     | ESP32-WROOM-32E/UE | Codificadores rotatorios x4, DPAD o interruptor funky (opcional), soporte de potenciómetro, hasta 24 entradas | Batería o externa |  si   | BLE  |
| [Configuración2](./hardware/setups/setup2/Setup2_es.md) | Caja de botones | ESP32-WROOM-32E/UE | Codificadores rotatorios x4, hasta 52 entradas                                                                |      externa      |  no   | BLE  |
| [Configuración3](./hardware/setups/setup3/Setup3_es.md) |     Volante     | ESP32-WROOM-32E/UE | Codificadores rotatorios x4, DPAD o interruptor funky (opcional), soporte de potenciómetro, hasta 24 entradas | Batería o externa |  si   | BLE  |

- [Cómo personalizar el firmware para crear su propia configuración de hardware y firmware](./hardware/subsystems/CustomizeHowto_es.md).

## Otros

- [Protocolo UART](./firmware/UARTProtocol_es.md) 

## Contribuyendo

- Formas de contribuir:
  
  - Reportar errores.
  - Contribuir con un diseño abierto para la carcasa del volante.
  - Contribuya con un diseño de circuito listo para implementar para otras placas basadas en ESP32.
  - Comparta su experiencia y contribuya con un tutorial.

- Política de pull request (contribuyendo con código nuevo):
  
  - Se rechazará el código para proyectos personalizados. Proporcione un código del que todos se puedan beneficiar.
  - Todos los controles de calidad relevantes deben actualizarse junto con el nuevo código.
  - Debes pasar todos los controles de calidad.
  - Proporcione una declaración explícita de que todos los controles de calidad fallaron (por lo tanto, el software pasó los controles de calidad).
  - Debe contribuir a la documentación del firmware como lo hace con el código.

## Documentación del firmware (sólo en inglés)

- [Glossary of terms and definitions](./firmware/glossary_en.md)
- [Firmware architecture](./firmware/FirmwareArchitecture_en.md)
- [Overview of Quality Controls](./firmware/FirmwareTesting_en.md)
- [Change log](./changelog.md)