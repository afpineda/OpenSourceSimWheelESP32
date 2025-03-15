<############################################################################

.SYNOPSYS
    Custom build system

.AUTHOR
    Angel Fernandez Pineda. Madrid. Spain. 2025.

.LICENSE
    Licensed under the EUPL

#############################################################################>

# Parameters

param (
    [Parameter(HelpMessage = "Path to project root")]
    [string]$RootPath = $null,
    [Parameter(HelpMessage = "Test name to compile. Others will be ignored")]
    [string]$TestName = $null
)

if ($RootPath.Length -eq 0) {
    $RootPath = Split-Path $($MyInvocation.MyCommand.Path) -parent
    $RootPath = Split-Path $RootPath -parent
}

# global constants
$_buildFile = "build.txt"
$_compiler = "g++"
$_arduino_includes_path = Join-Path $RootPath "/src/include"
$_cd_ci_includes_path = Join-Path $RootPath "/CD_CI/include"
$_compiler_args = @(
    "-fdiagnostics-color=always", # colored output
    "-std=c++20", # C++ standard revision 20
    "-iquote",
    $_arduino_includes_path, # Includes path
    "-iquote",
    $_cd_ci_includes_path, # Includes path
    "-c" # compile, don't link
    "-D"
    "CD_CI") # Custom define
# $_linker = "gcc"
# $_linker_args = @(
#     "-o")

# Initialization
$ErrorActionPreference = 'Stop'
$VerbosePreference = "continue"
$InformationPreference = "continue"


<#############################################################################
# Auxiliary functions
#############################################################################>

function Solve-SourceFile {
    param(
        [Parameter(Mandatory, ValueFromPipeline)]
        [string]$FileName,
        [Parameter(Mandatory)]
        [string]$CD_CI_folder,
        [Parameter(Mandatory)]
        [string]$ArduinoFolder,
        [Parameter(Mandatory)]
        [string]$MainFolder
    )
    process {
        $candidate = Join-Path $MainFolder $FileName
        if (Test-Path $candidate) {
            return (Get-ChildItem -LiteralPath $candidate)
        }
        $candidate = Join-Path $CD_CI_folder $FileName
        if (Test-Path $candidate) {
            return (Get-ChildItem -LiteralPath $candidate)
        }
        $candidate = Join-Path $ArduinoFolder $FileName
        if (Test-Path $candidate) {
            return (Get-ChildItem -LiteralPath $candidate)
        }
        throw "❌ Error. Unable to resolve path to '$FileName'"
    }
}

function Write-Work {
    param(
        [Parameter(Mandatory)]
        [string]$LiteralPath
    )
    Write-Host "🔍 Processing: " -NoNewline -ForegroundColor Green
    Write-Host $LiteralPath
}


function Write-Info {
    param (
        [string]$message
    )
    Write-Host "🛈 $message" -ForegroundColor Cyan
}

function Write-Warning {
    param (
        [string]$message
    )
    Write-Host "⚠️ $message" -ForegroundColor Yellow
}

function Write-SuccessMessage {
    Write-Host "✅ Success" -ForegroundColor Green
}

function Find-BuildFiles {
    param(
        [Parameter(Mandatory)]
        [string]$RootPath
    )
    $files = Get-ChildItem -Recurse -File -Path $RootPath
    $files | Where-Object { $_.Name.Equals($_buildFile) }
}

function Get-BuildContent {
    param (
        [Parameter(ValueFromPipeline)]
        [System.IO.FileSystemInfo]$FileObject
    )
    $LiteralPath = $FileObject.FullName
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

function Test-IsNewer {
    param (
        [Parameter(Mandatory)]
        [System.IO.FileSystemInfo]$inputFileObject,
        [Parameter(Mandatory)]
        [string]$outputFileName
    )
    if (Test-Path $outputFileName) {
        $inputTimeStamp = $inputFileObject.LastWriteTimeUtc;
        $outputFileObject = Get-ChildItem -Path $outputFileName
        $outputTimeStamp = $outputFileObject.LastWriteTimeUtc;
        return ($inputTimeStamp -gt $outputTimeStamp);
    }
    else {
        return $true
    }
}

function Invoke-Compiler {
    param (
        [Parameter(ValueFromPipeline)]
        [System.IO.FileSystemInfo]$inputFileObject
    )
    process {
        $fileName = $inputFileObject.Name
        $inputFilename = $inputFileObject.FullName
        $outputFileName = [System.IO.Path]::ChangeExtension($inputFileObject.FullName, '.o')
        if (Test-IsNewer $inputFileObject $outputFileName) {
            Write-Host "⛏ Compiling " -NoNewline -ForegroundColor Cyan
            Write-Host "$fileName "
            & $_compiler $_compiler_args -o $outputFileName $inputFilename
            if ($LASTEXITCODE -eq 0) {
                Write-SuccessMessage
            }
            else {
                throw "❌ Compiler error"
            }
        } else {
            Write-Host "🗸 Ignoring (already compiled) " -NoNewline -ForegroundColor Cyan
            Write-Host $fileName
        }
    }
}

function Get-OutputFiles {
    param (
        [Parameter(ValueFromPipeline)]
        [System.IO.FileSystemInfo]$inputFileObject
    )
    process {
        $fileName = [System.IO.Path]::ChangeExtension($inputFileObject, '.o')
        if (Test-Path $fileName) {
            Get-ChildItem -LiteralPath $fileName
        }
        else {
            throw "❌ Output file not found: ${$inputFileObject.Name}"
        }
    }
}

function Invoke-Linker {
    param (
        [Parameter(ValueFromPipelineByPropertyName)]
        [string]$FullName,
        [Parameter(Mandatory)]
        [string]$ExeFileName
    )
    begin {
        Write-Host "⛏ Linking " -NoNewline -ForegroundColor Cyan
        Write-Host "$ExeFileName"
        $fileList = @()
    }
    process {
        $fileList = $fileList + $FullName
    }
    end {
        & $_compiler -o $ExeFileName -flinker-output=exec $fileList
        if ($LASTEXITCODE -eq 0) {
            Write-SuccessMessage
        }
        else {
            throw "❌ Linker error"
        }
    }
}

function Test-IsRequired {
    param (
        [Parameter(Mandatory)]
        [string]$Folder
    )
    if ($TestName.Length -eq 0) {
        return $true
    }
    $FolderName = Split-Path $Folder -Leaf
    return $FolderName.ToLower().Equals($TestName.ToLower())
}

<#############################################################################
# MAIN
#############################################################################>

Write-Info "Root path = $RootPath"

$CD_CI_common_path = Join-Path $RootPath "CD_CI/common"
$Arduino_common_path = Join-Path $RootPath "src/common"

# Check there are common folders
if (-not (Test-Path $CD_CI_common_path)) {
    throw "❌ Error. '$CD_CI_common_path' folder not found"
}

if (-not (Test-Path $Arduino_common_path)) {
    throw "❌ Error. '$Arduino_common_path' folder not found"
}

$buildFiles = Find-BuildFiles $RootPath
foreach ($buildFile in $buildFiles) {
    $buildFolder = Split-Path $buildFile.FullName
    if (Test-IsRequired $buildFolder) {
        Write-Work $buildFolder
        $sourceFiles = Get-BuildContent $buildFile
        $sourceFiles = $sourceFiles | Solve-SourceFile -CD_CI_folder $CD_CI_common_path -ArduinoFolder $Arduino_common_path -MainFolder $buildFolder
        $sourceFiles | Invoke-Compiler
        $outputFiles = $sourceFiles | Get-OutputFiles
        $exeFileName = [System.IO.Path]::ChangeExtension($buildFile.FullName, '.exe')
        $outputFiles | Invoke-Linker -ExeFileName $exeFileName
        if ($IsLinux) {
            &chmod +x $buildFile.FullName >/dev/null
        }
    }
}