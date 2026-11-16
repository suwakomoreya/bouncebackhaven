# Build Bounce Back Haven as a WebAssembly game for itch.io (HTML5).
#
# Usage (from the project folder):
#   powershell -ExecutionPolicy Bypass -File .\build-web.ps1
#
# The first run installs Emscripten into %USERPROFILE%\emsdk and builds a web
# copy of raylib into .deps\raylib. Later runs reuse both.
#
# Output:
#   dist\web\index.html  (+ index.js / index.wasm / index.data)
#   dist\bouncebackhaven-web.zip  <- upload this to itch.io

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$Root       = $PSScriptRoot
$DepsDir    = Join-Path $Root '.deps'
$RaylibDir  = Join-Path $DepsDir 'raylib'
$RaylibSrc  = Join-Path $RaylibDir 'src'
$RaylibLib  = Join-Path $RaylibSrc 'libraylib.web.a'
$OutDir     = Join-Path $Root 'dist\web'
$ZipPath    = Join-Path $Root 'dist\bouncebackhaven-web.zip'
$RaylibTag  = '6.2'

function Write-Step([string]$Text) {
    Write-Host ""
    Write-Host "==> $Text" -ForegroundColor Cyan
}

function Fail([string]$Text) {
    Write-Host ""
    Write-Host "[error] $Text" -ForegroundColor Red
    exit 1
}

function Find-Emsdk {
    $candidates = @()
    if ($env:EMSDK) { $candidates += $env:EMSDK }
    $candidates += (Join-Path $HOME 'emsdk')
    $candidates += 'C:\emsdk'
    $candidates += 'C:\raylib\emsdk'

    foreach ($candidate in $candidates) {
        if ($candidate -and (Test-Path (Join-Path $candidate 'emsdk_env.ps1'))) {
            return (Resolve-Path $candidate).Path
        }
    }
    return $null
}

function Ensure-Make {
    # MSYS2's usr\bin has make plus the Unix tools raylib's Makefile expects.
    $msysMake = 'C:\msys64\usr\bin\make.exe'
    if (Test-Path $msysMake) {
        $env:PATH = (Split-Path $msysMake -Parent) + ';' + $env:PATH
        return
    }
    if (Get-Command make -ErrorAction SilentlyContinue) { return }
    if (Get-Command mingw32-make -ErrorAction SilentlyContinue) { return }

    $makeCandidates = @(
        'C:\msys64\usr\bin\make.exe',
        'C:\msys64\ucrt64\bin\mingw32-make.exe',
        'C:\msys64\mingw64\bin\mingw32-make.exe',
        'C:\raylib\w64devkit\bin\mingw32-make.exe'
    )
    foreach ($make in $makeCandidates) {
        if (Test-Path $make) {
            $env:PATH = (Split-Path $make -Parent) + ';' + $env:PATH
            return
        }
    }
    Fail "make was not found. Install MSYS2 (https://www.msys2.org) or run this from an MSYS2 shell."
}

# ---------------------------------------------------------------------------
# 1. Emscripten
# ---------------------------------------------------------------------------
$emsdk = Find-Emsdk

if (-not (Get-Command emcc -ErrorAction SilentlyContinue)) {
    if (-not $emsdk) {
        Write-Step "Installing Emscripten into $HOME\emsdk (one-time, ~1 GB)"
        $emsdk = Join-Path $HOME 'emsdk'
        & git clone --depth 1 https://github.com/emscripten-core/emsdk.git $emsdk
        if ($LASTEXITCODE -ne 0) { Fail 'Could not clone emsdk.' }

        Push-Location $emsdk
        try {
            & .\emsdk.bat install latest
            if ($LASTEXITCODE -ne 0) { Fail 'emsdk install failed.' }
            & .\emsdk.bat activate latest
            if ($LASTEXITCODE -ne 0) { Fail 'emsdk activate failed.' }
        } finally {
            Pop-Location
        }
    }

    Write-Step "Activating Emscripten from $emsdk"
    & (Join-Path $emsdk 'emsdk_env.ps1') | Out-Null
    if (-not (Get-Command emcc -ErrorAction SilentlyContinue)) {
        Fail "Emscripten is still not on PATH. Open a new terminal and run the build again."
    }
}

$emccVersion = (& emcc --version | Select-Object -First 1)
Write-Host "Using $emccVersion"

Ensure-Make

# ---------------------------------------------------------------------------
# 2. raylib compiled for the browser
# ---------------------------------------------------------------------------
if (-not (Test-Path $RaylibLib)) {
    if (-not (Test-Path (Join-Path $RaylibSrc 'Makefile'))) {
        Write-Step "Downloading raylib $RaylibTag"
        New-Item -ItemType Directory -Force $DepsDir | Out-Null
        & git clone --depth 1 --branch $RaylibTag https://github.com/raysan5/raylib.git $RaylibDir
        if ($LASTEXITCODE -ne 0) { Fail 'Could not clone raylib.' }
    }

    Write-Step "Building raylib $RaylibTag for WebAssembly"
    $env:EMSDK_PATH = $emsdk -replace '\\', '/'
    $jobs = [Environment]::ProcessorCount
    Push-Location $RaylibSrc
    try {
        & emmake make PLATFORM=PLATFORM_WEB "-j$jobs"
        if ($LASTEXITCODE -ne 0) { Fail 'raylib web build failed.' }
    } finally {
        Pop-Location
    }
}

if (-not (Test-Path $RaylibLib)) {
    Fail "raylib web library was not produced at $RaylibLib"
}

# ---------------------------------------------------------------------------
# 3. Game
# ---------------------------------------------------------------------------
Write-Step "Compiling Bounce Back Haven for the browser"
New-Item -ItemType Directory -Force $OutDir | Out-Null

$sources = @(Get-ChildItem (Join-Path $Root 'src') -Filter '*.cpp' | ForEach-Object { $_.FullName })
if ($sources.Count -eq 0) { Fail 'No .cpp files found in src\.' }

$compileArgs = @(
    '-std=c++17',
    '-O2',
    '-Wall',
    '-Wextra',
    '-I.',
    "-I$RaylibSrc",
    $sources,
    $RaylibLib,
    '-o', (Join-Path $OutDir 'index.html'),
    '-sUSE_GLFW=3',
    '-sASYNCIFY',
    '-sALLOW_MEMORY_GROWTH=1',
    '-sINITIAL_MEMORY=64MB',
    '-sSTACK_SIZE=5MB',
    '-sEXIT_RUNTIME=0',
    '-sGL_ENABLE_GET_PROC_ADDRESS=1',
    '-sENVIRONMENT=web',
    '--preload-file', 'assets@assets',
    '--shell-file', (Join-Path $Root 'web\shell.html')
)

Push-Location $Root
try {
    & em++ @compileArgs
    if ($LASTEXITCODE -ne 0) { Fail 'Game compilation failed.' }
} finally {
    Pop-Location
}

foreach ($file in @('index.html', 'index.js', 'index.wasm', 'index.data')) {
    if (-not (Test-Path (Join-Path $OutDir $file))) {
        Fail "Expected build output is missing: $file"
    }
}

# ---------------------------------------------------------------------------
# 4. Zip for itch.io (index.html must be at the root of the archive)
# ---------------------------------------------------------------------------
Write-Step "Packaging dist\bouncebackhaven-web.zip"
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
if (Test-Path $ZipPath) { Remove-Item $ZipPath -Force }

$archive = [System.IO.Compression.ZipFile]::Open($ZipPath, [System.IO.Compression.ZipArchiveMode]::Create)
try {
    foreach ($name in @('index.html', 'index.js', 'index.wasm', 'index.data')) {
        $path = Join-Path $OutDir $name
        [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile(
            $archive, $path, $name, [System.IO.Compression.CompressionLevel]::Optimal) | Out-Null
    }
} finally {
    $archive.Dispose()
}

$zipSize = [math]::Round((Get-Item $ZipPath).Length / 1MB, 2)
Write-Host ""
Write-Host "[ok] Web build:  $OutDir" -ForegroundColor Green
Write-Host "[ok] itch.io zip: $ZipPath ($zipSize MB)" -ForegroundColor Green
Write-Host ""
Write-Host "Upload the zip to itch.io, enable 'This file will be played in the browser',"
Write-Host "and use 1280x720 (or 'Fullscreen') for the embed size."
