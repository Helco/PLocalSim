#!/bin/bash

# This is basically a hacked run.local.sh
# don't expect anything fancy in here

PLS_MY_DIR=`pwd`

# Load (and fix) envvars
if [ ! -e '../scripts/envvars.sh' ] ; then
  echo "[FAIL] Could not load envvars.sh"
  exit 1
fi
PLS_ENV_PREFIX=$PLS_MY_DIR'/.'
source '../scripts/envvars.sh'

# Check for gdb
if hash $PLS_GDB 2>/dev/null; then
  : # no-op
else
  echo "[FAIL] Could not find required program gdb"
  exit 1
fi


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
PLS_APP_NAME='testapp'
PLS_APP_EXEC='./build/local/'$PLS_APP_NAME
if [ -e "$PLS_APP_EXEC" ] ; then
  cd build/local

  if [ "$1" == "--debug" ] ; then
    $PLS_GDB ./$PLS_APP_NAME
  else
    ./$PLS_APP_NAME
  fi
else
  echo "[FAIL] Could not find simulator, maybe it is not compiled yet?"
fi

cd $PLS_DIR_BACKUP
