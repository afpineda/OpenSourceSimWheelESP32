<############################################################################

.SYNOPSYS
    Copy or link required files from "common" and "include" folders into
    every Arduino's sketch folder for proper compilation

.USAGE
    Create a file named "includes.txt" into the sketch's folder.
    Insert every required file name into "includes.txt",
    relative to the "src" folder.
    Header files in "include" (.h/.hpp) are always linked/copied,
    so there is no need to include them.
    If ran with administrator privileges, symlinks will be created. If not,
    files will be copied, instead. In such a case, this script must be run
    again every time a file is touched at the "common" or "include" folders.

.AUTHOR
    Ángel Fernández Pineda. Madrid. Spain. 2022.

.LICENSE
    Licensed under the EUPL

#############################################################################>

# Parameters

param (
    [Parameter(HelpMessage = "Does not perform any effective action")]
    [switch]$Test = $false,
    [Parameter(HelpMessage = "Path to the 'src' folder of this project")]
    [string]$RootPath = $null
)

# global constants
$_includesFile = "includes.txt"

# Initialize
$ErrorActionPreference = 'Stop'
$VerbosePreference = "continue"
$InformationPreference = "continue"

if ($RootPath.Length -eq 0) {
    $RootPath = Split-Path $($MyInvocation.MyCommand.Path) -Parent
    $RootPath = Split-Path $RootPath -Parent
}

<#############################################################################
# Auxiliary functions
#############################################################################>

function Find-Includes {
    param(
        [Parameter(Mandatory)]
        [string]$RootPath
    )
    $files = Get-ChildItem -Recurse -File -Path $RootPath | Where-Object { $_.Name -eq $_includesFile }
    $files | ForEach-Object { $_.FullName }
}

function Find-HeaderFiles {
    param(
        [Parameter(Mandatory)]
        [string]$RootPath
    )
    $files = Get-ChildItem -Recurse -File -Path $RootPath -Filter *.hpp
    $files | ForEach-Object { $_.FullName }
}

function Set-LinkFilename {
    param (
        [Parameter(Mandatory)]
        [string]$SourceFullName,
        [Parameter(Mandatory)]
        [string]$DestinationFolder
    )
    $filename = Split-Path $SourceFullName -Leaf
    Join-Path $DestinationFolder $filename
}

function Write-ItemInfo {
    param(
        [Parameter(Mandatory)]
        [string]$LiteralPath
    )
    Write-Host "======================================================================" -ForegroundColor Yellow -BackgroundColor Black
    Write-Host "⛏ Processing path: " -NoNewline -ForegroundColor Cyan
    Write-Host $LiteralPath
}

function Write-FailureMessage {
    Write-Host "❌ Error " -ForegroundColor Red
}

function Write-SuccessMessage {
    Write-Host "✅ Success" -ForegroundColor Green
}

function Write-Warning {
    param (
        [string]$message
    )
    Write-Host "⚠️ $message" -ForegroundColor Yellow
}

function Write-Info {
    param (
        [string]$message
    )
    Write-Host "🛈 $message" -ForegroundColor Cyan
}

function Write-ForTesting {
    param (
        [string]$message
    )
    Write-Host $message -ForegroundColor DarkCyan
}

function New-SymLink {
    param (
        [Parameter(Mandatory)]
        [string] $ExistingFile,
        [Parameter(Mandatory)]
        [string] $LinkFile
    )
    try {
        # Create links
        New-Item -ItemType SymbolicLink -Path $LinkFile -Target $ExistingFile | Out-Null
    }
    catch [UnauthorizedAccessException] {
        # No admin privileges, copy files instead
        Copy-Item $ExistingFile -Destination $LinkFile -Force
        # Set this copy to read-only in order to prevent any confussion
        # when editing files
        Set-ItemProperty -Path $LinkFile -Name IsReadOnly -Value $true
    }
    catch [System.Management.Automation.ItemNotFoundException] {
        Write-Warning "$ExistingFile not found"
    }
}


function Get-IncludesFileContent {
    param (
        [Parameter(Mandatory)]
        [string]$LiteralPath
    )
    Get-Content $LiteralPath | ForEach-Object {
        $l = $_.Trim()
        if ($l.length -gt 0) {
            $ext = [System.IO.Path]::GetExtension($l).ToLower()
            if ([System.IO.Path]::IsPathRooted($l)) {
                Write-Warning "Ignoring absolute file name '$l'"
            }
            elseif ($ext.Equals(".hpp") -or $ext.Equals(".h")) {
                Write-Warning "Ignoring header file '$l'"
            }
            else {
                $l
            }
        }
    }
}

<#############################################################################
# MAIN
#############################################################################>

if ($Test) {
    Write-Host "🛈 Test mode" -ForegroundColor Cyan
}
Write-Host "🛈 Root path = " -NoNewline -ForegroundColor Cyan
Write-Host $RootPath

try {

    # Check there are common and include folders
    $commonPath = Join-Path $RootPath "src/common"
    $includePath = Join-Path $RootPath "src/include"

    if (-not (Test-Path $includePath)) {
        throw "❌ Error. 'include' folder not found"
    }

    if (-not (Test-Path $commonPath)) {
        throw "❌ Error. 'common' folder not found"
    }

    # Retrieve file names in the "include" folder
    $headerFiles = Find-HeaderFiles $includePath
    if ($headerFiles.Length -eq 0) {
        throw "❌ Error. 'include' folder is empty"
    }

    $spec_files = Find-Includes -RootPath $RootPath
    foreach ($specFile in $spec_files) {
        $specFolder = Split-Path $specFile
        Write-ItemInfo $specFolder

        Write-Info "Deleting previous links"
        $cpp = Join-Path $specFolder "*.cpp"
        $h = Join-Path $specFolder "*.h"
        $hpp = Join-Path $specFolder "*.hpp"
        if (-not $Test) {
            Remove-Item $cpp -Force
            Remove-Item $h -Force
            Remove-Item $hpp -Force
        }

        Write-Info "Creating links to header files"
        foreach ($headerFile in $headerFiles) {
            $destinationFile = Set-LinkFilename -SourceFullName $headerFile -DestinationFolder $specFolder
            if ($Test) {
                Write-ForTesting "$headerFile ⇒ $destinationFile"
            }
            else {
                New-SymLink -ExistingFile $headerFile -LinkFile $destinationFile
            }
        } # foreach ($headerFile ...

        Write-Info "Creating links to filenames found in '$_includesFile'"
        $includesContent = Get-IncludesFileContent -LiteralPath $specFile
        if ($includesContent.Length -eq 0) {
            Write-Warning "'$_includesFile' is empty"
        }
        foreach ($includedFile in $includesContent) {
            $sourceFile = Join-Path $commonPath $includedFile
            $destinationFile = Set-LinkFilename -SourceFullName $sourceFile -DestinationFolder $specFolder
            if (Test-Path $sourceFile) {
                if ($Test) {
                    Write-ForTesting "$sourceFile ⇒ $destinationFile"
                }
                else {
                    New-SymLink -ExistingFile $sourceFile -LinkFile $destinationFile
                } # if (Test-Path $sourceFile)
            }
            else {
                Write-Error "❌ ERROR. Non-existing file: '$sourceFile'"
            }
        }
    } # foreach ($includedFile
    Write-Host "======================================================================" -ForegroundColor Yellow -BackgroundColor Black
} # foreach ($specFile ...

finally {
}
