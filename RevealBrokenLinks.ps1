<############################################################################

.SYNOPSYS
    Reveal broken links in markdown files to local files and,
    optionally, to external http(s) resources.

.AUTHOR
    Angel Fernandez Pineda. Madrid. Spain. 2025.

.LICENSE
    Licensed under the EUPL

.PARAM RootPath
    Path to scan recursively for markdown files.

.PARAM ExternalLinks
    Whether to check or not external links to http/https resources.
    It takes long.
    Reported broken links must be checked manually
    as this is not 100% accurate.
    Do not use in the CD/CI pipeline.

#############################################################################>

# parameters

param (
    [Parameter(HelpMessage = "Path to project root")]
    [string]$RootPath = $null,
    [switch]$ExternalLinks
)

if ($RootPath.Length -eq 0) {
    $RootPath = $($MyInvocation.MyCommand.Path)
    $RootPath = Split-Path $RootPath -Parent
}

$_link_regexp = "\[.+?\]\((?<link>[^\(\)]+?)[\)|#]"
$_link_pattern = [Regex]::new($_link_regexp)
$_http_regexp = "http[s]?:\/\/.+"

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
            if ($link -match $_http_regexp) {
                if ($ExternalLinks) { $link }
            }
            elseif ($link -notlike "#*") {
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

function Get-UrlStatusCode {
    param (
        [Parameter(Mandatory)]
        [string] $Url
    )
    try {
        (Invoke-WebRequest -Uri $Url -UseBasicParsing -DisableKeepAlive -TimeoutSec 5).StatusCode
    }
    catch [Net.WebException] {
        if ($_.Exception.Status -eq "ConnectionClosed") {
            return 200
        }
        else {
            [int]$_.Exception.Response.StatusCode
        }
    }
}

function Test-Hyperlink {
    param (
        [Parameter(Mandatory)]
        [string]$link
    )
    if ($link -match $_http_regexp) {
        # External link
        $status = Get-UrlStatusCode $link
        return ($status -ge 200) -and `
        (($status -lt 400) -or ($status -eq 401) -or ($status -eq 403))
    }
    else {
        # Internal link
        return Test-Path $link
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
        $found = Test-Hyperlink $link
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
