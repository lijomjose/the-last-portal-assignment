@echo off
setlocal
call "%~dp0Puzzle\debug-windows.bat" %*
exit /b %errorlevel%
