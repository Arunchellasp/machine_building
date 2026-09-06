@echo off
:: --- raddbg Debug Script ---
:: Usage: debug or debug raddbg
setlocal

:: Unpack arguments
for %%a in (%*) do set "%%a=1"
if not "%raddbg%"=="1" set raddbg=1

:: --- Fill project environment if this script is run directly ---
if "%_home_%"=="" (
    pushd "%~dp0.."
    set "_home_=%cd%"
    popd
)
if "%_config_%"=="" set "_config_=%~dp0config"
if "%_build_%"=="" set "_build_=%_home_%\build"
if "%_project_name_%"=="" set "_project_name_=machine_maker"
if "%_apps_%"=="" set "_apps_=%~d0\apps"

:: --- Set executable and raddbg project paths ---
set "exe=%_build_%\%_project_name_%.exe"
set "raddbg_project=%_config_%\project.raddbg"
set "raddbg_exe=%_apps_%\raddbg\raddbg.exe"

if not exist "%raddbg_project%" (
    echo [error] raddbg project not found: "%raddbg_project%"
    exit /b 1
)

if not exist "%exe%" (
    echo [error] executable not found: "%exe%"
    echo [hint] run build first.
    exit /b 1
)

if not exist "%raddbg_exe%" (
    echo [error] raddbg executable not found: "%raddbg_exe%"
    exit /b 1
)

:: --- Run raddbg debugger ---
if "%raddbg%"=="1" (
    echo [raddbg debug]
    echo [raddbg exe] "%raddbg_exe%"
    echo [raddbg project] "%raddbg_project%"
    echo [target exe] "%exe%"
    pushd "%_home_%"
    start "" "%raddbg_exe%" --project:"%raddbg_project%" -- "%exe%"
    popd
)
