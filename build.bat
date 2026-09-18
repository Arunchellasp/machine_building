@echo off
setlocal enabledelayedexpansion

:: ==============================================================================
:: ImGui Standalone Application Build Script
:: Call from developer command prompt (with cl.exe in PATH)
:: Usage: build [debug|release]
:: ==============================================================================

:: Ensure we are working from the directory where this script resides
cd /D %~dp0

for %%a in (%*) do set "%%a=1"
if not "%release%"=="1" set debug=1
if "%debug%"=="1"   set release=0 && echo [debug mode]
if "%release%"=="1" set debug=0 && echo [release mode]

set _build_=build
set _home_=..
set third_party=%_home_%\code\third_party

set cl_warnings=/W4 /wd4201 /wd4267 /wd4244 /wd4576 /wd4005 /wd4245 /wd4310 /wd4100 /wd4018 /wd4189 /wd4996 /wd4146 /wd4505 /wd4864 /wd5054 /wd4701 /wd4457 /wd4305 /wd4309 /wd4459

:: Added /I%third_party%\stb so both #include "stb/stb_image.h" and #include "stb_image.h" work

set cl_libs=/I%_home_%\code\ /I%third_party% /I%third_party%\stb /I%third_party%\googol_tech /I%third_party%\glfw-3.4\deps /I%third_party%\glfw-3.4\include /I%third_party%\imgui-1.92.9b /I%third_party%\IconFontCppHeaders

set cl_common=/nologo /MT /EHsc /FC /Z7 /Oi /std:c++20 /Zc:strictStrings- /D_CRT_SECURE_NO_WARNINGS %cl_warnings% %cl_libs%

if "%debug%"=="1" (
    set compile=call cl /Od /Ob1 /DBUILD_DEBUG=1 %cl_common%
) else (
    set compile=call cl /O2 /DBUILD_DEBUG=0 %cl_common%
)

set compile_link=/link /IGNORE:4099 /SUBSYSTEM:windows /ENTRY:wmainCRTStartup /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:LIBCMTD /NODEFAULTLIB:libucrt.lib /DEFAULTLIB:ucrt.lib /incremental:no %third_party%\googol_tech\gts.lib %third_party%\googol_tech\ExtMdl.lib %third_party%\glfw-3.4\src\Release\glfw3.lib opengl32.lib gdi32.lib shell32.lib winmm.lib dwmapi.lib user32.lib advapi32.lib ole32.lib oleaut32.lib

if not exist "%_build_%" mkdir "%_build_%"

:: Ensure GoogolTech runtime DLLs and config files are present in the build directory
copy /Y "%third_party%\googol_tech\*.dll" "%_build_%\" >nul 2>&1
copy /Y "%third_party%\googol_tech\*.cfg" "%_build_%\" >nul 2>&1

pushd %_build_%
del *.pdb >nul 2>&1
%compile% %_home_%\code\machine_maker\machine_maker_main.cpp %compile_link% /out:machine_maker.exe || (popd && exit /b 1)
popd

echo [Build Success: build\machine_maker.exe]