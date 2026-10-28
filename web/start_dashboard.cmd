@rem 本入口只启动127.0.0.1静态网页服务并打开浏览器，不访问串口或下载FPGA。
@echo off
cd /d "%~dp0"
where py >nul 2>nul
if errorlevel 1 goto python_fallback
py -3 serve.py --open
goto finished
:python_fallback
python serve.py --open
:finished
pause
