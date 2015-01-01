#!/bin/bash

if [ -z "$DIR" ] ; then
	DIR=$(pwd)
fi


PLS_MINGW=$DIR'/MinGW/bin'
export PATH=$PATH':'$PLS_MINGW

PLS_SDL_BIN=$DIR'/SDL/bin'
PLS_SDL_INCLUDE=$DIR'/SDL/include'
PLS_SDL_INCLUDES='-I '$DIR'/SDL/include -I '$DIR'/SDL/include/SDL'
PLS_SDL_LIB=$DIR'/SDL/lib'
PLS_SDL_MAIN=$DIR'/SDL/source/SDL_win32_main.c -DNO_STDIO_REDIRECT'

PLS_SDK_INCLUDE=$DIR'/include'
PLS_DIR_LOCAL=$DIR'/../../local'
PLS_DIR_SDL_GFX=$DIR'/../../SDL_gfx'
PLS_DIR_ADD_SRC=$DIR'/../../additionalSource'

PLS_SIM_LIBS='-lmingw32 -lm -lSDL -lSDL_ttf -lSDL_image'
PLS_SIM_OUTPUT='/build/local'