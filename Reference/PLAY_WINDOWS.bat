@echo off
setlocal
cd /d "%~dp0"
if not exist "bin\last_portal.exe" (
  echo Missing game executable. Extract the entire assignment pack first.
  pause
  exit /b 1
)
"bin\last_portal.exe"
if errorlevel 1 (
  echo The reference game could not start or exited with an error.
  echo Use Windows x64 with working OpenGL graphics support.
  pause
  exit /b 1
)
