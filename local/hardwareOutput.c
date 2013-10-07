#include "globals.h"

typedef struct VibeElement
{
    bool vibe;
    uint32_t len;
} VibeElement;
static VibeElement vibes[MAX_VIBE_ELEMENTS];
static int vibeStart=0;
static int vibeSize=0;
static bool currentVibe=false;
static uint32_t vibesTick=0;

static int light_length=0; //-1=on 0=off >0=on for light_length ms
static int light_tick=0;

void initHardwareOutput ()
{
}

void updateHardwareOutput() {
    if (vibeSize>0) {
        if (vibesTick==0) {
            vibesTick=SDL_GetTicks();
            currentVibe=vibes[vibeStart].vibe;
        }
        if (SDL_GetTicks()-vibesTick>=vibes[vibeStart].len) {
            vibeStart++;
            if (vibeStart==MAX_VIBE_ELEMENTS)
                vibeStart=0;
            vibeSize--;
            if (vibeSize==0&&currentVibe)
                currentVibe=false;
            vibesTick=0;
        }
    }
    if (light_length>0&&SDL_GetTicks()-light_tick>=light_length)
        light_length=0;
}

bool getVibeState() {
    return currentVibe;
}

bool getLightState() {
    return (light_length!=0);
}

void vibes_cancel ()
{
    vibeStart=0;
    vibeSize=0;
}

void addVibe (uint32_t len,bool vibe)
{
    vibes[(vibeStart+vibeSize)%MAX_VIBE_ELEMENTS]=(VibeElement){vibe,len};
    vibeSize++;
}

void vibes_double_pulse ()
{
    addVibe(VIBE_SHORT_MS,true);
    addVibe(VIBE_SHORT_MS,false);
    addVibe(VIBE_SHORT_MS,true);
}

void vibes_short_pulse ()
{
    addVibe(VIBE_SHORT_MS,true);
}

void vibes_long_pulse ()
{
    addVibe(VIBE_LONG_MS,true);
}

void vibes_enqueue_custom_pattern (VibePattern pattern)
{
    int i;
    for (i=0;i<pattern.num_segments;i++)
        addVibe(pattern.durations[i],i%2);
}

void light_enable (bool toggle)
{
    light_length=(toggle?-1:0);
}

void light_enable_interaction ()
{
    light_tick=SDL_GetTicks();
    light_length=LIGHT_LENGTH;
}
