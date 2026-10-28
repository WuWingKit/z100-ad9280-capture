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
