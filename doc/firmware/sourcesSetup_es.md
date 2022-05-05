# Procedimiento de preparación del código fuente para compilación y carga

Antes de compilar y cargar *cualquiera* de los bocetos de Arduino proporcionados en este proyecto (en [src/Firmware](../../src/Firmware/) o [src/QualityControls](../../src/QualityControls/)), se debe seguir este procedimiento.

Esto se debe a cómo funciona el IDE de Arduino, __que evita que un único archivo fuente se comparta entre muchos bocetos__. Esos archivos fuente se encuentran en las carpetas [src/common](../../src/common/) y [src/include](../../src/include/).

Este procedimiento se basa en un **script de powershell**, llamado [**MakeSymLinks.ps1**](../../src/MakeSymLinks.ps1). Para ejecutar cualquier script de PowerShell, es posible que primero deba habilitarlos:

1. Ejecute `powershell.exe`

2. Escriba y ejecute:
   
   ```powershell
   Set-ExecutionPolicy unrestricted
   ```

3. Ahora, se puede ejecutar el script, escribiendo:
   
   ```powershell
   <<ruta_a_la_carpeta_del_proyecto>>/src/MakeSymLinks.ps1
   ```
   
   Otra forma es abrir *MakeSymLinks.ps1* con "Powershell ISE" y hacer clic en el icono "reproducir" (o presionar F5).

De todos modos, hay dos opciones:

## Ejecutando con privilegios de administrador (*recomendado*)

En este modo, el script creará enlaces simbólicos a los archivos fuente necesarios. Por lo tanto, no es necesario ejecutarlo más de una vez (a menos que la carpeta de su proyecto se sobrescriba con un comando GIT).

Para ejecutar `powershell.exe` o `Powershell ISE` con privilegios de administrador (en Windows):

1. Localice el icono adecuado con el explorador de archivos o mediante una búsqueda en el menú de Windows.
2. Haga clic derecho en el icono.
3. Elija "Ejecutar como administrador" en el menú emergente.

## Ejecutando *sin* privilegios de administrador

En este modo, el _script_ creará muchas *copias* de los archivos fuente requeridos en cada carpeta de bocetos. Esto significa que debe ejecutarse *nuevamente* si se modifica algún archivo fuente dentro de las carpetas [src/common](../../src/common/) o [src/include](../../src/include/).