@echo off
rem Builds Iron Ring on Windows and packs Dist\IronRing-<version>-windows-x64.zip (docs\building.md).
rem Arguments go to build-windows.ps1: -Test, -Run, -MinGW.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build-windows.ps1" %*
if errorlevel 1 pause
