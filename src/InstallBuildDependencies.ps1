<############################################################################

.SYNOPSYS
    Install dependencies for the Arduino build

.AUTHOR
    Angel Fernandez Pineda. Madrid. Spain. 2025.

.LICENSE
    Licensed under the EUPL

#############################################################################>

$arduino_cli = "arduino-cli"

<#############################################################################
# Dependency declaration
#############################################################################>

$esp_core_version = "3.1.3"
$nimble_version = "2.2.1"

<#############################################################################
# Auxiliary functions
#############################################################################>

function Test-ArduinoCLI {
    &$arduino_cli version | Out-Null
    if ($LASTEXITCODE -ne 0) {
        throw "Arduino-cli not found. Check your PATH."
    }
}

<#############################################################################
# MAIN
#############################################################################>

Write-Host "⛏ Installing Arduino dependencies" -ForegroundColor Green

# Initialization
$VerbosePreference = "continue"
$InformationPreference = "continue"
$ErrorActionPreference = 'Stop'

Test-ArduinoCLI

Write-Host "🛈 Updating core index" -ForegroundColor Cyan
& $arduino_cli config init --additional-urls "https://downloads.arduino.cc/packages/package_index.json"
& $arduino_cli core update-index

Write-Host "🛈 Installing ESP32 core ($esp_core_version)" -ForegroundColor Cyan
& $arduino_cli core install "esp32:esp32@$esp_core_version"

Write-Host "🛈 Installing NimBLE-Arduino ($nimble_version)" -ForegroundColor Cyan
& $arduino_cli lib install "NimBLE-Arduino@$nimble_version"
