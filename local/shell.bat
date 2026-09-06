@echo off

cd /D %~dp0

set _apps_=%~d0\apps
set _config_=%cd%\config

pushd ..
set _home_=%cd%
set _build_=%cd%\build
set _project_name_=machine_maker
popd

title %_project_name_%_cmd


set path=%cd%;%path%
set path=%_home_%;%path%
set path=%_build_%;%path%
set path=%_config_%;%path%
set path=%_apps_%\4coder;%path%
set path=%_apps_%\raddbg;%path%
set path=%_apps_%\ctime;%path%
set path="%_apps_%\cloc";%path%

:: --- Enforce English Language Output for CLI, MSVC & Tech Libraries ---
set VSLANG=1033
set LANG=en_US.UTF-8
set LC_ALL=en_US.UTF-8


call "%_apps_%\msvc\VC\Auxiliary\Build\vcvarsall.bat" x64

cd /D %_home_%

%windir%\system32\cmd.exe