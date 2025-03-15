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

<#############################################################################
# Main
#############################################################################>

$includePath = Join-Path $RootPath "src/include"
$commonPath = Join-Path $RootPath "src/common"
$firmwarePath = Join-Path $RootPath "src/Firmware"

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

$allCodeLines = Get-LineCount -LiteralPath $RootPath -Filter "*.?pp"
$allCodeLines += Get-LineCount -LiteralPath $RootPath -Filter "*.ino"

if ($allCodeLines -lt 1) {
    Write-Host "⚠️ No code found"
    exit 0
}

$systemCodeLines = Get-LineCount -LiteralPath $includePath -Filter "*.?pp"
$systemCodeLines += Get-LineCount -LiteralPath $commonPath -Filter "*.?pp"

$firmwareCodeLines = Get-LineCount -LiteralPath $firmwarePath -Filter "*.ino"

$qcCodeLines = $allCodeLines - ($systemCodeLines + $firmwareCodeLines)

$shellCodeLines = Get-LineCount -LiteralPath $RootPath -Filter "*.ps1"
$shellCodeLines += Get-LineCount -LiteralPath $RootPath -Filter "*.sh"

$docStatistics = Get-LineCount -LiteralPath $RootPath -Filter "*.md"

$firmwarePercentage = [math]::ceiling(($firmwareCodeLines*100) / $allCodeLines)
$systemPercentage = [math]::ceiling(($systemCodeLines*100) / $allCodeLines)
$qcPercentage = 100 - ($firmwarePercentage + $systemPercentage)

Write-Host "Total lines of code (*.?pp, *.ino) including commentaries: $allCodeLines (100%)"
Write-Host "- System: $systemCodeLines ($($systemPercentage)%)"
Write-Host "- Ready-to-deploy firmware: $firmwareCodeLines ($($firmwarePercentage)%)"
Write-Host "- Quality control: $qcCodeLines ($($qcPercentage)%)"
Write-Host "Total lines of documentation (*.md): $docStatistics"
Write-Host "Total lines of automation code (*.ps1, *.sh): $shellCodeLines"
