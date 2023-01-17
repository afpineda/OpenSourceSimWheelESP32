<############################################################################

.SYNOPSYS
    Copy or link required files from "common" and "include" folders into 
    every Arduino's sketch folder for proper compilation

.USAGE
    Create a file named "includes.txt" into the sketch's folder.
    Insert every required file name into "includes.txt", name and extension
    only, one into each line.
    If ran with administrator privileges, symlinks will be created. If not,
    files will be copied, instead. In such a case, this script must be run
    again every time a file is touched at the "common" or "include" folders.

.AUTHOR
    Ángel Fernández Pineda. Madrid. Spain. 2022.

.LICENSE
    Creative Commons Attribution 4.0 International (CC BY 4.0)
    
#############################################################################>

#setup
$ErrorActionPreference = 'Stop'

$thisPath = Split-Path $($MyInvocation.MyCommand.Path) -parent
cd $thispath

# global constants
$_includePath = (Resolve-Path ".\include").Path
$_commonPath = (Resolve-Path ".\common").Path
$_includesFile = "includes.txt"

# auxiliary functions

function Get-RequiredLinks
{
    param (
        [string]$path
    )
    $filename = Join-Path $path $_includesFile
    Get-Content $filename |% {
        $l = $_.Trim()
        if ($l.length -gt 0) {
            if ($l -like "*.h") {
                @{ target= $l
                   source= (Join-Path $_includePath $l)}
            } else {
                @{ target= $l
                   source= (Join-Path $_commonPath $l)}
            }
        }
    }
}

function Get-SketchFolders
{
    Get-ChildItem -Recurse -Filter *.ino |% { $_.FullName }
}

## MAIN

$VerbosePreference = "continue"
$InformationPreference  = "continue"

Write-Information $thisPath

# Look for sketch folders
$sketchFolders = Get-ChildItem -Recurse -Filter *.ino |% { Split-Path -Path $_.FullName }

# Exclude those where no "includes" file can be found
$sketchFolders = $sketchFolders |% {
    $inc = Join-Path $_ $_includesFile
    if (Test-Path $inc) { $_ }
}

foreach ($folder in $sketchFolders) {
    Write-Information $folder
    # Read required links from "includes.txt"
    $reqLinks = try {
        Get-RequiredLinks $folder
    } catch {
        Write-Warning "$_includesFile not readable at $folder"
        @() 
    }

    #delete previous CPP and H files
    $cpp = Join-Path $folder "*.cpp"
    $h = Join-Path $folder "*.h"
    #Write-Host "DEL $cpp"
    Remove-Item $cpp -Force
    Remove-Item $h -Force

    foreach ($linkSpec in $reqLinks) {
        $target = Join-Path $folder $linkSpec.target
        try {
            # Write-Host "$($linkSpec.source) <== $target"
            # Create links
            New-Item -ItemType SymbolicLink -Path $target -Target $linkSpec.source |Out-Null
        } catch [UnauthorizedAccessException] {
            # No admin privileges, copy files instead
            Copy-Item $linkSpec.source -Destination $target -Force 
        } catch [System.Management.Automation.ItemNotFoundException] {
           Write-Warning "$($linkSpec.source) not found" 
        }
    }
}