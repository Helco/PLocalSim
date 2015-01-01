#!/bin/bash
# Prepare for compiling
source ./envvars.sh
PLS_SIM_INCLUDES='-I '$PLS_SDL_INCLUDE' -isystem '$PLS_SDK_INCLUDE' -I '$PLS_DIR_LOCAL' -I '$PLS_DIR_LOCAL'/dummy -I '$PLS_DIR_SDL_GFX
PLS_SIM_OUTPUT='./bin/libPebbleLocalSim.a'
PLS_SIM_DIR_OUTPUT='./obj'
mkdir -p $PLS_SIM_DIR_OUTPUT
mkdir -p ${PLS_SIM_OUTPUT%/*}
filelist=$PLS_SIM_DIR_OUTPUT'/*'
rm -f $filelist

# Params:
#   1. directory to compile
#   2. 'silent' if you don't want to output every filename
#   3. extra compile options
function compileDirectory {
	filelist=$1'/*.c'
	for file in `ls $filelist` ; do
		filename=${file##*/} # without the directory
		objectFile=$PLS_SIM_DIR_OUTPUT'/'${filename%.*}'.o'
		if [ ! 'silent' = "$2" ] ; then
			echo Compiliing $filename
		fi
		$PLS_MINGW'/gcc' -c -x c -O2 $3 -std=c99 -mconsole $PLS_SIM_INCLUDES $file -o $objectFile
		if [ ! -e "$objectFile" ] ; then
			return 0
		fi
		$PLS_MINGW'/ar' rcs $PLS_SIM_OUTPUT $objectFile
	done
	return 1
}

# Compile the library
compileDirectory "$PLS_DIR_LOCAL" "$1" '-Wall -Wno-format -DWIN32 -D_WIN32'
if [ '1' = "$?" ] ; then
	if [ ! 'silent' = "$1" ] ; then echo Compiling SDL_gfx ; fi
	
	compileDirectory $PLS_DIR_SDL_GFX 'silent' '-w'
	if [ '1' = "$?" ] ; then
		if [ ! 'silent' = "$1" ] ; then echo Compiling additional source ; fi
		
		compileDirectory $PLS_DIR_ADD_SRC 'silent' '-w'
		if [ '0' = "$?" ] ; then
			echo Compilation failed
		fi
	else
		echo Compilation failed
	fi
else
	echo Compilation failed
fi