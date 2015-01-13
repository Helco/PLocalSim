#!/bin/bash

PLS_MY_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

if [ ! -e $PLS_MY_DIR'/scripts/envvars.sh' ] ; then
  echo "[FAIL] Could not load envvars.sh"
  exit 1
fi
source $PLS_MY_DIR'/scripts/envvars.sh'

# Find target path
PLS_TARGET_PATH=$1
PLS_IS_STANDALONE='true'
if [ -z "$PLS_TARGET_PATH" ] ; then
  if [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ] ; then
    echo "[FAIL] PLocalSim does need a specific directory on windows"
    exit 1
  else
    # try to find the pebble sdk
    PLS_PATH_PEBBLE=`which pebble`
    if [ $? -eq 0 ] ; then
      PLS_TARGET_PATH=`dirname $PLS_PATH_PEBBLE`
      PLS_IS_STANDALONE='false'
    else
      echo "[FAIL] PLocalSim could not locate the pebble sdk"
      exit 1
    fi
  fi
fi

# Check if the sdk is built already
PLS_SIM_LIB=$PLS_MY_DIR'/bin/libPLocalSim.a'
PLS_RSC_EXE=$PLS_MY_DIR'/bin/resCompiler'
PLS_RECOMPILE_RSC='false'
PLS_RSC_BUILD_ARGS=' '
PLS_OS_NAME='unix'
if [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
  PLS_RSC_EXE=$PLS_RSC_EXE'.exe'
  PLS_OS_NAME='windows'
elif [ "$PLS_IS_STANDALONE" == "true" ] ; then
  # Unix systems must use the c resourceCompiler because the python requires
  # the pebble libraries we don't have on stand-alone installations
  PLS_RECOMPILE_RSC='true'
  PLS_RSC_BUILD_ARGS='-c'
fi

if [ ! -e $PLS_SIM_LIB ] ; then
  echo "[INFO] PLocalSim library is not built yet, compiling..."
  if ./scripts/build.library.sh ; then
    echo "[INFO] Sucessfully compiled PLocalSim library"
  else
    echo "[FAIL] Could not compile PLocalSim library"
    exit 1
  fi
fi

if [ ! -e $PLS_RSC_EXE ] ; then
  PLS_RECOMPILE_RSC='true'
fi
if [ "$PLS_RECOMPILE_RSC" == "true" ] ; then
  echo "[INFO] PLocalSim resource compiler is not built yet, compiling..."
  if ./scripts/build.resCompiler.sh $PLS_RSC_BUILD_ARGS ; then
    echo "[INFO] Sucessfully compiled PLocalSim resource compiler"
  else
    echo "[FAIL] Could not compile PLocalSim resource compiler"
    exit 1
  fi
fi

# Create/Test directory
mkdir -p $PLS_TARGET_PATH'/PLocalSim/simdata'
if [ ! -d "$PLS_TARGET_PATH" ] ; then
  echo "[FAIL] Invalid directory: $PLS_TARGET_PATH"
  exit 1
fi

# Copy all of the sdk
echo "[INFO] Copy common sdk data"
cp -f -r $PLS_MY_DIR/bin/*                  $PLS_TARGET_PATH'/PLocalSim/'
cp -f -r $PLS_MY_DIR/sdkdata/common/*       $PLS_TARGET_PATH'/'
cp -f -r $PLS_MY_DIR/simdata/common/*       $PLS_TARGET_PATH'/PLocalSim/simdata/'

echo "[INFO] Copy OS specific data (may take a few minutes)"
cp -f -r $PLS_MY_DIR/sdkdata/$PLS_OS_NAME/* $PLS_TARGET_PATH'/'
cp -f -r $PLS_MY_DIR/simdata/$PLS_OS_NAME/* $PLS_TARGET_PATH'/PLocalSim/simdata/'

if [ "$PLS_IS_STANDALONE" == 'true' ]; then
  echo "[INFO] Copy data for stand-alone installations"
  mkdir -p $PLS_TARGET_PATH'/PLocalSim/include'
  cp -f -r $PLS_MY_DIR/include/*            $PLS_TARGET_PATH'/PLocalSim/include/' 
  cp -f -r $PLS_MY_DIR/sdkdata/standalone/* $PLS_TARGET_PATH'/'
fi
