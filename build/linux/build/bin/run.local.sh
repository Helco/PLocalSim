#!/bin/bash

while [ ! -e "appinfo.json" ]
do
	if [ `pwd` == '/' ]
	then
		echo "[FAIL] Could not find 'appinfo.json' in this directory or any parent" 1>&2
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

if [ "$1" == "--debug" ]
then
	gdb ./$APP
else
	./$APP
fi

cd ../../
