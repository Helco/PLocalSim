#include "globals.h"

#define LONG_CLICK_STD_DELAY 750 //no sdk value for this... because of onthebutton there is one

typedef struct ClickRecognizer
{
    ButtonId id;
    uint32_t lastDown;
    bool isDown;
    uint16_t clickCount; //reseted by multi_click.timeout
    bool longClick;
} ClickRecognizer;

static PebbleAppHandlers* appHandlers=0;
static ClickConfig clickConfig [NUM_BUTTONS];
static ClickConfig* clickConfigPtr [NUM_BUTTONS];
static ClickRecognizer clickRecognizer [NUM_BUTTONS];

ClickConfig** getClickConfig ()
{
    return clickConfigPtr;
}

void resetClickConfig ()
{
    int i;
    memset(clickConfig,0,sizeof(ClickConfig)*NUM_BUTTONS);
    for (i=0;i<NUM_BUTTONS;i++) {
        clickConfig[i].long_click.delay_ms=LONG_CLICK_STD_DELAY;

        clickRecognizer[i].lastDown=0;
        clickRecognizer[i].isDown=false;
        clickRecognizer[i].clickCount=0;
        clickRecognizer[i].longClick=false;
    }
}

void initButtons ()
{
    int i;
    for (i=0;i<NUM_BUTTONS;i++) {
        clickRecognizer[i].id=i;
        clickConfigPtr[i]=clickConfig+i;
    }
    resetClickConfig();
}

void onButtonDown (ButtonId id)
{
    ClickRecognizer* rec;
    ClickConfig* conf;
    bool handled=false;
    if (appHandlers==0)
        appHandlers=getAppHandlers();
    if (appHandlers->input_handlers.buttons.down!=0) {
        PebbleButtonEvent event;
        event.button_id=id;
        appHandlers->input_handlers.buttons.down(0,&event);
    }
    rec=clickRecognizer+id;
    conf=clickConfig+id;
    rec->clickCount++;
    rec->lastDown=SDL_GetTicks();
    rec->isDown=true;
    if (conf->raw.down_handler!=0) {
        conf->raw.down_handler(rec,conf->raw.context);
        handled=true;
    }
    if (conf->multi_click.handler!=0&&
        ((rec->clickCount>=conf->multi_click.min&&(conf->multi_click.max==0||rec->clickCount<=conf->multi_click.max)&&!conf->multi_click.last_click_only)||
         (rec->clickCount==conf->multi_click.max&&conf->multi_click.max!=0&&conf->multi_click.last_click_only))) {
         conf->multi_click.handler(rec,conf->context);
         handled=true;
    }
    if (!handled&&id==BUTTON_ID_BACK&&getWindowStackSize()>1)
        window_stack_pop (true);
}

void onButtonUp (ButtonId id)
{
    ClickRecognizer* rec;
    ClickConfig* conf;
    if (appHandlers==0)
        appHandlers=getAppHandlers();
    if (appHandlers->input_handlers.buttons.up!=0) {
        PebbleButtonEvent event;
        event.button_id=id;
        appHandlers->input_handlers.buttons.up(0,&event);
    }
    rec=clickRecognizer+id;
    conf=clickConfig+id;
    rec->isDown=false;
    if (conf->click.handler!=0&&(conf->long_click.handler==0||SDL_GetTicks()-rec->lastDown<conf->long_click.delay_ms))
        conf->click.handler(rec,conf->context);
    if (conf->raw.up_handler!=0)
        conf->raw.up_handler(rec,conf->raw.context);
    if (rec->longClick&&conf->long_click.delay_ms>0&&conf->long_click.release_handler!=0)
        conf->long_click.release_handler(rec,conf->context);
    rec->longClick=false;
}

void updateButtons ()
{
    ClickRecognizer* rec;
    ClickConfig* conf;
    int i;
    for (i=0;i<NUM_BUTTONS;i++) {
        rec=clickRecognizer+i;
        conf=clickConfig+i;
        if (conf->multi_click.timeout>0&&rec->lastDown>0&&SDL_GetTicks()-rec->lastDown>conf->multi_click.timeout)
            rec->clickCount=0;
        if (rec->isDown) {
            if (conf->click.handler!=0&&conf->click.repeat_interval_ms>0&&SDL_GetTicks()-rec->lastDown>=conf->click.repeat_interval_ms) {
                conf->click.handler(rec,conf->context);
                rec->lastDown=SDL_GetTicks();
                rec->clickCount++;
            }
            if (conf->long_click.handler!=0&&conf->long_click.delay_ms>0&&!rec->longClick&&SDL_GetTicks()-rec->lastDown>=conf->long_click.delay_ms) {
                conf->long_click.handler(rec,conf->context);
                rec->longClick=true;
            }
        }
    }
}

void buttonsUpdateWindow (Window* w)
{
    ClickConfig* configs[NUM_BUTTONS];
    int i;
    resetClickConfig ();
    if (w!=0&&w->click_config_provider!=0) {
        for (i=0;i<NUM_BUTTONS;i++)
            configs[i]=clickConfig+i;
        w->click_config_provider(configs,w->click_config_context);
    }
}

uint8_t click_number_of_clicks_counted (ClickRecognizerRef recognizer) {
    return ((ClickRecognizer*)recognizer)->clickCount;
}

ButtonId click_recognizer_get_button_id (ClickRecognizerRef recognizer) {
    return ((ClickRecognizer*)recognizer)->id;
}
