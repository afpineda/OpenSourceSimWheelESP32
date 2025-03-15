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

# parameters

param (
    [Parameter(HelpMessage = "Path to project root")]
    [string]$RootPath = $null,
    [Parameter(HelpMessage = "Sketch name to compile. Others will be ignored")]
    [string]$SketchName = $null
)

if ($RootPath.Length -eq 0) {
    $RootPath = Split-Path $($MyInvocation.MyCommand.Path) -parent
    $RootPath = Split-Path $RootPath -parent
}

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
    if ($SketchName.Length -eq 0) {
        $name_ext = '*.ino'
    } else {
        if ($SketchName.EndsWith('.ino')) {
            $name_ext = $SketchName
        } else {
            $name_ext = $SketchName + '.ino'
        }
    }
    Get-ChildItem -Recurse -Filter $name_ext -LiteralPath $RootPath | ForEach-Object { $_.FullName }
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
        } else {
            Write-Host "⚠️ Ignoring (no FQBN file): " -NoNewline -ForegroundColor Blue
            Write-Host $LiteralPath
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
    &$_compiler compile $Filename -b $fqbn --no-color --warnings all --build-path $BuildPath # 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-SuccessMessage
    }
    else {
        throw "❌ Compiler error"
    }
}

function New-TemporaryFolder {
    $File = New-TemporaryFile
    Remove-Item $File -Force | Out-Null
    $tempDir = [IO.Path]::GetTempPath()
    $tempFolderName = Join-Path $tempDir $File.Name
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
    Write-Host "⛏ Compiling " -NoNewline -ForegroundColor Blue
    Write-Host $filename -NoNewline
    Write-Host " 🛈 " -NoNewline -ForegroundColor Blue
    Write-Host $item.FQBN
    Write-Host "======================================================================" -ForegroundColor Yellow -BackgroundColor Black
}

# function Write-FailureMessage {
#     Write-Host "❌ Error " -ForegroundColor Red
# }

function Write-SuccessMessage {
    Write-Host "✅ Success" -ForegroundColor Green
}

function Write-Info {
    param (
        [string]$message
    )
    Write-Host "🛈 $message" -ForegroundColor Cyan
}

## Compile command example:
# &$_compiler compile $sketch -b esp32:esp32:esp32s3 --no-color

<#############################################################################
# MAIN
#############################################################################>

Write-Info "Root path = $RootPath"

# Initialization
$Arduino_common_path = Join-Path $RootPath "src/common"
$VerbosePreference = "continue"
$InformationPreference = "continue"
$ErrorActionPreference = 'Stop'

Test-ArduinoCLI

if (-not (Test-Path $Arduino_common_path)) {
    throw "❌ Error. '$Arduino_common_path' folder not found"
}

# Look for sketch files
$plan = Get-SketchFiles | Add-FQBN | Sort-Object { $_.FQBN }

# Create a temporary folder (will speed up compilation)
$tempFolder = New-TemporaryFolder
Write-Host "🛈 Temp folder = " -NoNewline -ForegroundColor Blue
Write-Host $tempFolder

$stopWatch = [System.Diagnostics.Stopwatch]::startNew();
try {
    # Compile
    foreach ($item in $plan) {
        # Write header
        Write-SketchInfo -LiteralPath $item.Sketch
        Invoke-ArduinoCLI -Filename $item.Sketch -FQBN $item.FQBN -BuildPath $tempFolder
    }
}
finally {
    # Print footer
    Write-Host "======================================================================" -ForegroundColor Yellow -BackgroundColor Black
    $stopWatch.Stop()
    $timeStr = "{0,2:d2}:{1,2:d2}:{2,2:d2}" -f $stopWatch.Elapsed.Hours, $stopWatch.Elapsed.Minutes, $stopWatch.Elapsed.Seconds
    Write-Host "🛈 Run time (hh:mm:ss) = " -NoNewline -ForegroundColor Blue
    Write-Host $timeStr

    # Remove temporary folder
    Remove-Item -Recurse -LiteralPath $tempFolder.FullName -Force | Out-Null
}