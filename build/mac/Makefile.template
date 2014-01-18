
APP_SRCS = $(wildcard ../src/*.c)

INCS = -Iinclude -Ibuild/tempLocal -I. -I../src

CFLAGS = -std=c11 -arch i386 -g

all: run_resCompiler PebbleSim

debug: run_resCompiler PebbleSim.debug

PebbleSim: $(APP_SRCS)
	cc $(CFLAGS) $(INCS) `sdl-config --libs` -lSDL_image -lSDL_ttf -o $@ $(APP_SRCS) PebbleLocalSim.a

PebbleSim.debug: $(APP_SRCS)
	cc $(CFLAGS) -ggdb $(INCS) `sdl-config --libs` -lSDL_image -lSDL_ttf -o $@ $(APP_SRCS) PebbleLocalSim.a

run_resCompiler:
	( cd .. ; sim/resCompiler )
	cp -r ../build/local/resources .
	cp -r ../build/tempLocal/src .

clean:
	rm -vfr *.o *~ resources src PebbleSim PebbleSim.debug PebbleSim.debug.dSYM log.txt
