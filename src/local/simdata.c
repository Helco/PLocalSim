#include "globals.h"

#define SIM_FILE_BUFFER 64
#define SIM_IMG_BASE_NAME "./images/%s"

typedef struct SimDataImage
{
    const char* fileName;
    SDL_Surface* surface;
} SimDataImage;

SimDataImage simImages [SIM_IMG_COUNT]={
    {"body.png",0},
    {"screenshadow.png",0},
    {"backlight.png",0},
    {"statusbar.png",0},
    {"vibe.png",0},
    {"scrollshadow.png",0}
};

bool loadSimulatorImages() {
    char fileBuffer[SIM_FILE_BUFFER]={0};
    for (int i=0;i<SIM_IMG_COUNT;i++) {
        sprintf (fileBuffer,SIM_IMG_BASE_NAME,simImages[i].fileName);
        simImages[i].surface=IMG_Load (fileBuffer);
        if (!simImages[i].surface) {
            printf ("[ERROR]: Couldn't load simulator image:%s\n",simImages[i].fileName);
            return false;
        }
    }
    return true;
}

void freeSimulatorImages() {
    for (int i=0;i<SIM_IMG_COUNT;i++) {
        SDL_FreeSurface(simImages[i].surface);
        simImages[i].surface=0;
    }
}

SDL_Surface* getSimulatorImage(int imageID) {
    return simImages[imageID].surface;
}
