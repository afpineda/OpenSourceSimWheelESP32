<############################################################################

.SYNOPSYS
    Reveal broken links in markdown files (to local files only)

.AUTHOR
    Angel Fernandez Pineda. Madrid. Spain. 2025.

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

$_link_regexp = "\[.+?\]\((?<link>[^\(\)]+?)[\)|#]"
$_link_pattern = [Regex]::new($_link_regexp)

<#############################################################################
# Auxiliary functions
#############################################################################>

function Get-Hyperlinks {
    param (
        [Parameter(Mandatory)]
        [System.IO.FileSystemInfo]$inputFileObject
    )
    $filePath = $inputFileObject.Directory.FullName
    $content = Get-Content -LiteralPath $inputFileObject.FullName
    foreach ($line in $content) {
        $pattern_matches = $_link_pattern.Matches($line)
        foreach ($match in $pattern_matches) {
            $link = $match.Groups[1].Value
            if ($link -notlike "http*") {
                if ([System.IO.Path]::IsPathRooted($link)) {
                    $link
                }
                else {
                    Join-Path $filePath $link
                }
            }
        }
    }
}

<#############################################################################
# MAIN
#############################################################################>

Write-Host "🛈 " -ForegroundColor Green -NoNewline
Write-Host "Looking for broken links in markdown files" -ForegroundColor Cyan

$candidates = Get-ChildItem -LiteralPath $RootPath -filter "*.md" -Recurse
$error_count = 0
foreach ($markdown in $candidates) {
    $links = Get-Hyperlinks $markdown
    $showFile = $true
    foreach ($link in $links) {
        $found = Test-Path $link
        if (-not $found) {
            if ($showFile) {
                Write-Host "In file " -NoNewline -ForegroundColor Green
                Write-Host $markdown.FullName
                $showFile = $false
            }
            Write-Host "Broken link to: " -NoNewline -ForegroundColor Red
            Write-Host $link
            $error_count++
        }
    }
}
if ($error_count -gt 0) {
    throw "❌ Broken links found"
}
else {
    Write-Host "✅ Success" -ForegroundColor Green
}
