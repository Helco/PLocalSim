#include "globals.h"

#define LONG_CLICK_STD_DELAY 750 //no sdk value for this...

typedef struct ClickRecognizer
{
    ButtonId id;
    uint32_t lastDown;
    bool isDown;
    uint16_t clickCount; //reseted by multi_click.timeout
    bool longClick;
} ClickRecognizer;

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
    rec=clickRecognizer+id;
    conf=&(clickConfig[id]);
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
    rec=clickRecognizer+id;
    conf=clickConfig+id;
    rec->isDown=false;
    if (conf->click.handler!=0&&!rec->longClick)
        conf->click.handler(rec,conf->context);
    if (conf->raw.up_handler!=0)
        conf->raw.up_handler(rec,conf->raw.context);
    if (rec->longClick&&conf->long_click.delay_ms>0&&conf->long_click.release_handler!=0)
        conf->long_click.release_handler(rec,conf->context);
    rec->longClick=false;
}

void service_buttons ()
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
            if (conf->long_click.delay_ms>0&&!rec->longClick&&SDL_GetTicks()-rec->lastDown>=conf->long_click.delay_ms) {
                if (conf->long_click.handler!=0)
                    conf->long_click.handler(rec,conf->context);
                rec->longClick=true;
            }
        }
    }
}

void buttonsUpdateWindow (Window* w)
{
    resetClickConfig ();
    if (w!=0&&w->click_config_provider!=0)
        w->click_config_provider(w->click_config_context);
}

uint8_t click_number_of_clicks_counted (ClickRecognizerRef recognizer) {
    return ((ClickRecognizer*)recognizer)->clickCount;
}

ButtonId click_recognizer_get_button_id (ClickRecognizerRef recognizer) {
    return ((ClickRecognizer*)recognizer)->id;
}

void window_single_click_subscribe(ButtonId button_id,ClickHandler handler) {
    clickConfig[button_id].click.repeat_interval_ms=0;
    clickConfig[button_id].click.handler=handler;
}

void window_single_repeating_click_subscribe(ButtonId button_id,uint16_t repeat_interval_ms,ClickHandler handler) {
    clickConfig[button_id].click.repeat_interval_ms=repeat_interval_ms;
    clickConfig[button_id].click.handler=handler;
}

void window_multi_click_subscribe(ButtonId button_id,uint8_t min_clicks,uint8_t max_clicks,uint16_t timeout,bool last_click_only,ClickHandler handler) {
    clickConfig[button_id].multi_click.min=min_clicks;
    clickConfig[button_id].multi_click.max=max_clicks;
    clickConfig[button_id].multi_click.timeout=timeout;
    clickConfig[button_id].multi_click.last_click_only=last_click_only;
    clickConfig[button_id].multi_click.handler=handler;
}

void window_long_click_subscribe(ButtonId button_id,uint16_t delay_ms,ClickHandler down_handler,ClickHandler up_handler) {
    clickConfig[button_id].long_click.delay_ms=(delay_ms<=0?LONG_CLICK_STD_DELAY:delay_ms);
    clickConfig[button_id].long_click.handler=down_handler;
    clickConfig[button_id].long_click.release_handler=up_handler;
}

void window_raw_click_subscribe(ButtonId button_id,ClickHandler down_handler,ClickHandler up_handler,void* context) {
    clickConfig[button_id].raw.down_handler=down_handler;
    clickConfig[button_id].raw.up_handler=up_handler;
    clickConfig[button_id].raw.context=context;
}

void window_set_click_context (ButtonId button_id,void* context) {
    clickConfig[button_id].context=context;
}
