@echo off
setlocal
cd /d "%~dp0"

echo [FantasyCraft] Budowanie aktualnej wersji...
ninja -C build
if errorlevel 1 (
    echo [ERROR] Kompilacja nie powiodla sie. Sprawdz build.bat lub konfiguracje CMake.
    pause
    exit /b 1
)

echo [FantasyCraft] Uruchamianie gry...
start "" "%~dp0fantasy_craft.exe"
