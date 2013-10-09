@echo off
set PLS_MINGW=.\MinGW\bin

set PLS_SDL_BIN=.\SDL\bin
set PLS_SDL_INCLUDE=.\SDL\include
set PLS_SDL_INCLUDES=-I .\SDL\include -I .\SDL\include\SDL
set PLS_SDL_LIB=.\SDL\lib
set PLS_SDL_MAIN=.\SDL\source\SDL_win32_main.c -DNO_STDIO_REDIRECT

set PLS_SDK_INCLUDE=..\..\include
set PLS_DIR_LOCAL=..\..\local
set PLS_DIR_SDL_GFX=..\..\SDL_gfx

set PLS_SIM_LIBS=-lmingw32 -lm -lSDL -lSDL_ttf -lSDL_image
set PLS_SIM_OUTPUT=.\build\local