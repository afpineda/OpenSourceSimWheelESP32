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
    Angel Fernandez Pineda. Madrid. Spain. 2024.

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
        [Parameter(Mandatory)]
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

function Add-FQBN {
    param(
        [Parameter(Mandatory, ValueFromPipeline)]
        [string]$LiteralPath
    )
    process {
        $path = Split-Path $LiteralPath
        $fqbn = Find-FQBN $path
        if ($fqbn.Length -gt 0) {
            @{ FQBN = $fqbn; Sketch = $LiteralPath }
        }
    }
}

function Invoke-ArduinoCLI {
    param(
        [Parameter(Mandatory)]
        [string]$Filename,
        [Parameter(Mandatory)]
        [string]$FQBN,
        [Parameter(Mandatory)]
        [string]$BuildPath
    )
    $sketch_folder = Split-Path -Parent $Filename
    $fqbn = Find-FQBN -LiteralPath $sketch_folder
    $ErrorActionPreference = 'Continue'
    &$_compiler compile $Filename -b $fqbn --no-color --warnings all --build-path $BuildPath # 2>&1
    $ErrorActionPreference = 'Stop'
}

function New-TemporaryFolder {
    $File = New-TemporaryFile
    Remove-Item $File -Force | Out-Null
    $tempFolderName = Join-Path $ENV:Temp $File.Name
    $Folder = New-Item -Itemtype Directory -Path $tempFolderName
    return $Folder
}

function Write-SketchInfo {
    param(
        [Parameter(Mandatory)]
        [string]$LiteralPath
    )
    $filename = Split-Path $LiteralPath -leaf
    Write-Host "======================================================================" -ForegroundColor Yellow -BackgroundColor Black
    Write-Host "⛏ " -NoNewline -ForegroundColor Magenta
    Write-Host "Compiling " -NoNewline  -ForegroundColor Green
    Write-Host $filename -NoNewline
    Write-Host " 🛈 " -NoNewline -ForegroundColor Magenta
    Write-Host $item.FQBN
    Write-Host "======================================================================" -ForegroundColor Yellow -BackgroundColor Black
}

function Write-FailureMessage {
    Write-Host "❌ Error " -ForegroundColor Red
}

function Write-SuccessMessage {
    Write-Host "✅ Success" -ForegroundColor Green
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

# Look for sketch files
$plan = Get-SketchFiles | Add-FQBN | Sort-Object { $_.FQBN }

# Create a temporary folder (will speed up compilation)
$tempFolder = New-TemporaryFolder

Clear-Host
Write-Host " 🛈 Temp folder: " -NoNewline -ForegroundColor Magenta
Write-Host $tempFolder
try {
    # Compile
    foreach ($item in $plan) {
        # Write header
        Write-SketchInfo -LiteralPath $item.Sketch
        Invoke-ArduinoCLI -Filename $item.Sketch -FQBN $item.FQBN -BuildPath $tempFolder
        if ($LASTEXITCODE -gt 0) {
            Write-FailureMessage
        }
        else {
            Write-SuccessMessage
        }
    }
}
finally {
    # Remove temporary folder
    Remove-Item -Recurse -LiteralPath $tempFolder.FullName -Force | Out-Null
}
