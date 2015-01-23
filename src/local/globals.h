#ifndef __GLOBALS_H__
#define __GLOBALS_H__
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "impl_pebble.h"

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
#include <stdio.h>
#include <stdarg.h>

//SYSTEM SYSTEM SYSTEM SYSTEM SYSTEM SYSTEM SYSTEM SYSTEM SYSTEM SYSTEM SYSTEM SYSTEM
#define SCROLL_LAYER_CLICK_DELAY 350
#define SCROLL_LAYER_SCROLL_AMOUNT 25
#define SCROLL_LAYER_SCROLL_DELAY 360
#define SCROLL_LAYER_SCROLL_DELAY_MAX 350
#define SCROLL_LAYER_SCROLL_SPEED ((double)SCROLL_LAYER_SCROLL_DELAY/SCROLL_LAYER_SCROLL_AMOUNT)

extern int pbl_main (void);

//#define APP_INFO __pbl_app_info
//extern PebbleAppInfo __pbl_app_info;
void toggle_24h_style ();
void toggle_bluetooth_connection ();
void toggle_battery_charger_plugged ();

uint8_t battery_charge_increase(void);
uint8_t battery_charge_decrease(void);

void accel_do_tap_on_axis(AccelAxisType axis, int32_t direction);

bool persistent_storage_load();
bool persistent_storage_save();
void persistent_storage_free();

bool setup_js_app();

//WATCHINFO WATCHINFO WATCHINFO WATCHINFO WATCHINFO WATCHINFO WATCHINFO

#define WINDOW_WIDTH 390
#define WINDOW_HEIGHT 440
#define BODY_WIDTH 275
#define BODY_HEIGHT 440
#define BODY_OFFSET_X (WINDOW_WIDTH/2 - BODY_WIDTH/2)
#define BODY_OFFSET_Y 0
#define PBL_SCREEN_OFFSET_X (BODY_OFFSET_X + 65)
#define PBL_SCREEN_OFFSET_Y (BODY_OFFSET_Y + 121)

void drawBody (SDL_Surface* screen, uint8_t buttonState);
void nextBody ();

//SERVICES SERVICES SERVICES SERVICES SERVICES SERVICES SERVICES SERVICES
typedef void (*ServiceUpdateCallback) (void);
enum SimServices {
    SIM_SERVICE_BUTTONS=0,
    SIM_SERVICE_HARDWARE_OUTPUT,
    SIM_SERVICE_ANIMATIONS,
    SIM_SERVICE_TIMERS,
    SIM_SERVICE_TICKS,
    SIM_SERVICE_BLUETOOTH,
    SIM_SERVICE_BATTERY,
    SIM_SERVICE_ACCEL_TAP,
    SIM_SERVICE_APP_MESSAGE,
    SIM_SERVICE_COUNT
};
typedef struct {
    struct {
        TimeUnits units;
        TickHandler handler;
    } ticks;
    ServiceUpdateCallback services[SIM_SERVICE_COUNT];
} ServiceData;
extern ServiceData serviceData;
void service_buttons ();
void service_hardware_output ();
void service_animations ();
void service_timers ();
void service_ticks ();
void service_bluetooth ();
void service_battery ();
void service_accel_tap ();
void service_app_message();
void service_app_sync(DictionaryIterator*);

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
void unloadSystemFonts ();
extern FILE* logFile;

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
GContext *app_get_current_graphics_context(void);
// Used to either draw or calculate size
GSize _graphics_draw_text(GContext *ctx, const char *text, const GFont font, const GRect box, const GTextOverflowMode overflow_mode, const GTextAlignment alignment, const GTextLayoutCacheRef layout, int getSizeOnly);
int lineHeightFromFont(GFont font);

//BUTTONS BUTTONS BUTTONS BUTTONS BUTTONS BUTTONS BUTTONS BUTTONS BUTTONS BUTTONS
void onButtonDown (ButtonId id);
void onButtonUp (ButtonId id);
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

//RENDER RENDER RENDER RENDER RENDER RENDER RENDER RENDER RENDER RENDER
#define MAX_WINDOWS 8
#define MAX_SCREEN_POOL 8
#define MIN_SCREEN_POOL 4
#define MAX_RENDER_LIST 12

#define createSurface(w,h) (SDL_CreateRGBSurface (0,w,h,32,0xff000000,0x00ff0000,0x0000ff00,0x000000ff))
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

//TIMERS
void freeTimers();

//RESHELPER RESHELPER RESHELPER RESHELPER RESHELPER RESHELPER RESHELPER
#define MAX_RESOURCE_NAME 32
#define RESOURCE_NAME_BASE "./resources/%d"
#ifdef __x86_64__
#  define RES_ID_TO_HANDLE(id) ((ResHandle)(uint64_t)(uint32_t)(id))
#else
#  define RES_ID_TO_HANDLE(id) ((ResHandle)(uint32_t)(id))
#endif
void copyResName (char* name,ResHandle h);

//ADDITIONAL PEBBLE OS ADDITIONAL PEBBLE OS ADDITIONAL PEBBLE OS
//functions, the original pebble API might have but doesn't
void gpoint_move_into_rect (GPoint* const point,const GRect* const rect);

//Helper functions
GPoint getPivotRotationOffset (GSize rectOrig,GSize rectRotated,GPoint pivot,double angle);

#endif //__GLOBALS_H_
