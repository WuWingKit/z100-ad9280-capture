@echo off
call "E:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
cd /d "%~dp0.."
cl /nologo /W4 /WX /TC sim\test_mcu_pipeline.c mcu_sync\radar_sync_pipeline.c /Fe:evidence\test_mcu_pipeline.exe /Fo:evidence\ /link > evidence\mcu_host_build.txt 2>&1
if errorlevel 1 exit /b 1
evidence\test_mcu_pipeline.exe > evidence\mcu_host_result.txt
