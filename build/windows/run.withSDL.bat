@echo off

REM set SDL dll path
set PLS_RSC_PATH=%~dp0
set PATH=%PATH%;%PLS_RSC_PATH%\SDL\bin
set FILE=
set ARG=

if "%1"=="-ARG" (
	set FILE=%2
	set ARG=%3
) else (
	set FILE=%1
)

REM run it
%PLS_RSC_PATH%\bin\%FILE% %ARG%
set PLS_RES_ERROR=%ERRORLEVEL%

REM print any output
REM if exist %PLS_RSC_PATH%\bin\stdout.txt (
REM	type %PLS_RSC_PATH%\bin\stdout.txt
REM	del %PLS_RSC_PATH%\bin\stdout.txt
REM )

REM return exit code
exit /B %PLS_RES_ERROR%