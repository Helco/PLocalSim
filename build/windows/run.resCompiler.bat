@echo off
REM Because of some problems the resCompiler needs a script to run itself...

REM set SDL dll path
set PLS_RSC_PATH=%~dp0
set PATH=%PATH%;%PLS_RSC_PATH%\SDL\bin

REM run it
%PLS_RSC_PATH%\bin\resCompiler.exe
set PLS_RES_ERROR=%ERRORLEVEL%

REM print any output
REM if exist %PLS_RSC_PATH%\bin\stdout.txt (
REM	type %PLS_RSC_PATH%\bin\stdout.txt
REM	del %PLS_RSC_PATH%\bin\stdout.txt
REM )

REM return exit code
exit /B %PLS_RES_ERROR%