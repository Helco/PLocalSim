RSRC_SRCS = \
	../../resourceCompiler/jsmn.c \
	../../resourceCompiler/main.c

SIM_SRCS = \
	../../SDL_gfx/SDL_gfxBlitFunc.c \
	../../SDL_gfx/SDL_gfxPrimitives.c \
	../../SDL_gfx/SDL_rotozoom.c \
	../../local/actionbar.c \
	../../local/animation.c \
	../../local/bitmap.c \
	../../local/buttons.c \
	../../local/draw.c \
	../../local/font.c \
	../../local/hardwareOutput.c \
	../../local/jsmn.c \
	../../local/layer.c \
	../../local/main.c \
	../../local/math.c \
	../../local/pebble_os.c \
	../../local/render.c \
	../../local/scrolllayer.c \
	../../local/simdata.c \
	../../local/timer.c \
	../../local/window.c

INCS = \
	-I../../include \
	-I../../local \
	-I../../SDL_gfx \
	-I../../local/dummy

CFLAGS = -std=c11 -arch i386 -D_TIME_T

%.o: %.c
	cc $(CFLAGS) $(INCS) -c $< -o $@

all: resCompiler PebbleLocalSim.a make_sim_dir

resCompiler: $(RSRC_SRCS)
	cc $(CFLAGS) `sdl-config --libs` -lSDL_image -o $@ $(RSRC_SRCS) 

PebbleLocalSim.a: $(SIM_SRCS:.c=.o)
	ar -r $@ $(SIM_SRCS:.c=.o)

make_sim_dir:
	rm -fr sim
	mkdir sim
	mv PebbleLocalSim.a sim
	mv resCompiler sim
	cp -r simdata sim
	cp -r ../../include sim
	cp Makefile.template sim/Makefile

clean:
	rm -fr $(SIM_SRCS:.c=.o) *.o *~
