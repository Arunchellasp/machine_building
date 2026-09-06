@echo off
ctime -begin "%_config_%\ctime.ctm"
call "%_home_%\build.bat" %*
ctime -end "%_config_%\ctime.ctm" %ERRORLEVEL%