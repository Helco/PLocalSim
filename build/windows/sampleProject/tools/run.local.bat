@echo off
if not exist .\pebble_app.ld (
	echo Please run this script from the project main folder!
	goto exit
)

call .\tools\envvars.bat
call %PLS_SDK_PATH%\envvars.bat
set /p PLS_PROJECT_NAME= < pebble_app.ld
set PLS_SIMULATOR_EXE=%PLS_PROJECT_NAME%.exe
if not exist %PLS_SIM_OUTPUT%\%PLS_SIMULATOR_EXE% (
	echo This project is not compiled yet!
	goto exit
)

pushd %PLS_SIM_OUTPUT%
%PLS_SIMULATOR_EXE%
popd

:exit