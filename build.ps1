# Build script for Metaphysics 3D Renderer
# Usage: .\build.ps1 [clean]

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
    Write-Host "  2. Make sure all dependencies are installed (GLFW, GLEW, GLM)" -ForegroundColor White
    Write-Host "  3. Check the error messages above for specific issues" -ForegroundColor White
    Set-Location $projectRoot
    exit 1
}

Write-Host ""
Write-Host "=== Build Successful! ===" -ForegroundColor Green
Write-Host ""
Write-Host "Executable location:" -ForegroundColor Cyan
Write-Host "  $buildDir\bin\metaphysics.exe" -ForegroundColor White
Write-Host ""
Write-Host "To run the application:" -ForegroundColor Cyan
Write-Host "  cd $buildDir\bin" -ForegroundColor White
Write-Host "  .\metaphysics.exe" -ForegroundColor White
Write-Host ""
Write-Host "Or directly:" -ForegroundColor Cyan
Write-Host "  .\$buildDir\bin\metaphysics.exe" -ForegroundColor White
Write-Host ""

Set-Location $projectRoot
