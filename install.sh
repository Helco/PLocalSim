#!/bin/bash

PLS_MY_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

if [ ! -e $PLS_MY_DIR'/envvars.sh' ] ; then
  echo "[FAIL] Could not load envvars.sh"
  exit 1
fi
source $PLS_MY_DIR'/envvars.sh'

# Check if the sdk is built already
PLS_SIM_LIB=$PLS_MY_DIR'/bin/libPLocalSim.a'
PLS_RSC_EXE=$PLS_MY_DIR'/bin/resCompiler'
if [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
  PLS_RSC_EXE=$PLS_RSC_EXE'.exe'
fi

if [ ! -e $PLS_SIM_LIB ] ; then
  echo "[INFO] PLocalSim library is not built yet, compiling..."
  if ./build.library.sh ; then
    echo "[INFO] Sucessfully compiled PLocalSim library"
  else
    echo "[FAIL] Could not compile PLocalSim library"
    exit 1
  fi
fi

if [ ! -e $PLS_RSC_EXE ] ; then
  echo "[INFO] PLocalSim resource compiler is not built yet, compiling..."
  if ./build.resCompiler.sh ; then
    echo "[INFO] Sucessfully compiled PLocalSim resource compiler"
  else
    echo "[FAIL] Could not compile PLocalSim resource compiler"
    exit 1
  fi
fi

# Find target path
PLS_TARGET_PATH=$1
if [ -z "$PLS_TARGET_PATH" ] ; then
  if [ -z "$1" ] ; then
    echo "[FAIL] PLocalSim does need a specific directory on windows"
    exit 1
  else
    # try to find the pebble sdk
    PLS_PATH_PEBBLE=`type -t pebble`
    if [ $? -eq 0 ] ; then
      PLS_TARGET_PATH=`dirname $PLS_PATH_PEBBLE`
    else
      echo "[FAIL] PLocalSim could not locate the pebble sdk"
      exit 1
    fi
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

echo "[INFO] Copy OS specific data"
PLS_OS_NAME="unix"
if [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
  echo "[INFO] May take a few minutes"
  PLS_OS_NAME="windows"
  
  # Copy include directory
  mkdir -p $PLS_TARGET_PATH'/PLocalSim/include'
  cp -f -r $PLS_MY_DIR/include/*            $PLS_TARGET_PATH'/PLocalSim/include/' 
fi
cp -f -r $PLS_MY_DIR/sdkdata/$PLS_OS_NAME/* $PLS_TARGET_PATH'/'
cp -f -r $PLS_MY_DIR/simdata/$PLS_OS_NAME/* $PLS_TARGET_PATH'/PLocalSim/simdata/'
