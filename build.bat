@echo off
setlocal enabledelayedexpansion

:: --- Определяем архитектуру ---
set ARCH=64
if "%~1"=="32" set ARCH=32

:: --- Проверка наличия cmake ---
cmake >nul 2>&1
if errorlevel 1 (
  if exist "C:\CMake\bin\cmake.exe" (
    set "PATH=C:\CMake\bin;%PATH%"
  ) else (
    echo Error: cmake not installed!!
    exit 1
  )
)

for /f "tokens=3" %%v in ('cmake --version ^| findstr "version"') do set CVER=%%v
for /f "tokens=1,2 delims=." %%a in ("%CVER%") do (
  set CVER1=%%a
  set CVER2=%%b
)

if %CVER1% lss 3 (
  echo Error: cmake version must be ^>= 3.25!!
  exit 1
) else (
  if %CVER1%==3 if %CVER2% lss 25 (
    echo Error: cmake version must be ^>= 3.25!!
    exit 1
  )
)

:: --- Проверка NSIS ---
makensis >nul 2>&1
if errorlevel 1 (
  if exist "C:\nsis\bin\makensis.exe" (
    set "PATH=C:\nsis\bin;%PATH%"
  ) else (
    echo Error: nsis not installed!!
    exit 1
  )
)
set NSISGEN=NSIS64
if "%ARCH%"=="32" set NSISGEN=NSIS

:: --- Проверка Qt ---
if not exist "C:\Qt" (
  echo Error: Qt not installed!!
  exit /b 1
)
for /d %%q in (C:\Qt\*) do set "QT=%%q"

:: --- Определяем toolchain ---
for /d %%t in (%QT%\Tools\mingw*%ARCH%) do set "TOOLCHAIN=%%t"
if "%TOOLCHAIN%"=="" (
  echo Error: x%ARCH% toolchain not found!!
  exit /b 1
)
set "PATH=%TOOLCHAIN%\bin;%PATH%"

:: --- Определяем Qt libs ---
set "QTLIBS=%QT%"
for /d %%l in (%QT%\5*) do set "QTLIBS=%%l"
for /d %%l in (%QTLIBS%\mingw*%ARCH%) do set "QTLIB=%%l"
if "%QTLIB%"=="" (
  echo Error: x%ARCH% Qt not found!!
  exit 1
)

:: --- Параметры CMake ---
set "CMAKEDEF=-DCMAKE_PREFIX_PATH=%QTLIB% -DCMAKE_BUILD_TYPE=Release -DAPP_DEPLOYQT=YES -DAPP_BUILD_TESTS=NO"

:: --- Сборка ---
echo Building application
rmdir /s /q bin 2>nul
rmdir /s /q build 2>nul
rmdir /s /q artifacts 2>nul

cmake -G "MinGW Makefiles" -S . -B build %CMAKEDEF%
if errorlevel 1 exit 1

cmake --build build -j %NUMBER_OF_PROCESSORS%
if errorlevel 1 exit 1

cpack -G %NSISGEN% --config build\CPackConfig.cmake -B artifacts
if errorlevel 1 exit 1

rmdir /s /q artifacts\_* 2>nul

exit 0
