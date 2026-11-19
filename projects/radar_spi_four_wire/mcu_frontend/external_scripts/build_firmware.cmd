@echo off
REM Author attribution: jingran Cheng (confirmed by project owner).
REM For internal learning use only. Original source notices retained.
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0build_firmware.ps1" %*
exit /b %errorlevel%

