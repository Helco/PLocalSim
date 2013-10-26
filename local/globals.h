#ifndef __GLOBALS_H__
#define __GLOBALS_H__
#include "pebble_os.h"
#include "pebble_fonts.h"
#include "pebble_app_info.h"
#include "SDL_gfxPrimitives.h"
#include "SDL_gfxBlitFunc.h"
#include "SDL_rotozoom.h"

#ifdef WIN32
#include <windows.h>
#else
#define min(A, B) (((A) < (B) ? (A) : (B)))
#define max(A, B) (((A) > (B) ? (A) : (B)))
#endif

#define clamp(A, X, B)  min(max(A, X), B)

#define PI 3.14159265
#define DEGtoRAD(deg) ((PI/180.0)*(deg))

#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>

//SYSTEM SYSTEM SYSTEM SYSTEM SYSTEM SYSTEM SYSTEM SYSTEM SYSTEM SYSTEM SYSTEM SYSTEM
#define SCROLL_LAYER_CLICK_DELAY 350
#define SCROLL_LAYER_SCROLL_AMOUNT 25
#define SCROLL_LAYER_SCROLL_DELAY 360
#define SCROLL_LAYER_SCROLL_DELAY_MAX 350
#define SCROLL_LAYER_SCROLL_SPEED ((double)SCROLL_LAYER_SCROLL_DELAY/SCROLL_LAYER_SCROLL_AMOUNT)

#define APP_INFO __pbl_app_info

extern void pbl_main(void *params);
extern PebbleAppInfo __pbl_app_info;

void unloadSystemFonts ();
void defaultRender (AppContextRef app_ctx, PebbleRenderEvent *event);
PebbleAppHandlers* getAppHandlers ();
void toggle_24h_style ();

GPoint getPivotRotationOffset (GSize rectOrig,GSize rectRotated,GPoint pivot,double angle);

extern FILE* logFile;

//SIMDATA SIMDATA SIMDATA SIMDATA SIMDATA SIMDATA SIMDATA SIMDATA SIMDATA SIMDATA SIMDATA
enum SimImages
{
    SIM_IMG_BODY=0,
    SIM_IMG_SCREEN_SHADOW,
    SIM_IMG_BACKLIGHT,
    SIM_IMG_STATUS_BAR,
    SIM_IMG_VIBE,
    SIM_IMG_SCROLL_SHADOW,
    SIM_IMG_COUNT
};
bool loadSimulatorImages ();
void freeSimulatorImages ();
SDL_Surface* getSimulatorImage (int imageID);

//DRAW DRAW DRAW DRAW DRAW DRAW DRAW DRAW DRAW DRAW DRAW DRAW DRAW DRAW DRAW DRAW DRAW
#define PBL_SCREEN_WIDTH 144
#define PBL_SCREEN_HEIGHT 168

#define LOCK(X) if(SDL_MUSTLOCK(X)) SDL_LockSurface(X)
#define UNLOCK(X) if(SDL_MUSTLOCK(X)) SDL_UnlockSurface(X)

struct GContext
{
	GColor fill_color;
	GColor stroke_color;
	GColor text_color;
	GCompOp compositing_mode;
};

typedef struct SDL_Point
{
    int32_t x,y;
} SDL_Point;

static const uint32_t r_white = 0xffffffff;
static const uint32_t r_black = 0x000000ff;
static const uint32_t r_clear = 0x00000000;

extern uint32_t getRawColor(uint8_t color);

extern SDL_Color s_white;
extern SDL_Color s_black;
extern SDL_Color s_clear;

SDL_Color getColor(uint8_t color);
void sdlrect_clip (SDL_Rect* const rect_to_clip,const SDL_Rect* const clipper);

void graphics_draw_bitmap_in_rect_to(GContext* ctx,const GBitmap* bitmap,GRect,SDL_Surface* to);
void graphics_draw_surface_in_rect (GContext* ctx,SDL_Surface* sur,GRect rect);
void graphics_draw_surface_in_rect_to (GContext* ctx,SDL_Surface* sur,GRect rect,SDL_Surface* to);

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

#define createSurface(w,h) (SDL_CreateRGBSurface (SDL_SWSURFACE|SDL_SRCALPHA,w,h,32,0xff000000,0x00ff0000,0x0000ff00,0x000000ff))
#define createScreen createSurface(PBL_SCREEN_WIDTH,PBL_SCREEN_HEIGHT)

bool initRender (SDL_Surface* screen);
void quitRender ();

void pushWindow (Window* w);
void popWindow ();
int getWindowStackSize ();

SDL_Surface* pushScreen ();
void popScreen ();
SDL_Surface* getTopScreen ();
void meltScreens ();

void pushLayer (Layer* l);
void popLayer ();
void markDirty (bool toggle);
GPoint getTopOffset ();
void setTopOffset (GPoint set);

bool render ();

//ADDITIONAL PEBBLE OS ADDITIONAL PEBBLE OS ADDITIONAL PEBBLE OS
//functions, the original pebble API might have but doesn't
void rotbmp_layer_init (RotBitmapLayer* layer,GRect frame);
void rotbmp_pair_layer_init (RotBmpPairLayer* layer,GRect frame);
void gpoint_move_into_rect (GPoint* const point,const GRect* const rect);

//ANIMATION ANIMATION ANIMATION ANIMATION ANIMATION ANIMATION ANIMATION
void updateAnimations ();

//TIMERS

typedef struct TimerEvent
{
	uint32_t timeout;
	uint32_t cookie;
	uint32_t handle;
	struct TimerEvent *next;
} TimerEvent;


void fire_timers();
bool remove_timer(TimerEvent *timer);
TimerEvent* search_timer_with_handle(uint32_t handle);
void create_new_timer(uint32_t timeout_ms, uint32_t cookie, uint32_t handle);



//RESHELPER RESHELPER RESHELPER RESHELPER RESHELPER RESHELPER RESHELPER
#define MAX_RESOURCE_NAME 32
#define RESOURCE_NAME_BASE "./resources/%d"
void copyResName (char* name,int id);
#endif //__GLOBAS_H__
