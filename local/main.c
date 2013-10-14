#include "globals.h"

#define SCREEN_WIDTH 350
#define SCREEN_HEIGHT 394
#define BODY_WIDTH 219
#define BODY_HEIGHT 394
#define BODY_COUNT 5
#define BODY_SRC_OFFSET_X 6
#define BODY_OFFSET_X (SCREEN_WIDTH/2-BODY_WIDTH/2)
#define BODY_OFFSET_Y 0
#define PBL_SCREEN_OFFSET_X (38+BODY_OFFSET_X)
#define PBL_SCREEN_OFFSET_Y (105+BODY_OFFSET_Y)

#define KEY_BUTTON_BACK SDLK_BACKSPACE
#define KEY_BUTTON_UP SDLK_UP
#define KEY_BUTTON_SELECT SDLK_RETURN
#define KEY_BUTTON_DOWN SDLK_DOWN
#define BUTTON_PRESSED_OFFSET 3 //*-1 for UP, SELECT and DOWN

#define LOG_FILE "log.txt"

const SDL_Rect buttonRects [NUM_BUTTONS]= {
    {0,114,6,76},
    {225,79,6,77},
    {225,172,6,48},
    {225,236,6,77}
};
const SDL_Rect vibeBaseRect= {0,0,60,BODY_HEIGHT};
const SDL_Rect lightRect= {0,0,SCREEN_WIDTH,SCREEN_HEIGHT};

static PebbleAppHandlers _PebbleAppHandlers= {0};
static PblTm _then={0};
static SDL_Surface* pebbleScreen=0,* screen=0,* bodyImg,* shadowImg,* vibeImg,* lightImg;
static float elapsed=0.0f;
static int bodyID=1;
static bool shadow=true;
static bool buttonState [NUM_BUTTONS]= {0,0,0,0};
static bool bodyRender=true;
static bool lastVibeState=false;
static bool lastLightState=false;
static char titleBuffer[]="./simdata/screenshots/yyyy-mm-dd-hh-mm.bmp\0\0";

FILE* logFile=0;

PebbleAppHandlers* getAppHandlers() {
    return &_PebbleAppHandlers;
}
float getTimeElapsed () {
    return elapsed;
}

void simulatorRender ();

int main(int argc,char* argv[]) {
    printf("[INFO] Entering main\n");
    uint32_t flags=SDL_INIT_VIDEO;
#ifndef WIN32
    flags|=SDL_INIT_EVENTTHREAD;
#endif
#ifdef __MACH__
	flags = SDL_INIT_EVERYTHING;
#endif
    if (SDL_Init(flags)==-1) {
        printf("SDL_Init: %s\n", SDL_GetError ());
        return -1;
    }
    screen=SDL_SetVideoMode(SCREEN_WIDTH,SCREEN_HEIGHT, 32, SDL_SWSURFACE);
    if(screen == NULL) {
        printf("SDL_SetVideoMode failed!\n");
        return -2;
    }
    SDL_WM_SetCaption ("Pebble Local Simulator - 24H Style",0);
    pebbleScreen = createScreen;

    if(TTF_Init()==-1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        return -3;
    }

    if (IMG_Init (IMG_INIT_PNG)==-1) {
        printf("IMG_Init: %s\n", IMG_GetError());
        return -4;
    }

    bodyImg=IMG_Load("./simdata/images/body.png");
    if (!bodyImg) {
        printf ("IMG_Load (body): %s\n",IMG_GetError());
        return -5;
    }
    shadowImg=IMG_Load("./simdata/images/shadow.png");
    if (!shadowImg) {
        printf ("IMG_Load (shadow): %s\n",IMG_GetError());
        return -6;
    }
    vibeImg=IMG_Load("./simdata/images/vibe.png");
    if (!vibeImg) {
        printf ("IMG_Load (vibe): %s\n",IMG_GetError());
        return -7;
    }
    lightImg=IMG_Load("./simdata/images/backlight.png");
    if (!lightImg) {
        printf ("IMG_Load (backlight): %s\n",IMG_GetError());
        return -8;
    }

    logFile=fopen (LOG_FILE,"a");

    if (!initRender(pebbleScreen))
        return -9;
    initHardwareOutput ();
    initButtons();
    pbl_main(NULL);
    unloadSystemFonts ();
    quitRender();

    if (logFile!=0)
        fclose(logFile);

    SDL_FreeSurface(lightImg);
    SDL_FreeSurface(vibeImg);
    SDL_FreeSurface(shadowImg);
    SDL_FreeSurface(bodyImg);

    IMG_Quit ();
    TTF_Quit ();
    SDL_Quit ();
    return 0;
}

void tick(bool firstTick) {
    PblTm time;
    PebbleTickEvent event;
    get_time(&time);
    TimeUnits units=_PebbleAppHandlers.tick_info.tick_units;
    TimeUnits changed=0;

    if (!firstTick) {
        if ((units&SECOND_UNIT) >0 && time.tm_sec!=_then.tm_sec)
            changed |= SECOND_UNIT;
        if ((units&MINUTE_UNIT) >0 && time.tm_min!=_then.tm_min)
            changed |= MINUTE_UNIT;
        if ((units&HOUR_UNIT) >0 && time.tm_hour!=_then.tm_hour)
            changed |= HOUR_UNIT;
        if ((units&DAY_UNIT) >0 && time.tm_yday!=_then.tm_yday)
            changed |= DAY_UNIT;
        if ((units&MONTH_UNIT) >0 && time.tm_mon!=_then.tm_mon)
            changed |= MONTH_UNIT;
        if ((units&YEAR_UNIT) >0 && time.tm_year!=_then.tm_year)
            changed |= YEAR_UNIT;
    }

    if (firstTick||changed!=0) {
        _then = time;
        event.tick_time=&_then; //_then lifes longer
        event.units_changed=_PebbleAppHandlers.tick_info.tick_units;
        (_PebbleAppHandlers.tick_info.tick_handler)(app_get_current_graphics_context(), &event);
    }
}

void app_event_loop(AppTaskContextRef app_task_ctx, PebbleAppHandlers *handlers) {
    bool isRunning=true;
    SDL_Event event;
    uint32_t lastTick;
    int32_t delay;
    PblTm now;
    printf("[DEBUG] Got app_event_loop\n");
    _PebbleAppHandlers = *handlers;
    if (_PebbleAppHandlers.init_handler!=NULL)
        (_PebbleAppHandlers.init_handler)(app_get_current_graphics_context());
    if(_PebbleAppHandlers.tick_info.tick_handler != NULL)
        tick (true);
    while (isRunning) {
        lastTick=SDL_GetTicks ();
        SDL_PumpEvents ();
        while (SDL_PollEvent (&event)) {
            switch (event.type) {
            case(SDL_QUIT): {
                isRunning=false;
            }
            break;
            case(SDL_KEYDOWN): {
                switch (event.key.keysym.sym) {
                case(SDLK_ESCAPE): {
                    isRunning=false;
                }
                break;
                case(SDLK_F1): {
                    bodyID=(bodyID+1)%BODY_COUNT;
                    bodyRender=true;
                }
                break;
                case(SDLK_F2): {
                    shadow=!shadow;
                    markDirty(true);
                    bodyRender=true;
                }
                break;
                case(SDLK_F3): {
                    toggle_24h_style();
                    if (clock_is_24h_style ())
                        SDL_WM_SetCaption("Pebble Local Simulator - 24H Style",0);
                    else
                        SDL_WM_SetCaption("Pebble Local Simulator - 12H Style",0);
                }
                break;
                case(SDLK_F12): {
                    get_time (&now);
                    string_format_time (titleBuffer,strlen(titleBuffer),"./simdata/screenshots/%Y-%m-%e-%H-%M-%S",&now);
                    strcat(titleBuffer,".bmp");
                    if (SDL_SaveBMP(screen,titleBuffer)<0)
                        printf("[WARN] SDL_SaveBMP: %s\n",SDL_GetError ());
                    else
                        printf ("[INFO] Saved screenshot: %s\n",titleBuffer);
                }
                break;
                case (KEY_BUTTON_BACK): {
                    buttonState[BUTTON_ID_BACK]=true;
                    bodyRender=true;
                    onButtonDown(BUTTON_ID_BACK);
                }
                break;
                case (KEY_BUTTON_UP): {
                    buttonState[BUTTON_ID_UP]=true;
                    bodyRender=true;
                    onButtonDown(BUTTON_ID_UP);
                }
                break;
                case (KEY_BUTTON_SELECT): {
                    buttonState[BUTTON_ID_SELECT]=true;
                    bodyRender=true;
                    onButtonDown(BUTTON_ID_SELECT);
                }
                break;
                case (KEY_BUTTON_DOWN): {
                    buttonState[BUTTON_ID_DOWN]=true;
                    bodyRender=true;
                    onButtonDown(BUTTON_ID_DOWN);
                }
                break;
                default:
                    ;
                }
            }
            break;
            case(SDL_KEYUP): {
                switch (event.key.keysym.sym) {
                case(KEY_BUTTON_BACK): {
                    buttonState[BUTTON_ID_BACK]=false;
                    bodyRender=true;
                    onButtonUp(BUTTON_ID_BACK);
                }
                break;
                case(KEY_BUTTON_UP): {
                    buttonState[BUTTON_ID_UP]=false;
                    bodyRender=true;
                    onButtonUp(BUTTON_ID_UP);
                }
                break;
                case(KEY_BUTTON_SELECT): {
                    buttonState[BUTTON_ID_SELECT]=false;
                    bodyRender=true;
                    onButtonUp(BUTTON_ID_SELECT);
                }
                break;
                case(KEY_BUTTON_DOWN): {
                    buttonState[BUTTON_ID_DOWN]=false;
                    bodyRender=true;
                    onButtonUp(BUTTON_ID_DOWN);
                }
                break;
                default:
                    ;
                }
            }
            break;
            default:
                ;
            }
        }

		fire_timers();
        if(_PebbleAppHandlers.tick_info.tick_handler != NULL)
            tick (false);
        updateButtons();
        updateAnimations ();
        updateHardwareOutput();
        if (lastVibeState!=getVibeState()) {
            bodyRender=true;
            lastVibeState=getVibeState();
        }
        if (lastLightState!=getLightState()) {
            bodyRender=true;
            lastLightState=getLightState();
        }

        if (render ()||bodyRender)
            simulatorRender ();

        delay=(SDL_GetTicks()-lastTick);
        elapsed=delay/1000.0f;
        delay=16-delay; //equals about 60 frames per second
        lastTick=SDL_GetTicks();
        if (delay>0)
            SDL_Delay(delay);
    }
    if (_PebbleAppHandlers.deinit_handler!=NULL)
        (_PebbleAppHandlers.deinit_handler)(app_get_current_graphics_context());
}

void simulatorRender() {
    SDL_Rect src,dst;
    int i;
    SDL_FillRect (screen,0,0);
    //buttons
    for (i=0; i<NUM_BUTTONS; i++) {
        src=buttonRects[i];
        dst=src;
        src.y+=bodyID*BODY_HEIGHT;
        dst.x+=BODY_OFFSET_X-dst.w;
        dst.y+=BODY_OFFSET_Y;
        if (buttonState[i]==true)
            dst.x+=BUTTON_PRESSED_OFFSET*(i!=BUTTON_ID_BACK?-1:1);
        SDL_BlitSurface(bodyImg,&src,screen,&dst);
    }
    //vibes
    if (lastVibeState) {
        src=vibeBaseRect;
        dst=vibeBaseRect;
        SDL_BlitSurface(vibeImg,&src,screen,&dst);
        src.x+=src.w;
        dst.x=SCREEN_WIDTH-dst.w;
        SDL_BlitSurface(vibeImg,&src,screen,&dst);
    }
    //Body
    src=(SDL_Rect) {
        BODY_SRC_OFFSET_X,bodyID*BODY_HEIGHT,BODY_WIDTH,BODY_HEIGHT
    };
    dst=(SDL_Rect) {
        BODY_OFFSET_X,BODY_OFFSET_Y,BODY_WIDTH,BODY_HEIGHT
    };
    SDL_BlitSurface(bodyImg,&src,screen,&dst);
    //screen
    src=(SDL_Rect) {
        0,0,PBL_SCREEN_WIDTH,PBL_SCREEN_HEIGHT
    };
    dst=(SDL_Rect) {
        PBL_SCREEN_OFFSET_X,PBL_SCREEN_OFFSET_Y,PBL_SCREEN_WIDTH,PBL_SCREEN_HEIGHT
    };
    SDL_BlitSurface(pebbleScreen,&src,screen,&dst);
    //shadow
    if (shadow)
        SDL_BlitSurface(shadowImg,&src,screen,&dst);
    //backlight
    if (lastLightState) {
        src=lightRect;
        dst=lightRect;
        SDL_BlitSurface(lightImg,&src,screen,&dst);
    }
    SDL_Flip (screen);
    bodyRender=false;
}
