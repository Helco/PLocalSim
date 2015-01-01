#!/bin/bash

SDK_DIR=$(cd $(dirname "$0"); pwd)

if [ -z "$1" ] ; then
	echo Usage: create.project.sh project_name 
else
	PROJECT_DIR='./'$1
	if [ -e $PROJECT_DIR'/pebble_app.ld' ] ; then
		echo The project $1 already exists
	else	
		# copy the project template
		mkdir -p $PROJECT_DIR
		filelist=$SDK_DIR'/projectTemplate/*'
		cp -arf $filelist $PROJECT_DIR > /dev/null 2>&1
	
		# generate scripts
		echo "$1" > $PROJECT_DIR'/pebble_app.ld'
		touch $PROJECT_DIR'/tools/envvars.sh'
		echo 'PLS_SDK_DIR='$SDK_DIR > $PROJECT_DIR'/tools/envvars.sh'
	
		# print friendly message
		echo You created the project: $1
		echo Now you can run: 
		echo "     cd $1"
		echo "     ./tools/build.local.sh"
		echo "     ./tools/run.local.sh"
	fi
fi