@echo off
setlocal
cd /d "%~dp0"

echo [FantasyCraft Build System] Rozpoczynanie kompilacji gry (Ninja / GCC / Clang C++17)...

if not exist build mkdir build

cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Konfiguracja CMake nie powiodla sie!
    pause
    exit /b %ERRORLEVEL%
)

cmake --build build --config Release
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Kompilacja silnika nie powiodla sie!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo ========================================================
echo  Kompilacja zakonczona sukcesem!
echo  Plik wykonywalny: %~dp0fantasy_craft.exe
echo ========================================================
echo Uruchom run.bat lub bezposrednio fantasy_craft.exe
pause
