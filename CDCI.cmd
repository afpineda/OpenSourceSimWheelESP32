@echo off
powershell .\CD_CI\Build.ps1
if ERRORLEVEL 1 goto end
powershell .\CD_CI\Run.ps1
if ERRORLEVEL 1 goto end
powershell .\CD_CI\Clean.ps1
:end