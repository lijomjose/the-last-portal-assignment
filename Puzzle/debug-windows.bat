@echo off
setlocal
cd /d "%~dp0"
set "mode=%~1"
if "%mode%"=="" set "mode=run"
if not "%~2"=="" goto usage
if "%mode%"=="run" goto mode_ok
if "%mode%"=="build-only" goto mode_ok
if "%mode%"=="gdb" goto mode_ok
goto usage
:mode_ok
if not exist puzzle\student (
  echo Missing Puzzle source. Extract the whole assignment project first.
  goto failed
)
where gcc >nul 2>nul
if errorlevel 1 (
  echo A 64-bit MinGW-w64 GCC compiler must be installed and available on PATH.
  goto failed
)
if "%mode%"=="gdb" (
  where gdb >nul 2>nul
  if errorlevel 1 (
    echo GDB is not installed. Use the default mode or install GDB first.
    goto failed
  )
)
if not exist build\puzzle-debug mkdir build\puzzle-debug
if errorlevel 1 goto failed
echo Building Puzzle with debug symbols and no optimisation...
gcc --version >build\puzzle-debug\build.log 2>&1
gcc -std=c99 -Wall -Wextra -Wpedantic -Werror -g3 -O0 ^
  -fno-omit-frame-pointer -static-libgcc -DPORTAL_PUZZLE=1 ^
  -Iinclude -Ivendor\raylib\windows\include ^
  src\main.c src\view.c src\game.c src\repair.c ^
  puzzle\student\stack.c puzzle\student\queue.c ^
  puzzle\student\attack_list.c puzzle\student\diagnostics.c ^
  vendor\raylib\windows\lib\libraylib.a ^
  -lopengl32 -lgdi32 -lwinmm -lm ^
  -o build\puzzle-debug\last_portal.exe >>build\puzzle-debug\build.log 2>&1
if errorlevel 1 (
  type build\puzzle-debug\build.log
  echo Build failed. Fix the first error, save and try again. No game was launched.
  goto failed
)
echo Build succeeded: build\puzzle-debug\last_portal.exe
echo Compiler log: build\puzzle-debug\build.log
if "%mode%"=="build-only" exit /b 0
if "%mode%"=="gdb" goto debugger
echo Opening your Puzzle. Close it to see the runtime log here.
echo Runtime log: build\puzzle-debug\run.log
build\puzzle-debug\last_portal.exe >build\puzzle-debug\run.log 2>&1
set "result=%errorlevel%"
type build\puzzle-debug\run.log
echo Game exited with status %result%.
pause
exit /b %result%
:debugger
echo GDB: after a crash, type bt for the call stack; quit to leave.
gdb -q -ex "set pagination off" ^
  -ex "set logging file build/puzzle-debug/gdb.log" ^
  -ex "set logging overwrite on" -ex "set logging enabled on" ^
  -ex run --args build\puzzle-debug\last_portal.exe
set "result=%errorlevel%"
pause
exit /b %result%
:usage
echo Usage: debug-windows.bat [run^|build-only^|gdb]
pause
exit /b 2
:failed
echo Close the game before rebuilding. See DEBUGGING.md for help.
pause
exit /b 1
