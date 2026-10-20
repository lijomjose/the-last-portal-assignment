@echo off
setlocal
cd /d "%~dp0"
if not exist build\puzzle mkdir build\puzzle
gcc -std=c99 -Wall -Wextra -Wpedantic -Werror -DPORTAL_PUZZLE=1 -Iinclude ^
  tests\check_repair.c src\repair.c puzzle\student\stack.c puzzle\student\queue.c puzzle\student\attack_list.c puzzle\student\diagnostics.c ^
  -o build\puzzle\check_repairs.exe
if errorlevel 1 exit /b 1
build\puzzle\check_repairs.exe %*
