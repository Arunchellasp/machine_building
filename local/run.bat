@echo off
if "%_project_name_%"=="" set "_project_name_=machine_maker"
if "%_build_%"=="" set "_build_=%~dp0..\build"
"%_build_%\%_project_name_%.exe"