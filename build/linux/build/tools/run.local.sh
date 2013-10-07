#!/bin/bash

while [ ! -e "pebble_app.ld" ]
do
	if [ `pwd` == '/' ]
	then
		echo "[FAIL] Could not find 'pebble_app.ld' in this directory or any parent" 1>&2
		exit 1
	fi
	cd ..
done

CWD=`pwd`
APP=`basename $CWD`

cd ./build/local/

while [ ! -e $APP ]
do
	if [ `pwd` == '/' ]
	then
		echo "[FAIL] Local simulator is not built" 1>&2
		exit 1
	fi
	cd ..
done

./$APP

cd ../../
