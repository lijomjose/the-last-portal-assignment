@echo off
setlocal
cd /d "%~dp0"
set "edition=%~1"
if "%edition%"=="" set "edition=puzzle"
if not exist puzzle\student if "%~1"=="" set "edition=completed"
if "%edition%"=="puzzle" (
  set "student_dir=puzzle\student"
  set "puzzle=1"
) else if "%edition%"=="completed" (
  set "student_dir=completed"
  set "puzzle=0"
) else (
  echo Choose puzzle or completed.
  exit /b 2
)
where gcc >nul 2>nul
if errorlevel 1 (
  echo A 64-bit MinGW-w64 GCC compiler is required. See README.md.
  exit /b 1
)
if not exist build\%edition% mkdir build\%edition%
gcc -std=c99 -Wall -Wextra -Wpedantic -Werror -O2 -static-libgcc ^
  -DPORTAL_PUZZLE=%puzzle% -Iinclude -Ivendor\raylib\windows\include ^
  src\main.c src\view.c src\game.c src\repair.c ^
  %student_dir%\stack.c %student_dir%\queue.c %student_dir%\attack_list.c %student_dir%\diagnostics.c ^
  vendor\raylib\windows\lib\libraylib.a ^
  -lopengl32 -lgdi32 -lwinmm -lm -o build\%edition%\last_portal.exe
if errorlevel 1 exit /b 1
echo Built: build\%edition%\last_portal.exe
