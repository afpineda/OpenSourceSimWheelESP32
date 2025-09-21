<############################################################################

.SYNOPSYS
    Compute test coverage

.AUTHOR
    Angel Fernandez Pineda. Madrid. Spain. 2025.

.LICENSE
    Licensed under the EUPL

#############################################################################>

# Parameters

param (
    [Parameter(HelpMessage = "Path to project root")]
    [string]$RootPath = $null
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

function Find-ProfiledFiles {
    param(
        [Parameter(Mandatory)]
        [string]$RootPath
    )
    $cpp_files = Get-ChildItem -Recurse -File -Filter "*.cpp" -Path $RootPath
    foreach ($file in $cpp_files) {
        $filename = $file.Fullname
        $fn_wo = [System.IO.Path]::GetFileNameWithoutExtension($filename)
        $path = [System.IO.Path]::GetDirectoryName($filename)
        $fn = Join-Path -Path $path -ChildPath $fn_wo
        $gcda = $fn + ".gcda"
        $gcno = $fn + ".gcno"
        if ((Test-Path $gcda) -and (Test-Path $gcno)) {
            $file.Fullname
        }
    }
}

function Test-GNUCoverage {
    gcov --version | Out-Null
    if ($LASTEXITCODE -eq 0) {
        return $true
    }
    else {
        return $false
    }
}

function Invoke-GNUCoverage {
    param(
        [Parameter(Mandatory)]
        [string]$filename
    )
    $filename = $filename.Replace("\", "/")
    $output = gcov -n "$filename"
    $capture = $false
    foreach ($line in $output) {

        if ($capture) {
            Write-Host $line -ForegroundColor Gray
            $capture = $false
        }
        else {
            $capture = ($line -match "File '(.+)'") -and ($filename.Equals($Matches.1))
        }
    }
}

<#############################################################################
# MAIN
#############################################################################>

try {
    if (-not (Test-GNUCoverage)) {
        throw "GNU code coverage tool (gcov) not found"
    }
    $commonPath = Join-Path $RootPath "src/common"
    if (-not (Test-Path $commonPath)) {
        throw "Project folder not found"
    }

    foreach ($cpp in Find-ProfiledFiles($commonPath)) {
        Write-Host "ℹ️  Test coverage: " -NoNewline -ForegroundColor Green
        Write-Host $([System.IO.Path]::GetFileName($cpp))
        Invoke-GNUCoverage $cpp
    }
}
catch {
    # Do not fail
    Write-Host "⚠️  Test coverage not computed: " -ForegroundColor Red -NoNewline
    Write-Host $_
}