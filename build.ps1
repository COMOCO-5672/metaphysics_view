# Build script for Metaphysics 3D Renderer (Windows)
#
# What it does:
#   - Creates `build/` if missing, runs `cmake ..` when needed, then `cmake --build .`
#   - First configure compiles bundled GLFW, GLEW (from framework/), Assimp (from metaphysics_deps/), and the app
#
# Usage:
#   .\build.ps1              # incremental build
#   .\build.ps1 -Clean       # delete build/, then full reconfigure + build

param(
    [switch]$Clean = $false
)

Write-Host "=== Metaphysics 3D Renderer Build Script ===" -ForegroundColor Cyan
Write-Host ""

$buildDir = "build"
$projectRoot = $PSScriptRoot

# Clean build if requested
if ($Clean) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    if (Test-Path $buildDir) {
        Remove-Item -Recurse -Force $buildDir
    }
    Write-Host "Clean complete." -ForegroundColor Green
    Write-Host ""
}

# Check if build directory exists
if (!(Test-Path $buildDir)) {
    Write-Host "Creating build directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

Set-Location $buildDir

# Check if reconfiguration is needed
$needsReconfigure = $Clean -or !(Test-Path "CMakeCache.txt")

if ($needsReconfigure) {
    Write-Host "Configuring CMake..." -ForegroundColor Yellow
    cmake ..
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "CMake configuration failed!" -ForegroundColor Red
        Set-Location $projectRoot
        exit 1
    }
}

# Build
Write-Host ""
Write-Host "Building project (this may take a few minutes on first build)..." -ForegroundColor Yellow
Write-Host "Tip: Assimp is being compiled, please be patient..." -ForegroundColor Gray
cmake --build .

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "Build failed!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Common solutions:" -ForegroundColor Yellow
    Write-Host "  1. Try: .\build.ps1 -Clean" -ForegroundColor White
    Write-Host "  2. Install CMake + a C++ toolchain (Visual Studio Build Tools is enough). GLFW/GLM/GLEW/Assimp are bundled or auto-found." -ForegroundColor White
    Write-Host "  3. Check the error messages above for specific issues" -ForegroundColor White
    Set-Location $projectRoot
    exit 1
}

Write-Host ""
Write-Host "=== Build Successful! ===" -ForegroundColor Green
Write-Host ""

$exeDebug = Join-Path $projectRoot "$buildDir\bin\Debug\metaphysics.exe"
$exeRelease = Join-Path $projectRoot "$buildDir\bin\Release\metaphysics.exe"
$exeFlat = Join-Path $projectRoot "$buildDir\bin\metaphysics.exe"
$exePath = $null
if (Test-Path $exeDebug) { $exePath = $exeDebug }
elseif (Test-Path $exeRelease) { $exePath = $exeRelease }
elseif (Test-Path $exeFlat) { $exePath = $exeFlat }

Write-Host "Executable:" -ForegroundColor Cyan
if ($exePath) {
    Write-Host "  $exePath" -ForegroundColor White
    $exeDir = Split-Path $exePath -Parent
    Write-Host ""
    Write-Host "Run from its folder (so DLLs resolve):" -ForegroundColor Cyan
    Write-Host "  cd `"$exeDir`"" -ForegroundColor White
    Write-Host "  .\metaphysics.exe" -ForegroundColor White
} else {
    Write-Host "  (not found under $buildDir\bin — check CMake generator / config)" -ForegroundColor Yellow
    Write-Host "  Typical: $buildDir\bin\Debug\metaphysics.exe (Visual Studio)" -ForegroundColor White
    Write-Host "  Or:      $buildDir\bin\metaphysics.exe (Ninja / single-config)" -ForegroundColor White
}
Write-Host ""

Set-Location $projectRoot
