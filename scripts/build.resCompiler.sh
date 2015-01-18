#!/bin/bash

PLS_MY_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

if [ ! -e $PLS_MY_DIR'/envvars.sh' ] ; then
  echo "[FAIL] Could not load envvars.sh"
  exit 1
fi
source $PLS_MY_DIR'/envvars.sh'

# The compilation process of the c program is cross-platform but
# at the moment we use the python version on unix systems
PLS_RSC_OUTPUT='./bin/resCompiler'

PLS_USE_C='false'
if [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ] ; then
  PLS_USE_C='true'
elif [ "$1" == "-c" ] ; then
  PLS_USE_C='true'
fi

if [ "$PLS_USE_C" == "true" ]; then 
  # check if we have gcc
  if hash $PLS_GCC 2>/dev/null; then
    mkdir -p ./bin

    # prepare some variables
    PLS_RSC_INCLUDES='-I ./src/resourceCompiler -I ./src/jsmn'
    PLS_RSC_SOURCE=`ls ./src/resourceCompiler/*.c`' '`ls ./src/jsmn/*.c`
    PLS_RSC_GCC_ARGS='-O2 -std=c99 -Wno-unused-result'
    PLS_RSC_LIBS='-lSDL2main -lSDL2 -lSDL2_image'
    
    # MinGW specific values
    if [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
      PLS_RSC_GCC_ARGS='-DWIN32 -D_WIN32 -mconsole -L '$PLS_DIR_SDL'/lib '$PLS_RSC_GCC_ARGS
      PLS_RSC_INCLUDES='-I '$PLS_DIR_SDL'/include -I '$PLS_DIR_SDL'/include/SDL2 '$PLS_RSC_INCLUDES
      PLS_RSC_LIBS='-lmingw32 '$PLS_RSC_LIBS
      PLS_RSC_OUTPUT=$PLS_RSC_OUTPUT'.exe'

      cp -u -r $PLS_DIR_SDL/bin/* ./bin/
    fi

    # Compile the resource compiler
    if $PLS_GCC $PLS_RSC_GCC_ARGS $PLS_RSC_INCLUDES $PLS_RSC_SOURCE $PLS_RSC_LIBS -o $PLS_RSC_OUTPUT ; then
      exit 0
    else
      echo "[FAIL] Compilation failed"
      exit 1
    fi

  else
    echo "Could not find required program gcc"
    exit 1
  fi

else # if the python version shall be used
  cp -f './src/resourceCompiler/resCompiler.py' $PLS_RSC_OUTPUT
fi
