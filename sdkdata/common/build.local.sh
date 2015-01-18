#!/bin/bash

# Check if we have access to gcc
PLS_SDK_ROOT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PLS_SDK_DIR=$PLS_SDK_ROOT_DIR'/PLocalSim'

if [ ! -e $PLS_SDK_DIR'/envvars.sh' ] ; then
  echo "[FAIL] Could not load envvars.sh"
  exit 1
fi
source $PLS_SDK_DIR'/envvars.sh'

PLS_COMPILE_MODE='release'
if [ "$1" == "--debug" ] ; then
  PLS_COMPILE_MODE='debug'
fi
if hash $PLS_GCC 2>/dev/null; then
  echo "[INFO] Compile app as "$PLS_COMPILE_MODE
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
mkdir -p ./build/local/
cp -f ./appinfo.json ./build/local/
cp -f -r $PLS_SDK_DIR/simdata/* ./build/local/

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
PLS_RSC=$PLS_SDK_DIR'/resCompiler'
PLS_POST_COMMAND=' '
if [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
  PLS_RSC=$PLS_RSC'.exe'
elif [ -e $PLS_SDK_DIR/../../.env/bin/activate ] ; then
  # use the virtual environment to access the required python libs
  source $PLS_SDK_DIR'/../../.env/bin/activate'
  PLS_POST_COMMAND='deactivate'
fi

error='true'
if $PLS_RSC ; then
  error='false'
fi

$PLS_POST_COMMAND

if [ "$error" == 'true' ] ; then
  echo "[FAIL] Could not compile resources"
  exit 1
fi

# Prepare compile variables
PLS_APP_INCLUDES='-I ./build/tempLocal/ -I ./build/tempLocal/src/'
PLS_APP_LIB_INCLUDES='-L '$PLS_SDK_DIR
PLS_APP_LIBS='-lPLocalSim -lSDL2 -lSDL2_ttf -lSDL2_image -lm -lpthread'
PLS_APP_ARGS='-c -x c -O2 -Wall -std=c99 -DLOCALSIM'
PLS_APP_PATH=`pwd`
PLS_APP_NAME=`basename $PLS_APP_PATH`
PLS_APP_OUT='./build/local/'$PLS_APP_NAME
PLS_SDK_HEADERS=$PLS_SDK_DIR'/include'

# Special values
if [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
  # Windows/MinGW
  # ' -I '$PLS_SDK_DIR$PLS_DIR_SDL'/include '
  PLS_APP_INCLUDES=$PLS_APP_INCLUDES
  PLS_APP_LIB_INCLUDES=$PLS_APP_LIB_INCLUDES' -L '$PLS_DIR_SDL'/lib'
  PLS_APP_LIBS='-lmingw32 -lSDL2main '$PLS_APP_LIBS
  PLS_APP_ARGS=$PLS_APP_ARGS' -mconsole -DWIN32 -D_WIN32'
  PLS_APP_OUT=$PLS_APP_OUT'.exe'
else
  # Unix
  if [ -e $PLS_SDK_ROOT_DIR/pebble ] ; then
    PLS_SDK_HEADERS=$PLS_SDK_DIR'/../../Pebble/include'
  fi
fi
PLS_APP_INCLUDES=$PLS_APP_INCLUDES' -isystem '$PLS_SDK_HEADERS

# Debug flag
if [ "$1" == "--debug" ] ; then
  PLS_APP_ARGS=$PLS_APP_ARGS' -ggdb'
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
   
