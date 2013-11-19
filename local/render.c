#include "globals.h"

#define STATUS_TIME_BUFFER_SIZE 16

typedef struct RenderEntry
{
    SDL_Surface* clipScreen;
    Layer* layer;
} RenderEntry;

static Window* windowStack [MAX_WINDOWS];
static int windowStackSize=0;
static bool isDirty=true;
static SDL_Surface* screenPool [MAX_SCREEN_POOL];
static int screenPoolSize=0;
static int usedScreenPool=0;
static RenderEntry renderList [MAX_RENDER_LIST];
static int renderListSize=0;
static GPoint topOffset;
static SDL_Surface* statusImg;
static const GBitmap* statusBarIcon [MAX_WINDOWS];
static int statusBarIconSize=0;
static char statusTimeBuffer[STATUS_TIME_BUFFER_SIZE]={0};
static GFont statusTimeFont;
static bool statusBarVisible=true;

bool initRender (SDL_Surface* screen)
{
    for (screenPoolSize=0;screenPoolSize<MIN_SCREEN_POOL;screenPoolSize++) {
        screenPool[screenPoolSize]=createScreen;
        if (!screenPool[screenPoolSize]) {
            printf ("Failed to create enough screen buffers!\n");
            return false;
        }
    }
    renderListSize=1;
    renderList[0]=((RenderEntry){screen,0});
    isDirty=true;
    statusImg=getSimulatorImage(SIM_IMG_STATUS_BAR);
    statusTimeFont=fonts_get_system_font (FONT_KEY_GOTHIC_14);
    return (statusTimeFont!=0);
}

void quitRender ()
{
    int i;
    windowStackSize=0;
    isDirty=true;
    screenPoolSize=0;
    for (i=0;i<MAX_WINDOWS;i++)
        windowStack[i]=0;
    for (i=0;i<MAX_SCREEN_POOL;i++) {
        if (screenPool[i]!=0) {
            SDL_FreeSurface(screenPool[i]);
            screenPool[i]=0;
        }
    }
}

void pushWindow (Window* w) {
    if (windowStackSize>0&&windowStack[windowStackSize-1]->window_handlers.disappear!=0)
        windowStack[windowStackSize-1]->window_handlers.disappear(windowStack[windowStackSize-1]);
    windowStack[windowStackSize]=w;
    windowStackSize++;
    //topOffset=w->layer.frame.origin;
    if (!w->is_loaded) {
        if (w->window_handlers.load)
            w->window_handlers.load (w);
        w->is_loaded=true;
    }
    if (w->window_handlers.appear)
        w->window_handlers.appear (w);
    if (w->status_bar_icon) {
        statusBarIcon[statusBarIconSize]=w->status_bar_icon;
        statusBarIconSize++;
    }
    statusBarVisible=!window_stack_get_top_window ()->is_fullscreen;
    buttonsUpdateWindow(window_stack_get_top_window ());
    markDirty(true);
}

void popWindow () {
    Window* w=window_stack_get_top_window ();
    windowStackSize--;
    if (w->window_handlers.disappear)
        w->window_handlers.disappear(w);
    windowStack[windowStackSize]=0;
    if (windowStackSize>0) {
        w=windowStack[windowStackSize-1];
        topOffset=w->layer->frame.origin;
        if (w->window_handlers.appear)
            w->window_handlers.appear(w);
    }
    if (w->status_bar_icon)
        statusBarIconSize--;
    if (windowStackSize>0)
        statusBarVisible=!window_stack_get_top_window ()->is_fullscreen;
    buttonsUpdateWindow(window_stack_get_top_window ());
    markDirty(true);
}

int getWindowStackSize() {
    return windowStackSize;
}

SDL_Surface* pushScreen () {
    if (screenPool[usedScreenPool]==0)
        screenPool[usedScreenPool]=createScreen;
    SDL_Surface* screen=screenPool[usedScreenPool];
    usedScreenPool++;
    SDL_FillRect(screen,0,0x00000000);
    return screen;
}

void popScreen () {
    usedScreenPool--;
}

SDL_Surface* getTopScreen () {
    int i;
    for (i=renderListSize-1;i>=0;i--) {
        if (renderList[i].clipScreen!=0)
            return renderList[i].clipScreen;
    }
    return 0;
}

void pushLayer (Layer* l) {
    renderList[renderListSize].layer=l;
    renderList[renderListSize].clipScreen=pushScreen();
    if (l->clips) {
        SDL_Rect rect={topOffset.x+l->frame.origin.x,topOffset.y+l->frame.origin.y,l->frame.size.w,l->frame.size.h};
        SDL_SetClipRect (renderList[renderListSize].clipScreen,&rect);
    }
    renderListSize++;
    topOffset.x+=l->bounds.origin.x+l->frame.origin.x;
    topOffset.y+=l->bounds.origin.y+l->frame.origin.y;
}

void popLayer () {
    Layer* l=renderList[renderListSize-1].layer;
    topOffset.x-=l->bounds.origin.x+l->frame.origin.x;
    topOffset.y-=l->bounds.origin.y+l->frame.origin.y;
    popScreen ();
    SDL_Surface* src=getTopScreen ();
    renderListSize--;
    SDL_Surface* dst=getTopScreen ();
    SDL_SetClipRect(src,0);
    SDL_gfxBlitRGBA(src,0,dst,0);
}

void markDirty (bool toggle) {
    isDirty=toggle;
}

GPoint getTopOffset () {
    return topOffset;
}

void setTopOffset (GPoint set) {
    topOffset=set;
}

void renderLayer (Layer* l,GContext* ctx) {
    if (l->hidden==true)
        return;
    pushLayer (l);
    Layer* cursor=l->first_child;
    if (l->update_proc)
        l->update_proc (l,ctx);
    while (cursor!=0) {
        renderLayer (cursor,ctx);
        cursor=cursor->next_sibling;
    }
    popLayer ();
}

void window_render (Window* w,GContext* ctx)
{
    Layer* cursor;
    if (w!=0) {
        SDL_FillRect(getTopScreen(),0,getRawColor (w->background_color));
        cursor=w->layer;
        while (cursor!=0) {
            renderLayer(cursor,ctx);
            cursor=cursor->next_sibling;
        }
    }
}

bool render () {
    SDL_Rect src,dst;
    if (isDirty==true) {
        SDL_FillRect(getTopScreen(),0,r_black);
        window_render(window_stack_get_top_window (),app_get_current_graphics_context());
        //Render status bar
        if (statusBarVisible) {
            dst=(SDL_Rect){0,0,144,16};
            SDL_FillRect (getTopScreen (),&dst,r_black);
            if (statusBarIconSize>0) {
                graphics_draw_bitmap_in_rect (app_get_current_graphics_context(),statusBarIcon[statusBarIconSize-1],GRect(8-statusBarIcon[statusBarIconSize-1]->bounds.size.w/2,8-statusBarIcon[statusBarIconSize-1]->bounds.size.h/2,statusBarIcon[statusBarIconSize-1]->bounds.size.w,statusBarIcon[statusBarIconSize-1]->bounds.size.h));
            }
            else {
                src=(SDL_Rect){0,0,16,16};
                dst.w=16;
                SDL_BlitSurface(statusImg,&src,getTopScreen(),&dst);
            }
            src=(SDL_Rect){16,0,16,16};
            dst=(SDL_Rect){144-18,0,16,16};
            SDL_BlitSurface(statusImg,&src,getTopScreen(),&dst);
            clock_copy_time_string(statusTimeBuffer,STATUS_TIME_BUFFER_SIZE);
            graphics_context_set_text_color (app_get_current_graphics_context(),GColorWhite);
            graphics_draw_text (app_get_current_graphics_context(),statusTimeBuffer,statusTimeFont,GRect(0,0,144,16),GTextOverflowModeTrailingEllipsis,GTextAlignmentCenter,0);
        }

        isDirty=false;
        return true;
    }
    return false;
}

bool window_stack_contains_window(Window *window) {
    int i;
    for (i=0;i<windowStackSize;i++) {
        if (windowStack[i]==window)
            return true;
    }
    return false;
}

Window *window_stack_get_top_window(void) {
    if (windowStackSize!=0)
        return windowStack[windowStackSize-1];
    return 0;
}

bool window_stack_remove(Window *window, bool animated) {
    int i,j;
    if (window_stack_get_top_window ()==window) {
        popWindow (); //for animated and window_handlers
        return true;
    }
    else {
        for (i=0;i<windowStackSize;i++) {
            if (windowStack[i]==window) {
                break; //to get the index in i
            }
        }
        if (i==windowStackSize)
            return false;
        for (j=i;j<windowStackSize;j++)
            windowStack[j]=windowStack[j+1];
        return true;
    }
}

void meltScreens () {
    SDL_Rect clipRect;
    SDL_GetClipRect(getTopScreen(),&clipRect);
    SDL_Surface* myScreen=getTopScreen();
    renderList[renderListSize-1].clipScreen=pushScreen();
    SDL_SetClipRect(renderList[renderListSize-1].clipScreen,&clipRect);
    for (int i=0;i<renderListSize;i++)
        SDL_gfxBlitRGBA(renderList[i].clipScreen,0,myScreen,0);
    popScreen();
    renderList[renderListSize-1].clipScreen=myScreen;
}
