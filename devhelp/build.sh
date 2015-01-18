#!/bin/bash

# This is basically a hacked build.local.sh
# don't expect anything fancy in here

# Load (and fix) envvars
if [ ! -e '../scripts/envvars.sh' ] ; then
  echo "[FAIL] Could not load envvars.sh"
  exit 1
fi
PLS_ENV_PREFIX='.'
source '../scripts/envvars.sh'

# Check for gcc
if hash $PLS_GCC 2>/dev/null; then
  : # no-op
else
  echo "[FAIL] Could not find required program gcc"
  exit 1
fi

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

# Copy resources and app info
PLS_OS_NAME='unix'
if [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
  PLS_OS_NAME='windows'
fi

mkdir -p ./build/local/
cp -u  ./appinfo.json ./build/local/
cp -u  -r ../simdata/common/* ./build/local/
cp -u  -r ../simdata/$PLS_OS_NAME/* ./build/local/

# Check for and copy JS
if [ -e './src/js/pebble-js-app.js' ] ; then
  if [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
     echo "[WARNING] Windows does not support javascript apps yet"
  else 
    cat $PLS_SDK_DIR/header.js > ./build/local/pebble-js-app-local.js
    cat ./src/js/pebble-js-app.js >> ./build/local/pebble-js-app-local.js
  fi
fi

# Compile resources
PLS_RSC='../bin/resCompiler'
if [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
  PLS_RSC=$PLS_RSC'.exe'
fi # No virualenv because this is not a pebble-installed sdk

if [ ! -e $PLS_RSC ] ; then
  echo "[FAIL] Could not start resource compiler"
elif $PLS_RSC ; then
  : # no-op
else
  echo "[FAIL] Could not compile resources"
  exit 1
fi

# Prepare compile variables
PLS_APP_INCLUDES='-I ./build/tempLocal/ -I ./build/tempLocal/src/ -isystem ../include '
PLS_APP_LIB_INCLUDES='-L ../bin'
PLS_APP_LIBS='-lPLocalSim -lSDL2 -lSDL2_ttf -lSDL2_image -lm -lpthread'
PLS_APP_ARGS='-c -x c -O2 -Wall -std=c99 -DLOCALSIM -ggdb'
PLS_APP_NAME='testapp'
PLS_APP_OUT='./build/local/'$PLS_APP_NAME

# Special values
if [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
  # Windows/MinGW
  PLS_APP_INCLUDES=$PLS_APP_INCLUDES
  PLS_APP_LIB_INCLUDES=$PLS_APP_LIB_INCLUDES' -L '$PLS_DIR_SDL'/lib'
  PLS_APP_LIBS='-lmingw32 -lSDL2main '$PLS_APP_LIBS
  PLS_APP_ARGS=$PLS_APP_ARGS' -mconsole -DWIN32 -D_WIN32'
  PLS_APP_OUT=$PLS_APP_OUT'.exe'
fi

# Compile object files
mkdir -p ./build/tempLocal
rm -f ./build/tempLocal/*.o
filelist='./src/*.c'
PLS_APP_OBJECTS=" "
for file in `ls $filelist` ; do
  filename=${file##*/} # without the directory
  echo 'Compiling '$filename
  objectFile=./build/tempLocal/${filename%.*}'.o'
  if $PLS_GCC $PLS_APP_ARGS $PLS_APP_INCLUDES $file -o $objectFile ; then
    PLS_APP_OBJECTS=$PLS_APP_OBJECTS' '$objectFile
  else
    exit 1
  fi
done

echo Linking
$PLS_GCC $PLS_APP_OBJECTS $PLS_APP_LIB_INCLUDES $PLS_APP_LIBS -o $PLS_APP_OUT
   
