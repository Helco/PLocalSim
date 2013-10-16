@echo off

if "%PLS_SDK_PATH%"=="" (
	echo Please run this script with build.local.bat in your project\tools directory!
	goto exit
)
	
call %PLS_SDK_PATH%\envvars.bat
call %PLS_SDK_PATH%\run.withSDL.bat resCompiler.exe
if errorlevel 0 goto compile
goto exit


:compile
set PLS_SIM_LIB=-L %PLS_SDK_PATH%\%PLS_SDL_LIB% -L %PLS_SDK_PATH%\bin -lpebbleLocalSim
set /p PLS_PROJECT_NAME= < pebble_app.ld
set PLS_SIMULATOR_EXE=%PLS_SIM_OUTPUT%\%PLS_PROJECT_NAME%.exe
set PLS_SIM_TMP_DIR=.\build\tempLocal\
set PLS_SIM_OBJ_DIR=.\build\tempLocal\obj\
set PLS_SIM_DIR_SOURCE=.\src\
set PLS_SIM_SOURCE_OBJ=
set PLS_SIM_INCLUDES=-isystem %PLS_SDK_PATH%\%PLS_SDK_INCLUDE% -I %PLS_SDK_PATH%\%PLS_SDL_INCLUDE% -I %PLS_SDK_PATH%\%PLS_SDL_INCLUDE%\SDL  -I %PLS_SIM_DIR_SOURCE% -I %PLS_SIM_TMP_DIR% -I %PLS_SIM_TMP_DIR%\src\
set PLS_SIM_WINRES_PATH=%PLS_SIM_TMP_DIR%\winres
setlocal enabledelayedexpansion

if not exist %PLS_SIM_OBJ_DIR% mkdir %PLS_SIM_OBJ_DIR%

for %%F in (%PLS_SIM_DIR_SOURCE%\*.c) do (
	echo Compiling %%~nxF
	%PLS_SDK_PATH%\%PLS_MINGW%\gcc -c -x c -O2 -Wall -std=c99 -DWIN32 -mconsole -DLOCALSIM %PLS_SIM_INCLUDES% %PLS_SIM_DIR_SOURCE%\%%~nxF -o %PLS_SIM_OBJ_DIR%\%%~nF.o
	if not exist %PLS_SIM_OBJ_DIR%\%%~nF.o goto exit
	set PLS_SIM_SOURCE_OBJ=!PLS_SIM_SOURCE_OBJ! %PLS_SIM_OBJ_DIR%\%%~nF.o
	REM Extracting metadata 
	start %PLS_SDK_PATH%\%PLS_MINGW%\objcopy --only-section=.pbl_header -O binary %PLS_SIM_OBJ_DIR%\%%~nF.o %PLS_SIM_TMP_DIR%\metadata.bin > NUL
	if exist %PLS_SIM_TMP_DIR%\metadata.bin cp %PLS_SIM_TMP_DIR%\metadata.bin %PLS_SIM_OUTPUT%\metadata.bin >NUL
)

call %PLS_SDK_PATH%\run.withSDL.bat -ARG metaCompiler.exe %PLS_PROJECT_NAME%
if errorlevel 0 goto compilewinres
goto exit

:compilewinres
echo Compiling metadata
%PLS_SDK_PATH%\%PLS_MINGW%\windres %PLS_SIM_WINRES_PATH%.rc -O coff -o %PLS_SIM_WINRES_PATH%.res
if not exist %PLS_SIM_WINRES_PATH%.res goto exit

echo Linking
%PLS_SDK_PATH%\%PLS_MINGW%\gcc -std=c99 -Wall -O2 -mconsole -DLOCALSIM %PLS_SIM_INCLUDES% %PLS_SDK_PATH%\%PLS_SDL_MAIN% %PLS_SIM_SOURCE_OBJ% %PLS_SIM_LIB% %PLS_SIM_LIBS% %PLS_SIM_WINRES_PATH%.res -o %PLS_SIMULATOR_EXE%	

:exit