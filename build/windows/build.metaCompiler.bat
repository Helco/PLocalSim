@echo off
call envvars.bat

set PLS_RSC_INCLUDES=-I ..\..\metadataCompiler\ %PLS_SDL_INCLUDES% -isystem %PLS_SDK_INCLUDE% -I %PLS_DIR_SDL_GFX%
set PLS_RSC_SOURCE=..\..\metadataCompiler\*.c %PLS_DIR_SDL_GFX%\*.c
set PLS_RSC_LIBS=-L %PLS_SDL_LIB% -lmingw32 %PLS_SDL_MAIN% -lSDL -lSDL_image
set PLS_RSC_OUTPUT=.\bin\metaCompiler.exe

%PLS_MINGW%\gcc -std=c99 -x c -Wall -mconsole -DWIN32 %PLS_RSC_INCLUDES% %PLS_RSC_SOURCE% %PLS_RSC_LIBS% -o %PLS_RSC_OUTPUT%