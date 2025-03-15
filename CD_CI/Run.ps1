<############################################################################

.SYNOPSYS
    Run test executables

.AUTHOR
    Angel Fernandez Pineda. Madrid. Spain. 2025.

.LICENSE
    Licensed under the EUPL

#############################################################################>

# Parameters

param (
    [Parameter(HelpMessage = "Path to project root")]
    [string]$RootPath = $null,
    [Parameter(HelpMessage = "Test name to run. Others will be ignored")]
    [string]$TestName = $null

)

if ($RootPath.Length -eq 0) {
    $RootPath = Split-Path $($MyInvocation.MyCommand.Path) -parent
    $RootPath = Split-Path $RootPath -parent
}

# global constants

# Initialization
$ErrorActionPreference = 'Stop'
$VerbosePreference = "continue"
$InformationPreference = "continue"


<#############################################################################
# Auxiliary functions
#############################################################################>

function Write-Work {
    param(
        [Parameter(Mandatory)]
        [string]$LiteralPath
    )
    Write-Host "⛏ Running: " -NoNewline -ForegroundColor Green
    Write-Host $LiteralPath
    Write-Host "======================================================================" -ForegroundColor Yellow -BackgroundColor Black
}

function Write-Info {
    param (
        [string]$message
    )
    Write-Host "🛈 $message" -ForegroundColor Cyan
}

function Write-SuccessMessage {
    Write-Host "✅ Success" -ForegroundColor Green
}


function Find-ExeFiles {
    param(
        [Parameter(Mandatory)]
        [string]$RootPath
    )
    Get-ChildItem -Recurse -File -Filter "build.exe" -Path $RootPath
}

function Find-ExeTitle {
    param(
        [Parameter(Mandatory)]
        [System.IO.FileSystemInfo]$ExeFile
    )
    $folder = $ExeFile.Directory.FullName
    $titleFile = Join-Path $folder "title.txt"
    if (Test-Path $titleFile) {
        return Get-Content -Path $titleFile -TotalCount 1
    }
    else {
        return $ExeFile.Directory.Name
    }
}

function Invoke-ExeFile {
    param(
        [Parameter(Mandatory)]
        [System.IO.FileSystemInfo]$ExeFile
    )
    $fullName = $ExeFile.FullName
    & $fullName
    if ($LASTEXITCODE -eq 0) {
        Write-SuccessMessage
    }
    else {
        throw "❌ Test failed"
    }
}

function Test-IsRequired {
    param (
        [Parameter(Mandatory)]
        [System.IO.FileSystemInfo]$ExeFile
    )
    if ($TestName.Length -eq 0) {
        return $true
    }
    $FolderName = $ExeFile.Directory.Name
    return $FolderName.ToLower().Equals($TestName.ToLower())
}

<#############################################################################
# MAIN
#############################################################################>

Write-Info "Root path = $RootPath"

$CD_CI_path = Join-Path $RootPath "CD_CI"
$exeFiles = Find-ExeFiles $CD_CI_path

foreach ($exe in $exeFiles) {
    if (Test-IsRequired $exe) {
        $title = Find-ExeTitle $exe
        Write-Work $title
        Invoke-ExeFile $exe
    }
}