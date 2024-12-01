@echo off
cd /d "%~dp0"  REM Change to the directory where the script is located
git pull
git add .
git commit -m "update"
git push