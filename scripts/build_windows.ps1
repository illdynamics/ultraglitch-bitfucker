# build_windows.ps1 — UltraGlitch BitFucker Windows build script
# Requires: Visual Studio 2022 with C++ workload, CMake 3.22+, Git
#
# Usage:
#   .\scripts\build_windows.ps1
#
# Output:
#   dist\windows-x86_64\UltraGlitch BitFucker.vst3\  (VST3 bundle)
#   dist\windows-x86_64\UltraGlitch BitFucker.exe     (Standalone)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$BuildDir = Join-Path $ProjectRoot "build-win"
$DistDir = Join-Path $ProjectRoot "dist\windows-x86_64"

Write-Host "=== UltraGlitch BitFucker — Windows x64 Build ===" -ForegroundColor Cyan
Write-Host "Project root: $ProjectRoot"
Write-Host "Build dir:    $BuildDir"
Write-Host "Dist dir:     $DistDir"

# ── Configure ────────────────────────────────────────────────────────────────
Write-Host "`n=== Configuring (Visual Studio 2022, x64) ===" -ForegroundColor Yellow
cmake -S $ProjectRoot -B $BuildDir -G "Visual Studio 17 2022" -A x64
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }

# ── Build ────────────────────────────────────────────────────────────────────
Write-Host "`n=== Building Release ===" -ForegroundColor Yellow
cmake --build $BuildDir --config Release
if ($LASTEXITCODE -ne 0) { throw "CMake build failed" }

# ── Collect artefacts ────────────────────────────────────────────────────────
Write-Host "`n=== Collecting artefacts ===" -ForegroundColor Yellow

$ArtefactsBase = Join-Path $BuildDir "UltraGlitch_artefacts\Release"
$VST3Source = Join-Path $ArtefactsBase "VST3"
$StandaloneSource = Join-Path $ArtefactsBase "Standalone"

# Create dist directory
New-Item -ItemType Directory -Force -Path $DistDir | Out-Null

# Copy VST3 bundle
$VST3Bundle = Get-ChildItem -Path $VST3Source -Filter "*.vst3" -Directory -ErrorAction SilentlyContinue
if ($VST3Bundle) {
    Write-Host "  Copying VST3: $($VST3Bundle.Name)"
    Copy-Item -Recurse -Force $VST3Bundle.FullName $DistDir
} else {
    Write-Warning "VST3 bundle not found in $VST3Source"
}

# Copy Standalone exe
$StandaloneExe = Get-ChildItem -Path $StandaloneSource -Filter "*.exe" -ErrorAction SilentlyContinue
if ($StandaloneExe) {
    Write-Host "  Copying Standalone: $($StandaloneExe.Name)"
    Copy-Item -Force $StandaloneExe.FullName $DistDir
} else {
    Write-Warning "Standalone exe not found in $StandaloneSource"
}

# ── Verify ───────────────────────────────────────────────────────────────────
Write-Host "`n=== Verification ===" -ForegroundColor Green
$vst3Exists = Test-Path (Join-Path $DistDir "UltraGlitch BitFucker.vst3")
$exeExists = Test-Path (Join-Path $DistDir "UltraGlitch BitFucker.exe")

if ($vst3Exists) { Write-Host "  ✅ VST3 bundle present" } else { Write-Host "  ❌ VST3 bundle MISSING" -ForegroundColor Red }
if ($exeExists) { Write-Host "  ✅ Standalone exe present" } else { Write-Host "  ❌ Standalone exe MISSING" -ForegroundColor Red }

if ($vst3Exists -and $exeExists) {
    Write-Host "`n🎉 Windows build complete! Artefacts in: $DistDir" -ForegroundColor Green
} else {
    throw "Build verification failed — missing artefacts"
}
