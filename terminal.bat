@echo off
cd /D %~dp0\local
wt -M -d . -p "Command Prompt" cmd /k shell.bat