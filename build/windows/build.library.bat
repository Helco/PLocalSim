@echo off
REM prepare for compiling
call envvars.bat

set PLS_SIM_INCLUDES=-I %PLS_SDL_INCLUDE% -isystem %PLS_SDK_INCLUDE% -I %PLS_DIR_LOCAL% -I %PLS_DIR_SDL_GFX% 

set PLS_SIM_OUTPUT=.\bin\libpebbleLocalSim.a
set PLS_SIM_DIR_OUTPUT=.\obj\

if not exist %PLS_SIM_DIR_OUTPUT% mkdir %PLS_SIM_DIR_OUTPUT%

REM compile the simulator
for %%F in (%PLS_DIR_LOCAL%\*.c) do (
	if not "%1"=="silent" (
		echo Compiling %%~nxF
	)
	%PLS_MINGW%\gcc -c -x c -O2 -Wall -std=c99 -DWIN32 -mconsole %PLS_SIM_INCLUDES% %PLS_DIR_LOCAL%\%%~nxF -o %PLS_SIM_DIR_OUTPUT%\%%~nF.o 
	%PLS_MINGW%\ar rcs %PLS_SIM_OUTPUT% %PLS_SIM_DIR_OUTPUT%\%%~nF.o
)

REM compile SDL_gfx
if not "%1"=="silent" (
	echo Compiling SDL_gfx
)
for %%F in (%PLS_DIR_SDL_GFX%\*.c) do (
	%PLS_MINGW%\gcc -c -x c -O2 -w -std=c99 -mconsole %PLS_SIM_INCLUDES% %PLS_DIR_SDL_GFX%\%%~nxF -o %PLS_SIM_DIR_OUTPUT%\%%~nF.o 
	%PLS_MINGW%\ar rcs %PLS_SIM_OUTPUT% %PLS_SIM_DIR_OUTPUT%\%%~nF.o
)