#include "globals.h"

#define KEY_BUTTON_BACK SDLK_BACKSPACE
#define KEY_BUTTON_UP SDLK_UP
#define KEY_BUTTON_SELECT SDLK_RETURN
#define KEY_BUTTON_DOWN SDLK_DOWN

#define LOG_FILE "log.txt"

const SDL_Rect buttonRects [NUM_BUTTONS]= {
    {0,114,6,76},
    {225,79,6,77},
    {225,172,6,48},
    {225,236,6,77}
};
const SDL_Rect vibeBaseRect= {0,0,60,BODY_HEIGHT};
const SDL_Rect lightRect= {0,0,WINDOW_WIDTH,WINDOW_HEIGHT};

static SDL_Window* window = 0;
static SDL_Renderer* renderer = 0;
static SDL_Texture* screenTexture = 0;
static SDL_Surface* pebbleScreen=0,* screen=0,* shadowImg,* vibeImg,* lightImg;
static float elapsed=0.0f;
static bool shadow=true;
static uint8_t buttonState = 0;
static bool bodyRender=true;
static bool lastVibeState=false;
static bool lastLightState=false;
static char titleBuffer[]="./screenshots/yyyy-mm-dd-hh-mm.bmp\0\0";

ServiceData serviceData={{0,0},{service_buttons,service_hardware_output,service_animations,service_timers,service_ticks,service_bluetooth,service_battery,service_accel_tap,
#ifndef WIN32
	service_app_message}};
#else
	0}};
#endif

FILE* logFile=0;

float getTimeElapsed () {
    return elapsed;
}

void simulatorRender ();

#undef main
#ifdef WIN32
#  define main SDL_main
#endif

int main(int argc, char* argv[]) {

    printf("[INFO] Entering main\n");
#ifndef WIN32
    SDL_SetMainReady();
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) == -1) {
        printf("SDL_Init: %s\n", SDL_GetError ());
        return -1;
    }

	window = SDL_CreateWindow("PLocalSim - 24H Style",
							  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
							  WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	if (!window) {
		printf("SDL_CreateWindow: %s\n", SDL_GetError());
		return -2;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (!renderer) {
		printf("SDL_CreateRenderer: %s\n", SDL_GetError());
		return -2;
	}

	screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
									  WINDOW_WIDTH, WINDOW_HEIGHT);
	if (!screenTexture) {
		printf("SDL_CreateTexture: %s\n", SDL_GetError());
		return -2;
	}

    screen = createSurface(WINDOW_WIDTH, WINDOW_HEIGHT);
    if(screen == NULL) {
        printf("SDL_CreateRGBSurface failed!\n");
        return -2;
    }
    pebbleScreen = createScreen;

    if(TTF_Init()==-1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        return -3;
    }

    if (IMG_Init (IMG_INIT_PNG)==-1) {
        printf("IMG_Init: %s\n", IMG_GetError());
        return -4;
    }

    if (!loadSimulatorImages())
        return -5;
    shadowImg=getSimulatorImage(SIM_IMG_SCREEN_SHADOW);
    vibeImg=getSimulatorImage(SIM_IMG_VIBE);
    lightImg=getSimulatorImage(SIM_IMG_BACKLIGHT);
    logFile=fopen (LOG_FILE,"a");

    if (!initRender(pebbleScreen))
        return -9;
    persistent_storage_load();
#ifndef WIN32
    if (!setup_js_app()) {
        printf("[ERROR] Failed to Initialize Connection to JS app\n");
        return -10;
	}
#endif

    initHardwareOutput ();
    initButtons();
    pbl_main();
    unloadSystemFonts ();
    quitRender();
    persistent_storage_free();

    if (logFile!=0)
        fclose(logFile);

    freeSimulatorImages();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
    IMG_Quit ();
    TTF_Quit ();
    SDL_Quit ();
    return 0;
}

void app_event_loop() {
    bool isRunning=true;
    SDL_Event event;
    uint32_t lastTick;
    int32_t delay;
    int i;
    struct tm* now;
    printf("[DEBUG] Got app_event_loop\n");
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
                    nextBody();
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
                        SDL_SetWindowTitle(window, "PLocalSim - 24H Style");
                    else
                        SDL_SetWindowTitle(window, "PLocalSim - 12H Style");
                }
                break;
                case(SDLK_F4): {
                    toggle_bluetooth_connection();
                    printf("[INFO] Toggle bluetooth %s\n", bluetooth_connection_service_peek() ? "ON":"OFF");
                }
                break;
                case(SDLK_F5): {
                    BatteryChargeState state;
                    toggle_battery_charger_plugged();
                    state = battery_state_service_peek();
                    printf("[INFO] Toggle plugged: %d%%, %s charging, %s plugged\n",
                           state.charge_percent, state.is_charging ? "":"not", state.is_plugged ? "":"not");
                }
                break;
                case(SDLK_F12): {
                    time_t timeSec=time(0);
                    now=localtime(&timeSec);
                    strftime (titleBuffer,strlen(titleBuffer),"./screenshots/%Y-%m-%e-%H-%M-%S",now);
                    strcat(titleBuffer,".bmp");
                    if (SDL_SaveBMP(screen,titleBuffer)<0)
                        printf("[WARN] SDL_SaveBMP: %s\n",SDL_GetError ());
                    else
                        printf ("[INFO] Saved screenshot: %s\n",titleBuffer);
                }
                break;
                case(SDLK_PLUS): {
                    BatteryChargeState state;
                    battery_charge_increase();
                    state = battery_state_service_peek();
                    printf("[INFO] Battery state: %d%%, %s charging, %s plugged\n",
                           state.charge_percent, state.is_charging ? "":"not", state.is_plugged ? "":"not");
                }
                break;
                case(SDLK_MINUS): {
                    BatteryChargeState state;
                    battery_charge_decrease();
                    state = battery_state_service_peek();
                    printf("[INFO] Battery state: %d%%, %s charging, %s plugged\n",
                           state.charge_percent, state.is_charging ? "":"not", state.is_plugged ? "":"not");
                }
                break;
                case (SDLK_x): {
                    int32_t direction=1;
                    if(event.key.keysym.mod & KMOD_SHIFT) {
                        direction=-1;
                    }
                    accel_do_tap_on_axis(ACCEL_AXIS_X, direction);
                    printf("[INFO] Tap X %d\n", direction);
                }
                break;
                case (SDLK_y): {
                    int32_t direction=1;
                    if(event.key.keysym.mod & KMOD_SHIFT) {
                        direction=-1;
                    }
                    accel_do_tap_on_axis(ACCEL_AXIS_Y, direction);
                    printf("[INFO] Tap Y %d\n", direction);
                }
                break;
                case (SDLK_z): {
                    int32_t direction=1;
                    if(event.key.keysym.mod & KMOD_SHIFT) {
                        direction=-1;
                    }
                    accel_do_tap_on_axis(ACCEL_AXIS_Z, direction);
                    printf("[INFO] Tap Z %d\n", direction);
                }
                break;
                case (KEY_BUTTON_BACK): {
                    buttonState |= 1 << BUTTON_ID_BACK;
                    bodyRender=true;
                    onButtonDown(BUTTON_ID_BACK);
                }
                break;
                case (KEY_BUTTON_UP): {
                    buttonState |= 1 << BUTTON_ID_UP;
                    bodyRender=true;
                    onButtonDown(BUTTON_ID_UP);
                }
                break;
                case (KEY_BUTTON_SELECT): {
                    buttonState |= 1 << BUTTON_ID_SELECT;
                    bodyRender=true;
                    onButtonDown(BUTTON_ID_SELECT);
                }
                break;
                case (KEY_BUTTON_DOWN): {
                    buttonState |= 1 << BUTTON_ID_DOWN;
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
                    buttonState ^= 1<<BUTTON_ID_BACK;
                    bodyRender=true;
                    onButtonUp(BUTTON_ID_BACK);
                }
                break;
                case(KEY_BUTTON_UP): {
                    buttonState ^= 1<<BUTTON_ID_UP;
                    bodyRender=true;
                    onButtonUp(BUTTON_ID_UP);
                }
                break;
                case(KEY_BUTTON_SELECT): {
                    buttonState ^= 1<<BUTTON_ID_SELECT;
                    bodyRender=true;
                    onButtonUp(BUTTON_ID_SELECT);
                }
                break;
                case(KEY_BUTTON_DOWN): {
                    buttonState ^= 1<<BUTTON_ID_DOWN;
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

		for (i=0;i<SIM_SERVICE_COUNT;i++) {
			if (serviceData.services[i])
				(serviceData.services[i]) ();
		}
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
}

void simulatorRender() {
    SDL_Rect src,dst;
    SDL_FillRect (screen,0,0);

    drawBody(screen, buttonState);

    //vibes
    if (lastVibeState) {
        src=vibeBaseRect;
        dst=vibeBaseRect;
        SDL_BlitSurface(vibeImg,&src,screen,&dst);
        src.x+=src.w;
        dst.x=WINDOW_WIDTH-dst.w;
        SDL_BlitSurface(vibeImg,&src,screen,&dst);
    }

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
    //flip the screen
	SDL_UpdateTexture(screenTexture, 0, screen->pixels, screen->pitch);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, screenTexture, 0, 0);
	SDL_RenderPresent(renderer);

    bodyRender=false;
}
