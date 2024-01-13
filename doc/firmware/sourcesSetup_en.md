# Source code setup procedure

Before compiling and uploading *any* of the Arduino sketches provided in this project (under [src/Firmware](../../src/Firmware/) or [src/QualityControls](../../src/QualityControls/)), this procedure must be followed.

This is due to how the Arduino IDE works, **which prevents a single source file to be shared between many sketches**. Those source files are located at folders [src/common](../../src/common/) and [src/include](../../src/include/).

This procedure relays on a **powershell script**, named [**MakeSymLinks.ps1**](../../src/MakeSymLinks.ps1). In order to run any powershell script, you may need to enable them first:

1. Run `powershell.exe`

2. type and run:

   ```powershell
   Set-ExecutionPolicy unrestricted
   ```

3. Now, the script may be run, by typing:

   ```powershell
   <<path_to_project_folder>>/src/MakeSymLinks.ps1
   ```

   Another way is to open *MakeSymLinks.ps1* with "Powershell ISE" and click on the "play" icon (or press F5).

Anyway, there are two choices:

## Running with administrator privileges (*recommended*)

In this mode, the script will create symbolic links to the needed source files. So there is no need to run more than once (unless your project folder gets overwritten by a GIT command).

In order to run `powershell.exe` or `Powershell ISE` with administrator privileges (in Windows):

1. Locate the proper icon with the file explorer or through a search at the windows' menu.
2. Right-click on the icon.
3. Choose "Run as administrator" in the popup menu.

## Running *without* administrator privileges

In this mode, the script will create many *copies* of the required source files at each sketch folder. This means that it must be run *again* if any source file is modified inside folders [src/common](../../src/common/) or [src/include](../../src/include/).
