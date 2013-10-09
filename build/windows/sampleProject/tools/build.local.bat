@echo off
if not exist pebble_app.ld (
	echo Please run this script from the project main folder!
	goto exit
) 

call .\tools\envvars.bat
%PLS_SDK_PATH%\build.project.bat

:exit