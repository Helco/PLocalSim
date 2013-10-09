@echo off
call envvars.bat

set PLS_RSC_INCLUDES=-I ..\..\resourceCompiler\ %PLS_SDL_INCLUDES%
set PLS_RSC_SOURCE=..\..\resourceCompiler\*.c
set PLS_RSC_LIBS=-L %PLS_SDL_LIB% -lmingw32 %PLS_SDL_MAIN% -lSDL -lSDL_image
set PLS_RSC_OUTPUT=.\bin\resCompiler.exe

%PLS_MINGW%\gcc -std=c99 -x c -Wall -mconsole -DWIN32 %PLS_RSC_INCLUDES% %PLS_RSC_SOURCE% %PLS_RSC_LIBS% -o %PLS_RSC_OUTPUT%