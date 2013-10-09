@echo off

if "%PLS_SDK_PATH%"=="" (
	echo Please run this script with build.local.bat in your project\tools directory!
	goto exit
)
	
call %PLS_SDK_PATH%\envvars.bat
call %PLS_SDK_PATH%\run.resCompiler.bat
if errorlevel 0 goto compile
goto exit

:compile

set PLS_SIM_INCLUDES=-isystem %PLS_SDK_PATH%\%PLS_SDK_INCLUDE% -I %PLS_SDK_PATH%\%PLS_SDL_INCLUDE% -I %PLS_SDK_PATH%\%PLS_SDL_INCLUDE%\SDL  -I .\src\ -I .\build\tempLocal\ -I .\build\tempLocal\src\
set PLS_SIM_LIB=-L %PLS_SDK_PATH%\%PLS_SDL_LIB% -L %PLS_SDK_PATH%\bin -lpebbleLocalSim
set PLS_SIM_SOURCE=.\src\*.c
set /p PLS_PROJECT_NAME= < pebble_app.ld
set PLS_SIMULATOR_EXE=%PLS_SIM_OUTPUT%\%PLS_PROJECT_NAME%.exe

%PLS_SDK_PATH%\%PLS_MINGW%\gcc -x c -std=c99 -Wall -O2 -mconsole -DLOCALSIM %PLS_SIM_INCLUDES% %PLS_SIM_SOURCE% %PLS_SIM_LIB% %PLS_SDK_PATH%\%PLS_SDL_MAIN% %PLS_SIM_LIBS% -o %PLS_SIMULATOR_EXE%	

:exit