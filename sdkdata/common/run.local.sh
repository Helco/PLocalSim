#!/bin/bash

PLS_DIR_BACKUP=`pwd`

# Check if we are in a project
while [ ! -e "appinfo.json" ]
do
	if [ `pwd` == '/' ]
	then
		echo "[FAIL] Could not find 'appinfo.json' in this directory or any parent" 1>&2
		exit 1
	fi
	cd ..
done

# Run the simulator if we find it
PLS_APP_PATH=`pwd`
PLS_APP_NAME=`basename $PLS_APP_PATH`
PLS_APP_EXEC='./build/local/'$PLS_APP_NAME
if [ -e "$PLS_APP_EXEC" ] ; then
  cd build/local
  ./$PLS_APP_NAME
else
  echo "[FAIL] Could not find simulator, maybe it is not compiled yet?"
fi

cd $PLS_DIR_BACKUP