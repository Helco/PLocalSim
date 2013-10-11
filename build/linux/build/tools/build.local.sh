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

#Copy resources
mkdir -p ./build/local/simdata
cp -r ./tools/local/simdata/* ./build/local/simdata/


#Build
export LIBRARY_PATH=$LIBRARY_PATH':./tools/local/'
SDL_LIBS='-lSDL -lSDLmain -lSDL_ttf -lSDL_image -lm'
RT_LIB='-lPebbleLocalSim'
INCLUDES='-I ./include/ -I ./build/tempLocal/ -I ./build/tempLocal/src/'
RT_SRC='./include/local/*.c'
APP_SRC='./src/*.c'

CWD=`pwd`
APP=`basename $CWD`
OUTPUT='./build/local/'$APP

./tools/local/resCompiler
if [ $? -eq 0 ]
then
	gcc -x c -std=c99 -Wall -o $OUTPUT  $INCLUDES $APP_SRC -lm $RT_LIB $SDL_LIBS
fi
