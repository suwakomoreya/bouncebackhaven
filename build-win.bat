@echo off
rem Build bouncebackhaven for Windows with MSYS2 MinGW-w64 + raylib.
rem
rem Works from CMD or PowerShell:
rem   build-win.bat        compile only
rem   build-win.bat run    compile and launch the game
rem
rem One-time setup: a MinGW-w64 g++ that includes raylib, either W64DevKit
rem (bundles raylib under include/ and lib/, as in C:\raylib) or in an
rem "MSYS2 UCRT64" terminal:
rem   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib
rem
rem Optional: set RAYLIB_DIR to use a downloaded raylib instead of MSYS2's package.

setlocal EnableExtensions EnableDelayedExpansion

set "ROOT=%~dp0"
set "NAME=bouncebackhaven"
set "SRC_DIR=%ROOT%src"
set "BUILD_DIR=%ROOT%build"

rem --- locate g++ ------------------------------------------------------------
where g++ >nul 2>nul
if not errorlevel 1 goto :have_gxx

if exist "C:\msys64\ucrt64\bin\g++.exe" (
    set "PATH=C:\msys64\ucrt64\bin;%PATH%"
    goto :have_gxx
)
if exist "%LOCALAPPDATA%\msys64\ucrt64\bin\g++.exe" (
    set "PATH=%LOCALAPPDATA%\msys64\ucrt64\bin;%PATH%"
    goto :have_gxx
)
if exist "%ProgramFiles%\msys64\ucrt64\bin\g++.exe" (
    set "PATH=%ProgramFiles%\msys64\ucrt64\bin;%PATH%"
    goto :have_gxx
)

echo [error] g++ not found.
echo Install MSYS2 from https://www.msys2.org, open "MSYS2 UCRT64", and run:
echo   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib
exit /b 1

:have_gxx
if not exist "%SRC_DIR%\*.cpp" (
    echo [error] no .cpp files found in %SRC_DIR%
    exit /b 1
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

rem --- source list (response file handles paths with spaces) -----------------
set "RSP=%BUILD_DIR%\sources.rsp"
if exist "%RSP%" del "%RSP%"
for %%f in ("%SRC_DIR%\*.cpp") do (
    set "ONE=%%~f"
    rem GCC response files treat backslash as an escape, so use forward slashes.
    set "ONE=!ONE:\=/!"
    echo "!ONE!">>"%RSP%"
)

rem --- raylib link flags -----------------------------------------------------
if defined RAYLIB_DIR (
    set RAYLIB_FLAGS=-I"%RAYLIB_DIR%\include" -L"%RAYLIB_DIR%\lib" -lraylib
) else (
    rem A raylib bundled with the compiler itself (W64DevKit) is a static
    rem libraylib.a with no separate glfw3 import library, so -lraylib alone
    rem links it. MSYS2's raylib package ships a shared libraylib.dll plus
    rem libglfw3, so that one needs both.
    set "RAYLIB_FLAGS=-lraylib -lglfw3"
    set "GXX_BIN="
    for /f "delims=" %%f in ('where g++') do if not defined GXX_BIN set "GXX_BIN=%%f"
    for %%f in ("!GXX_BIN!") do set "RAYLIB_LIBDIR=%%~dpf..\lib\"
    if exist "!RAYLIB_LIBDIR!libraylib.a" if not exist "!RAYLIB_LIBDIR!libglfw3.a" if not exist "!RAYLIB_LIBDIR!libglfw3.dll.a" set "RAYLIB_FLAGS=-lraylib"
)

echo [build] %NAME%.exe
g++ -std=c++17 -O2 -Wall -Wextra @"%RSP%" -o "%BUILD_DIR%\%NAME%.exe" %RAYLIB_FLAGS% -lopengl32 -lgdi32 -lwinmm
if errorlevel 1 (
    echo.
    echo [error] build failed. If raylib is missing, run in "MSYS2 UCRT64":
    echo   pacman -S --needed mingw-w64-ucrt-x86_64-raylib
    exit /b 1
)

rem --- copy runtime DLLs so the exe runs outside the MSYS2 shell --------------
for %%d in (libraylib.dll glfw3.dll) do (
    for /f "delims=" %%p in ('where %%d 2^>nul') do copy /y "%%p" "%BUILD_DIR%\" >nul
)

echo [ok]    %BUILD_DIR%\%NAME%.exe
echo         run: "%BUILD_DIR%\%NAME%.exe"

rem Optional: "build-win.bat run" compiles and launches the game.
if /i "%~1"=="run" (
    pushd "%ROOT%"
    "%BUILD_DIR%\%NAME%.exe"
    popd
)
exit /b 0
