<############################################################################

.SYNOPSYS
    Compile all sketches in batch

.USAGE
    Create a file named ".fqbn" anywhere in the path to each sketch folder.
    The first line in that file must contain a Full Qualified Board Name
    to be used for compilation.
    If no ".fqbn" file is found, the sketch will be ignored.
    Arduino-cli must be available in the PATH environment variable.
    For output redirection:
    ./BatchCompile.ps1 *>filename.txt

.AUTHOR
    Ángel Fernández Pineda. Madrid. Spain. 2024.

.LICENSE
    Licensed under the EUPL

#############################################################################>

#setup
$ErrorActionPreference = 'Stop'

$thisPath = Split-Path $($MyInvocation.MyCommand.Path) -parent
Set-Location $thispath

# global constants
$_compiler = "arduino-cli"
$_fqbn_file = ".fqbn"

<#############################################################################
# Auxiliary functions
#############################################################################>
function Test-ArduinoCLI {
    &$_compiler version | Out-Null
    if ($LASTEXITCODE -ne 0) {
        throw "Arduino-cli not found. Check your PATH."
    }
}

function Get-SketchFiles {
    Get-ChildItem -Recurse -Filter *.ino | ForEach-Object { $_.FullName }
}

function Find-FQBN {
    param(
        [string]$LiteralPath
    )
    $candidate = Join-Path $LiteralPath $_fqbn_file
    if (Test-Path $candidate) {
        # File exists. Read first line only.
        return Get-Content -Path $candidate -TotalCount 1
    }
    else {
        # File does not exist. Look for it in the parent folder.
        try {
            $parent = Split-Path -Parent $LiteralPath
            return Find-FQBN -LiteralPath $parent
        }
        catch {
            # There is no parent folder
            return
        }
    }
}

function Invoke-ArduinoCLI {
    param(
        [string]$Filename,
        [string]$BuildPath
    )
    Write-Host "--------------------------------------------------------------------------------"
    Write-Host "Sketch: $Filename"

    $sketch_folder = Split-Path -Parent $Filename
    $fqbn = Find-FQBN -LiteralPath $sketch_folder
    if ($null -ne $fqbn) {
        Write-Host "FQBN: $fqbn"
        Write-Host "================================================================================"
        $ErrorActionPreference = 'Continue'
        &$_compiler compile $Filename -b $fqbn --no-color --warnings all --build-path $BuildPath # 2>&1
        $ErrorActionPreference = 'Stop'
    }
    else {
        Write-Host "No FQBN file found. Ignoring."
    }
}

function New-TemporaryFolder {
    $File = New-TemporaryFile
    Remove-Item $File -Force | Out-Null
    $tempFolderName = Join-Path $ENV:Temp $File.Name
    $Folder = New-Item -Itemtype Directory -Path $tempFolderName
    return $Folder
}

## Compile command example:
# &$_compiler compile $sketch -b esp32:esp32:esp32s3 --no-color

<#############################################################################
# MAIN
#############################################################################>

# Initialization
$VerbosePreference = "continue"
$InformationPreference = "continue"
Test-ArduinoCLI

# Create a temporary folder (will speed up compilation)
$tempFolder = New-TemporaryFolder

try {
    # Look for sketch files
    $sketchFiles = Get-SketchFiles

    # Compile
    foreach ($sketch in $sketchFiles) {
        Invoke-ArduinoCLI -Filename $sketch -BuildPath $tempFolder
    }
}
finally {
    # Remove temporary folder
    Remove-Item -Recurse -LiteralPath $tempFolder.FullName -Force | Out-Null
}
