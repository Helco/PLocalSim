#include "globals.h"

#define WATCH_COLOR_COUNT (WATCH_INFO_COLOR_PINK + 1)

static const SDL_Rect buttonRects [NUM_BUTTONS * 2]= {
    //Original pebble
    {16, 127, 6, 80},
    {252, 91, 6, 80},
    {252, 187, 6, 50},
    {252, 253, 6, 80},

    //Pebble Steel
    {12, 131, 13, 63},
    {247, 133, 13, 63},
    {250, 196, 11, 48},
    {247, 244, 12, 62}
};

static WatchInfoColor currentWatchColor = WATCH_INFO_COLOR_RED;

bool getIsBodySteel () {
    return watch_info_get_model() == WATCH_INFO_MODEL_PEBBLE_STEEL;
}

void nextBody () {
    currentWatchColor = (currentWatchColor + 1) % WATCH_COLOR_COUNT;
}

void drawBody (SDL_Surface* screen, uint8_t buttonState) {
    SDL_Surface* bodyImg = getSimulatorImage(SIM_IMG_BODY);
    SDL_Rect dst, src;
    int i;

    for (i=0; i<NUM_BUTTONS; i++) {
        src = dst = buttonRects[getIsBodySteel()*NUM_BUTTONS + i];
        src.y += 3 * 440;
        dst.x += BODY_OFFSET_X;
        dst.y += BODY_OFFSET_Y;

        if (buttonState & (1 << i))
            dst.x += (i==BUTTON_ID_BACK ? 3 : -3);

        if (currentWatchColor == WATCH_INFO_COLOR_STAINLESS_STEEL)
            src.x += 1 * 275;
        else if (currentWatchColor == WATCH_INFO_COLOR_MATTE_BLACK)
            src.x += 2 * 275;

        SDL_BlitSurface(bodyImg, &src, screen, &dst);
    }

    dst = (SDL_Rect) {BODY_OFFSET_X, BODY_OFFSET_Y, BODY_WIDTH, BODY_HEIGHT};
    i = (int)currentWatchColor;
    src = (SDL_Rect) {(i%4) * BODY_WIDTH, (i/4) * BODY_HEIGHT,
                                BODY_WIDTH, BODY_HEIGHT};
    SDL_BlitSurface(bodyImg, &src, screen, &dst);
}

WatchInfoColor watch_info_get_color () {
    return currentWatchColor;
}

WatchInfoModel watch_info_get_model () {
    if (currentWatchColor == WATCH_INFO_COLOR_UNKNOWN)
        return WATCH_INFO_MODEL_UNKNOWN;
    else if (currentWatchColor == WATCH_INFO_COLOR_STAINLESS_STEEL ||
             currentWatchColor == WATCH_INFO_COLOR_MATTE_BLACK)
        return WATCH_INFO_MODEL_PEBBLE_STEEL;
    else
        return WATCH_INFO_MODEL_PEBBLE_ORIGINAL;
}

WatchInfoVersion watch_info_get_firmware_version () {
    const WatchInfoVersion version = {2, 8, 1};
    return version;
}
