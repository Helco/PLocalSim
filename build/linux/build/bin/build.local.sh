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

SIM_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

#Copy resources
mkdir -p ./build/local/simdata
cp -r $SIM_DIR/localsim/simdata/* ./build/local/simdata/

#Build
if [ "$1" == "--debug" ]
then
	DEBUG_FLAGS="-ggdb"
fi

if [ "$(uname -m)" == "x86_64" ]
then
	ARCH_FLAGS="-m32"
fi

export LIBRARY_PATH=$LIBRARY_PATH':'$SIM_DIR'/localsim/'
SDL_LIBS='-lSDL -lSDLmain -lSDL_ttf -lSDL_image -lm'
RT_LIB='-lPebbleLocalSim'
INCLUDES='-I '$SIM_DIR'/../Pebble/include/ -I ./build/tempLocal/ -I ./build/tempLocal/src/'
APP_SRC='./src/*.c'

CWD=`pwd`
APP=`basename $CWD`
OUTPUT='./build/local/'$APP

RUN_RESCOMPILER=$SIM_DIR'/localsim/resCompiler'
$RUN_RESCOMPILER

if [ $? -eq 0 ]
then
	gcc $ARCH_FLAGS $DEBUG_FLAGS -x c -std=c99 -Wall -o $OUTPUT  $INCLUDES $APP_SRC -lm $RT_LIB $SDL_LIBS
fi
