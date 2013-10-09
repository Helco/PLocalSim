@echo off 
REM Variables
set PLS_INST_FROM=%~dp0
set PLS_INST_TO=%CD%
set PLS_ROBOCOPY_SILENT=/NJH /NJS /NP /NFL /NDL /NS /NC

REM Get SDK dir
if not "%1%"=="" (
	set PLS_INST_TO=%1%
)
echo Install pebble simulator sdk in: %PLS_INST_TO%

REM Install
echo Copy Simulator binaries
robocopy %PLS_INST_FROM%\bin %PLS_INST_TO%\bin %PLS_ROBOCOPY_SILENT% /E >NUL

echo Copy Pebble SDK headers
robocopy %PLS_INST_FROM%\include %PLS_INST_TO%\include %PLS_ROBOCOPY_SILENT% /E >NUL

echo Copy MinGW (if it exists)
robocopy %PLS_INST_FROM%\MinGW %PLS_INST_TO%\MinGW %PLS_ROBOCOPY_SILENT% /E >NUL

echo Copy SDL
robocopy %PLS_INST_FROM%\SDL %PLS_INST_TO%\SDL %PLS_ROBOCOPY_SILENT% /E >NUL

echo Copy default project
robocopy %PLS_INST_FROM%\sampleProject %PLS_INST_TO%\sampleProject %PLS_ROBOCOPY_SILENT% /E >NUL

echo Copy Simulator scripts
robocopy %PLS_INST_FROM% %PLS_INST_TO% envvars.bat %PLS_ROBOCOPY_SILENT% >NUL
robocopy %PLS_INST_FROM% %PLS_INST_TO% build.project.bat %PLS_ROBOCOPY_SILENT% >NUL
robocopy %PLS_INST_FROM% %PLS_INST_TO% run.project.bat %PLS_ROBOCOPY_SILENT% >NUL
robocopy %PLS_INST_FROM% %PLS_INST_TO% create.project.bat %PLS_ROBOCOPY_SILENT% >NUL
robocopy %PLS_INST_FROM% %PLS_INST_TO% run.resCompiler.bat %PLS_ROBOCOPY_SILENT% >NUL