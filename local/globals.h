#ifndef __GLOBALS_H__
#define __GLOBALS_H__
#include "pebble_os.h"
#include "pebble_fonts.h"
#include "SDL_gfxPrimitives.h"
#include "SDL_gfxBlitFunc.h"

#ifdef WIN32
#include <windows.h>
#else
#define min(A, B) (((A) < (B) ? (A) : (B)))
#define max(A, B) (((A) > (B) ? (A) : (B)))
#define clamp(A, X, B)  min(max(A, X), B)
#endif

#define PI 3.14159265

#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>

#define PBL_SCREEN_WIDTH 144
#define PBL_SCREEN_HEIGHT 168

#define LOCK(X) if(SDL_MUSTLOCK(X)) SDL_LockSurface(X)
#define UNLOCK(X) if(SDL_MUSTLOCK(X)) SDL_UnlockSurface(X)

//#helco
struct GContext
{
	GColor fill_color;
	GColor stroke_color;
	GColor text_color;
	GCompOp compositing_mode;
};

typedef struct SDL_Point
{
    int x,y;
} SDL_Point;

extern void pbl_main(void *params);

//extern SDL_Surface* getSystemScreen ();
extern void unloadSystemFonts ();
extern void defaultRender (AppContextRef app_ctx, PebbleRenderEvent *event);
extern PebbleAppHandlers* getAppHandlers ();
extern void toggle_24h_style ();

                         //AABBGGRR
static const uint32_t r_white = 0xffffffff;
static const uint32_t r_black = 0x000000ff;
static const uint32_t r_clear = 0x00000000;

extern uint32_t getRawColor(uint8_t color);

extern SDL_Color s_white;
extern SDL_Color s_black;
extern SDL_Color s_clear;

extern SDL_Color getColor(uint8_t color);
extern void sdlrect_clip (SDL_Rect* const rect_to_clip,const SDL_Rect* const clipper);

extern FILE* logFile;

//BUTTONS BUTTONS BUTTONS BUTTONS BUTTONS BUTTONS BUTTONS BUTTONS BUTTONS BUTTONS
void onButtonDown (ButtonId id);
void onButtonUp (ButtonId id);
void updateButtons ();
void buttonsUpdateWindow (Window* w);
void initButtons ();
ClickConfig** getClickConfig ();

//HARDWARE OUTPUT HARDWARE OUTPUT HARDWARE OUTPUT HARDWARE OUTPUT HARDWARE OUTPUT
#define MAX_VIBE_ELEMENTS 32
#define VIBE_SHORT_MS 300
#define VIBE_LONG_MS 500
#define LIGHT_LENGTH 2000

bool getVibeState ();
bool getLightState ();

void initHardwareOutput ();
void updateHardwareOutput ();

//RENDER RENDER RENDER RENDER RENDER RENDER RENDER RENDER RENDER RENDER
#define MAX_WINDOWS 8
#define MAX_SCREEN_POOL 8
#define MIN_SCREEN_POOL 4
#define MAX_RENDER_LIST 12

bool initRender (SDL_Surface* screen);
void quitRender ();

void pushWindow (Window* w);
void popWindow ();
int getWindowStackSize ();

SDL_Surface* pushScreen ();
void popScreen ();
SDL_Surface* getTopScreen ();

void pushLayer (Layer* l);
void popLayer ();
void markDirty (bool toggle);
GPoint getTopOffset ();

bool render ();

//ANIMATION ANIMATION ANIMATION ANIMATION ANIMATION ANIMATION ANIMATION
void updateAnimations ();

//RESHELPER RESHELPER RESHELPER RESHELPER RESHELPER RESHELPER RESHELPER
#define MAX_RESOURCE_NAME 32
#define RESOURCE_NAME_BASE "./resources/%d"
void copyResName (char* name,int id);
#endif //__GLOBAS_H__
