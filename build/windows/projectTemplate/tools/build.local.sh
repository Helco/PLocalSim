#!/bin/bash

if [ ! -e './pebble_app.ld' ] ; then
	echo Please run this script from the main project folder!
else
	source ./tools/envvars.sh
	source $PLS_SDK_DIR'/build.project.sh'
fi