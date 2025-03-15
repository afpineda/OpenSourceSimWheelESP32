<############################################################################

.SYNOPSYS
    Delete build artifacts

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

# Initialization
$ErrorActionPreference = 'Stop'
$VerbosePreference = "continue"
$InformationPreference = "continue"

<#############################################################################
# MAIN
#############################################################################>

$toDelete = Get-ChildItem -Recurse -Filter "*.exe" -Path $RootPath
$toDelete | ForEach-Object { $_.Delete() }
$toDelete = Get-ChildItem -Recurse -Filter "*.o" -Path $RootPath
$toDelete | ForEach-Object { $_.Delete() }
