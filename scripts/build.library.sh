#!/bin/bash

PLS_MY_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
PLS_LIB_CACHE=$2

if [ ! -e $PLS_MY_DIR'/envvars.sh' ] ; then
  echo "[FAIL] Could not load envvars.sh"
  exit 1
fi
source $PLS_MY_DIR'/envvars.sh'

# check if we have gcc and ar
if hash $PLS_GCC 2>/dev/null; then
  if hash $PLS_AR 2>/dev/null; then

    # prepare some variables
    PLS_SIM_INCLUDES='-isystem ./include -I ./src/local -I ./src/local/dummy -I ./src/SDL_gfx -I ./src/jsmn'

    PLS_SIM_OUTPUT='./bin/libPLocalSim.a'
    PLS_SIM_TMP_OUT='./obj'
    PLS_SIM_GCC_ARGS='-c -std=c99 '

    if [ "$1" == "--debug" ]; then
      PLS_SIM_GCC_ARGS=$PLS_SIM_GCC_ARGS' -ggdb'
    else
      PLS_SIM_GCC_ARGS=$PLS_SIM_GCC_ARGS' -O2'
    fi
    
    # MinGW specific values
    if [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
      PLS_SIM_GCC_ARGS='-DWIN32 -D_WIN32 -mconsole '$PLS_SIM_GCC_ARGS
      PLS_SIM_INCLUDES='-I '$PLS_DIR_SDL'/include -isystem '$PLS_DIR_SDL'/include/SDL2 '$PLS_SIM_INCLUDES
    fi

    # create directories and delete old object files
    mkdir -p ./bin
    mkdir -p $PLS_SIM_TMP_OUT
    filelist=$PLS_SIM_TMP_OUT'/*.o'
    
    # how to compile a directory, params:
    #   directory to compile
    #   'silent' if you don't want to output every filename
    #   extra compile options
    function compileDirectory {
      filelist=$1'/*.c'
      for file in `ls $filelist` ; do
        filename=${file##*/} # without the directory
        objectFile=$PLS_SIM_TMP_OUT'/'${filename%.*}'.o'
        hashFile=$PLS_LIB_CACHE'/'${filename%.*}'.md5' 

        dobuildfile='true'
        error='true'

        # Check cache
        if [ ! -z "$PLS_LIB_CACHE" ] ; then
          if [ -e "$objectFile" ] ; then 
            if [ -e "$hashFile" ] ; then
              savedhash=$(<$hashFile)
              calcedhash=`date -r $file +%s``date -r $objectFile +%s`
              if [ "$savedhash" == "$calcedhash" ] ; then
                dobuildfile='false'
                if [ ! 'silent' = "$2" ] ; then
                  echo Using cached $filename 
                fi
                if $PLS_AR rcs $PLS_SIM_OUTPUT $objectFile ; then
                  error='false'
                fi
              fi
            fi
          fi
        fi       

        # Compile file
        if [ "$dobuildfile" == "true" ]; then   
          if [ ! 'silent' = "$2" ] ; then
            echo Compiling $filename
          fi    
          if $PLS_GCC $PLS_SIM_GCC_ARGS $3 $PLS_SIM_INCLUDES $file -o $objectFile ; then
            if $PLS_AR rcs $PLS_SIM_OUTPUT $objectFile ; then
               error='false'

               # Update cache
               if [ ! -z "$PLS_LIB_CACHE" ] ; then
                 newhash=`date -r $file +%s``date -r $objectFile +%s`
                 touch "$hashFile"
                 echo "$newhash" > "$hashFile"
               fi
            fi
          fi
        fi

        if [ 'true' = "$error" ] ; then
          echo Compilation failed
          rm -f $PLS_SIM_OUTPUT
          exit 1
        fi

      done
      return 1
    }

    # Compile the library
    compileDirectory ./src/local 'no' '-Wall -Wno-format'

    echo Compiling SDL_gfx 
    compileDirectory './src/SDL_gfx' 'silent' '-Wall'
    
    echo Compiling jsmn
    compileDirectory './src/jsmn' 'silent' '-w'

    if [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
      echo Compiling additional source
      compileDirectory './src/additional' 'silent' '-w'
    fi

    exit 0

  else
    echo "Could not find required program ar"
    exit 1
  fi
else
  echo "Could not find required program gcc"
  exit 1
fi
