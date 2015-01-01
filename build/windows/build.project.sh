#!/bin/bash

if [ -z "$PLS_SDK_DIR" ] ; then
	echo Please build your project by running ./tools/build.local.sh in your project folder!
else
	DIR=$PLS_SDK_DIR
	source $PLS_SDK_DIR'/envvars.sh'
	"$PLS_SDK_DIR"'/bin/resCompiler.exe'
	# if [ '1' = "$?" ] ; then
		# Prepare variables
		PLS_SIM_LIB='-L '$PLS_SDL_LIB' -L '$PLS_SDK_DIR'/bin -lPebbleLocalSim'
		PLS_PROJECT_NAME=$(<pebble_app.ld)
		PLS_SIMULATOR_EXE='.'$PLS_SIM_OUTPUT'/'$PLS_PROJECT_NAME'.exe'
		PLS_SIM_TMP_DIR=./build/tempLocal/
		PLS_SIM_OBJ_DIR=./build/tempLocal/obj/
		PLS_SIM_DIR_SOURCE=./src/
		PLS_SIM_SOURCE_OBJ=""
		PLS_SIM_INCLUDES='-isystem '$PLS_SDK_INCLUDE' -I '$PLS_SDL_INCLUDE'/SDL'
		PLS_SIM_INCLUDES2='-I '$PLS_SIM_DIR_SOURCE' -I '$PLS_SIM_TMP_DIR' -I '$PLS_SIM_TMP_DIR'src/'
		PLS_SIM_WINRES_PATH=$PLS_SIM_TMP_DIR'winres'
		PLS_GCC_STD_ARGS='-c -O2 -Wall -std=c99 -mconsole -DLOCALSIM'
		PLS_GCC_PATH=$PLS_MINGW'/gcc.exe'
		
		mkdir -p "$PLS_SIM_OBJ_DIR"
		filelist="$PLS_SIM_DIR_SOURCE"'/*.c'
		for file in `ls $filelist` ; do
			filename=${file##*/} # without the directory
			echo 'Compiling '$filename
			objectFile=$PLS_SIM_OBJ_DIR${filename%.*}'.o'
			$PLS_GCC_PATH $PLS_GCC_STD_ARGS -DWIN32 $PLS_SIM_INCLUDES $PLS_SIM_INCLUDES2 $file -o $objectFile
			if [ ! -e "$objectFile" ] ; then
				exit
			fi
			PLS_SIM_SOURCE_OBJ=$PLS_SIM_SOURCE_OBJ' '$objectFile
		done

		echo Linking
		$PLS_GCC_PATH $PLS_SIM_INCLUDES $PLS_SIM_INCLUDES2 $PLS_SDL_MAIN $PLS_SIM_SOURCE_OBJ $PLS_SIM_LIB $PLS_SIM_LIBS -o $PLS_SIMULATOR_EXE

	# else
	# 	echo "Could not compile resources!"
	# fi
fi