<############################################################################

.SYNOPSYS
    Get project statistics

.AUTHOR
    Angel Fernandez Pineda. Madrid. Spain. 2024.

.LICENSE
    Licensed under the EUPL

#############################################################################>

# parameters

param (
    [Parameter(HelpMessage = "Path to project root")]
    [string]$RootPath = $null
)

if ($RootPath.Length -eq 0) {
    $RootPath = $($MyInvocation.MyCommand.Path)
    $RootPath = Split-Path $RootPath -Parent
}

<#############################################################################
# Auxiliary functions
#############################################################################>

function Get-LineCount {
    param (
        [Parameter(Mandatory)]
        [string]$LiteralPath,
        [Parameter(Mandatory)]
        [string]$Filter
    )
    $files = Get-ChildItem -LiteralPath $LiteralPath -filter $Filter -Recurse -File `
    | ForEach-Object { $_.FullName }
    $totalLines = 0
    foreach ($file in $files) {
        $lc = (Get-Content $file).Count
        $totalLines += $lc
    }
    return $totalLines
}

function Get-Percentage{
    param (
        [Parameter(Mandatory)]
        [int]$Value,
        [Parameter(Mandatory)]
        [int]$Total
    )
    return [math]::ceiling(($Value*100) / $Total)
}

<#############################################################################
# Main
#############################################################################>

$ErrorActionPreference = 'Stop';

$includePath = Join-Path $RootPath "src/include"
$commonPath = Join-Path $RootPath "src/common"
$firmwarePath = Join-Path $RootPath "src/Firmware"
$cd_ci_Path = Join-Path $RootPath "CD_CI"
$qcPath = Join-Path $RootPath "src/QualityControl"

Write-Host "🛈 Project statistics: " -ForegroundColor Blue

if (-not (Test-Path $includePath)) {
    throw "❌ Error. '$includePath' folder not found"
}

if (-not (Test-Path $commonPath)) {
    throw "❌ Error. '$commonPath' folder not found"
}

if (-not (Test-Path $firmwarePath)) {
    throw "❌ Error. '$commonPath' folder not found"
}

if (-not (Test-Path $cd_ci_Path)) {
    throw "❌ Error. '$cd_ci_Path' folder not found"
}

if (-not (Test-Path $qcPath)) {
    throw "❌ Error. '$qcPath' folder not found"
}

$systemCodeLines = Get-LineCount -LiteralPath $includePath -Filter "*.?pp"
$systemCodeLines += Get-LineCount -LiteralPath $commonPath -Filter "*.?pp"

$firmwareCodeLines = Get-LineCount -LiteralPath $firmwarePath -Filter "*.ino"

$cd_ci_codeLines = Get-LineCount -LiteralPath $cd_ci_Path -Filter "*.?pp"
$cd_ci_codeLines += Get-LineCount -LiteralPath $cd_ci_Path -Filter "*.?pp"

$qc_codeLines = Get-LineCount -LiteralPath $qcPath -Filter "*.ino"

$allCodeLines = $systemCodeLines + $firmwareCodeLines + $cd_ci_codeLines + $qc_codeLines

if ($allCodeLines -le 0)
{
    throw "❌ Error. No code found."
}

$shellCodeLines = Get-LineCount -LiteralPath $RootPath -Filter "*.ps1"
$shellCodeLines += Get-LineCount -LiteralPath $RootPath -Filter "*.sh"

$docStatistics = Get-LineCount -LiteralPath $RootPath -Filter "*.md"

Write-Host "Total lines of code (*.?pp, *.ino) including commentaries: $allCodeLines (100%)"
Write-Host "- System.................................................. $systemCodeLines ($(Get-Percentage $systemCodeLines $allCodeLines)%)"
Write-Host "- Ready-to-deploy firmware................................ $firmwareCodeLines ($(Get-Percentage $firmwareCodeLines $allCodeLines)%)"
Write-Host "- Hardware-dependent tests................................ $qc_codeLines ($(Get-Percentage $qc_codeLines $allCodeLines)%)"
Write-Host "- Automated tests in the CD/CI chain...................... $cd_ci_codeLines ($(Get-Percentage $cd_ci_codeLines $allCodeLines)%)"
Write-Host "Total lines of documentation (*.md):                       $docStatistics"
Write-Host "Total lines of automation code (*.ps1, *.sh):              $shellCodeLines"
