@echo off
setlocal
cd /d "%~dp0"
set "edition=%~1"
if "%edition%"=="" set "edition=puzzle"
if not exist puzzle\student if "%~1"=="" set "edition=completed"
call build-windows.bat %edition%
if errorlevel 1 (
  pause
  exit /b 1
)
build\%edition%\last_portal.exe
