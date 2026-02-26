# package_release.ps1 — Package UltraGlitch BitFucker release zip (Windows)
# Usage: .\scripts\package_release.ps1
# Produces: v0.4.0-beta.zip in the project root

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Version = (Get-Content (Join-Path $ProjectRoot "VERSION")).Trim()
$ZipName = "$Version.zip"
$ZipPath = Join-Path $ProjectRoot $ZipName

Write-Host "=== Packaging UltraGlitch BitFucker $Version ===" -ForegroundColor Cyan

if (Test-Path $ZipPath) { Remove-Item $ZipPath }

# Compress — exclude build dirs, .git, and other large intermediates
$exclude = @("build", "build-win", "build-mac", ".git", "__MACOSX", "_deps", "*.zip")

Push-Location $ProjectRoot
$items = Get-ChildItem -Force | Where-Object {
    $name = $_.Name
    -not ($exclude | Where-Object { $name -like $_ })
}

Compress-Archive -Path $items.FullName -DestinationPath $ZipPath -Force
Pop-Location

Write-Host "`nRelease packaged: $ZipPath" -ForegroundColor Green
Write-Host "Size: $([math]::Round((Get-Item $ZipPath).Length / 1MB, 2)) MB"
