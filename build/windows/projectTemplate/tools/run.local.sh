#!/bin/bash

if [ ! -e './pebble_app.ld' ] ; then
	echo Please run this script from the main project folder!
else
	source ./tools/envvars.sh
	DIR=$(pwd -W)
	source $PLS_SDK_DIR'/envvars.sh'
	PLS_SIMULATOR_EXE=$(<./pebble_app.ld)'.exe'
	PLS_SIMULATOR_EXE_PATH='.'$PLS_SIM_OUTPUT'/'$PLS_SIMULATOR_EXE
	if [ -e "$PLS_SIMULATOR_EXE_PATH" ] ; then
		cd '.'$PLS_SIM_OUTPUT
		'./'$PLS_SIMULATOR_EXE
		cd $DIR
	else
		echo This project is not compiled yet!
	fi
fi