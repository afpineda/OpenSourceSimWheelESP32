# Software de cliente Bluetooth UART

Su dispositivo anfitrión (PC, teléfono, etc.) debe estar conectado al sistema para poder enviar/recibir datos. Windows también requiere que el sistema esté emparejado. Estas son algunas aplicaciones gratuitas:

- **Android**:
  
  - [nRF Connect para dispositivos móviles, de Nordic Semiconductors](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp&gl=US)
  - [Terminal bluetooth serie](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal)

- **Apple**:
  
  - [nRF Connect for Mobile, de Nordic Semiconductors](https://apps.apple.com/us/app/nrf-connect-for-mobile/id1054362403)

- **Windows**:
  
  - [Bluetooth LE Explorer](https://www.microsoft.com/en-us/p/bluetooth-le-explorer/9n0ztkf1qd98?msclkid=3e571cc3c48711ec86a9bd20456650f0&activetab=pivot:overviewtab)

# Protocolo UART

El protocolo UART permite que cualquier aplicación en el dispositivo anfitrión envíe y reciba líneas simples de texto al sistema. Cada línea de texto debe terminar con un carácter de nueva línea. Todas los textos distinguen entre mayúsculas y minúsculas.

## Comandos AT disponibles

| Comando       | Formato de parámetro o respuesta   | Acción                                                                |
| ------------- | ---------------------------------- | --------------------------------------------------------------------- |
| AT+A?         | 0, 1 o 2                           | Obtener la función actual de las paletas del embrague (ver más abajo) |
| AT+A=`número` | 0, 1 o 2                           | Establecer la función de las paletas del embrague (ver más abajo)     |
| AT+B?         | "Y" o "N" (sin comillas)           | Comprueba si los botones ALT están asignados a la función ALT         |
| AT+B=Y        | Mayúsculas                         | Usar los botones ALT para la función ALT                              |
| AT+B=N        | Mayúsculas                         | Usar los botones ALT como botones normales                            |
| AT+C?         | número hexadecimal corto con signo | Obtener el punto de mordida del embrague                              |
| AT+C=`número` | número hexadecimal corto con signo | Establecer el punto de mordida del embrague                           |
| AT+R=Y        | Mayúsculas                         | Reinicie la calibración automática de la batería                      |

Las funciones del embrague son:

- 0 --> embrague estilo F1
- 1 --> Modo alternativo
- 2 --> Botones normales y corrientes

El punto de mordida es un byte con signo en el rango de -127 a 127, donde -127 significa "liberado" y 127 significa "completamente embragado".

El PC anfitrión debe leer las respuestas a los comandos AT y actuar en consecuencia. Consulte la [sintaxis y respuestas a comandos AT](https://www.developershome.com/sms/atCommandsIntro2.asp).

# Enviar y mostrar datos de simulación de conducción

Cada fotograma de simulación se puede escribir en una sola línea con este formato (no es un comando AT):

> !`<Marcha>` `<PorcentajeRPM>` `<Velocidad>` `<MapaMotor>` `<ABS>` `<TC>`

Donde `<Marcha>` es un único carácter cualquiera que se pueda visualizar.
Otros campos deben escribirse como **dígitos hexadecimales** en **mayúsculas** (no anteponer "0x"):

- `<Velocidad>` debe escribirse como **cuatro** dígitos hexadecimales para un número entero sin signo de 2 bytes, comenzando por el byte menos significativo (2 dígitos), de izquierda a derecha.
- Otros campos deben escribirse como **dos** dígitos hexadecimales para un entero sin signo de un solo byte.

Se espera que `<PorcentajeRPM>` exprese un porcentaje en el rango 0-100 (decimal).
Si `<Marcha>` es un **espacio en blanco**, no se mostrarán datos.

Por ejemplo:

> !N552D01050403

significa:

- `<Marcha>` = 'N'
- `<PorcentajeRPM>` = 55 (hexadecimal) = 85 (dec)
- `<Velocidad>` = 2D01 (hexadecimal alineado a palabra) = ​​012D (número hexadecimal) = 301 (dec)
- `<MapaMotor>` = 05 (hexadecimal) = 5 (dec)
- `<ABS>` = 04 (hexadecimal) = 4 (dec)
- `<TC>` = 03 (hexadecimal) = 3 (dec)

Los campos se pueden omitir comenzando por el lado derecho. Los mensajes mal formados serán ignorados.