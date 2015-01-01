#!/bin/bash
# Prepare for compiling
source ./envvars.sh

PLS_RSC_INCLUDES=$PLS_SDL_INCLUDES' -I ../../resourceCompiler/ '
PLS_RSC_SOURCE='../../resourceCompiler/*.c'
PLS_RSC_LIBS='-L '$PLS_SDL_LIB' -lmingw32 '$PLS_SDL_MAIN' -lSDL -lSDL_image'
PLS_RSC_OUTPUT='./bin/resCompiler.exe'

rm -f $PLS_RSC_OUTPUT
$PLS_MINGW'/gcc' -std=c99 -x c -Wall -mconsole -DWIN32 $PLS_RSC_INCLUDES \
	$PLS_RSC_SOURCE $PLS_RSC_LIBS -o $PLS_RSC_OUTPUT