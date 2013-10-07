#include "globals.h"

#define ACTION_BAR_SPACING 3

void action_bar_click_config_provider (ClickConfig** clickConfig,void* context);
void action_bar_click_handler_up (ClickRecognizerRef recognizer,void* context);
void action_bar_click_handler_down (ClickRecognizerRef recognizer,void* context);
void action_bar_update_handler (Layer* me,GContext* ctx);

void action_bar_layer_init(ActionBarLayer *action_bar) {
    layer_init (&action_bar->layer,GRect(0,0,0,0));
    layer_set_update_proc ((Layer*)action_bar,action_bar_update_handler);
    action_bar->icons[0]=0;
    action_bar->icons[1]=0;
    action_bar->icons[2]=0;
    action_bar->window=0;
    action_bar->context=0;
    action_bar->click_config_provider=0;
    action_bar->is_highlighted=0;
    action_bar->background_color=GColorBlack;
}

void action_bar_layer_set_context(ActionBarLayer *action_bar, void *context) {
    action_bar->context=context;
}

void action_bar_layer_set_click_config_provider(ActionBarLayer *action_bar, ClickConfigProvider click_config_provider) {
    action_bar->click_config_provider=click_config_provider;
    if (click_config_provider!=0&&action_bar->window!=0&&window_stack_get_top_window()==action_bar->window)
        click_config_provider (getClickConfig (),action_bar->context);
}

void action_bar_layer_set_icon(ActionBarLayer *action_bar, ButtonId button_id, const GBitmap *icon) {
    bool hasSet=true;
    switch (button_id) {
        case(BUTTON_ID_UP):{action_bar->icons[0]=icon;}break;
        case(BUTTON_ID_SELECT):{action_bar->icons[1]=icon;}break;
        case(BUTTON_ID_DOWN):{action_bar->icons[2]=icon;}break;
        default:{hasSet=false;}break;
    }
    if (hasSet&&action_bar->click_config_provider!=0&&action_bar->window!=0&&window_stack_get_top_window()==action_bar->window)
        action_bar->click_config_provider (getClickConfig (),action_bar->context);
}

void action_bar_layer_clear_icon(ActionBarLayer *action_bar, ButtonId button_id) {
    action_bar_layer_set_icon (action_bar,button_id,0);
}

void action_bar_layer_add_to_window(ActionBarLayer *action_bar, struct Window *window) {
    GRect p=window->layer.frame;
    layer_insert_below_sibling((Layer*)action_bar,&window->layer);
    layer_set_frame ((Layer*)action_bar,GRect(p.size.w-ACTION_BAR_WIDTH,ACTION_BAR_SPACING,ACTION_BAR_WIDTH,p.size.h-2*ACTION_BAR_SPACING));
    window_set_click_config_provider_with_context  (window,action_bar_click_config_provider,action_bar);
    action_bar->window=window;
}

void action_bar_layer_remove_from_window(ActionBarLayer *action_bar) {
    if (action_bar->window!=0) {
        Layer* cursor=&action_bar->window->layer;
        while (cursor->next_sibling!=0) {
            if (cursor->next_sibling==((Layer*)action_bar)) {
                cursor->next_sibling=action_bar->layer.next_sibling;
                break;
            }
            cursor=cursor->next_sibling;
        }
        action_bar->window->click_config_provider=0;
        action_bar->window->click_config_context=0;
        if (window_stack_get_top_window()==action_bar->window) {
            ClickConfig** clickConfig=getClickConfig ();
            int i;
            for (i=BUTTON_ID_UP;i<=BUTTON_ID_DOWN;i++) {
                if (clickConfig[i]->raw.up_handler==action_bar_click_handler_up)
                    clickConfig[i]->raw.up_handler=0;
                if (clickConfig[i]->raw.down_handler==action_bar_click_handler_down)
                    clickConfig[i]->raw.down_handler=0;
            }
        }
        action_bar->window=0;
    }
}

void action_bar_layer_set_background_color(ActionBarLayer *action_bar, GColor background_color) {
    action_bar->background_color=background_color;
    layer_mark_dirty ((Layer*)action_bar);
}

void action_bar_click_config_provider (ClickConfig** clickConfig,void* context) {
    ActionBarLayer* layer=(ActionBarLayer*)context;
    clickConfig[BUTTON_ID_UP]->raw.up_handler=action_bar_click_handler_up;
    clickConfig[BUTTON_ID_UP]->raw.down_handler=action_bar_click_handler_down;
    clickConfig[BUTTON_ID_UP]->raw.context=context;
    clickConfig[BUTTON_ID_SELECT]->raw.up_handler=action_bar_click_handler_up;
    clickConfig[BUTTON_ID_SELECT]->raw.down_handler=action_bar_click_handler_down;
    clickConfig[BUTTON_ID_SELECT]->raw.context=context;
    clickConfig[BUTTON_ID_DOWN]->raw.up_handler=action_bar_click_handler_up;
    clickConfig[BUTTON_ID_DOWN]->raw.down_handler=action_bar_click_handler_down;
    clickConfig[BUTTON_ID_DOWN]->raw.context=context;
    if (layer->click_config_provider)
        layer->click_config_provider(clickConfig,layer->context);
}

void action_bar_click_handler_up (ClickRecognizerRef rec,void* context) {
    ActionBarLayer* layer=(ActionBarLayer*)context;
    unsigned mask;
    switch (click_recognizer_get_button_id (rec))
    {
        case(BUTTON_ID_UP):{mask=0;}break;
        case(BUTTON_ID_SELECT):{mask=1;}break;
        case(BUTTON_ID_DOWN):{mask=2;}break;
        default:{return;}break;
    }
    mask=(1<<mask)^((unsigned)~0);
    layer->is_highlighted&=mask;
    layer_mark_dirty((Layer*)layer);
}

void action_bar_click_handler_down (ClickRecognizerRef rec,void* context) {
    ActionBarLayer* layer=(ActionBarLayer*)context;
    unsigned mask;
    switch (click_recognizer_get_button_id (rec))
    {
        case(BUTTON_ID_UP):{mask=0;}break;
        case(BUTTON_ID_SELECT):{mask=1;}break;
        case(BUTTON_ID_DOWN):{mask=2;}break;
        default:{return;}break;
    }
    layer->is_highlighted|=1<<mask;
    layer_mark_dirty((Layer*)layer);
}

void action_bar_update_handler (Layer* me,GContext* ctx) {
    ActionBarLayer* bar=(ActionBarLayer*)me;
    GCompOp compOp;
    //TODO: implement corner_mask in graphics_fill_rect
    graphics_context_set_fill_color (ctx,bar->background_color);
    graphics_fill_rect (ctx,GRect(0,0,me->frame.size.w+3,me->frame.size.h),3,0);
    if (bar->icons[0]!=0) {
        if (bar->is_highlighted&(1<<0))
            compOp=GCompOpAssignInverted;
        else
            compOp=GCompOpAssign;
        graphics_context_set_compositing_mode(ctx,compOp);
        graphics_draw_bitmap_in_rect (ctx,bar->icons[0],GRect(11-bar->icons[0]->bounds.size.w/2,20+11-bar->icons[0]->bounds.size.h,bar->icons[0]->bounds.size.w,bar->icons[0]->bounds.size.h));
    }
    if (bar->icons[1]!=0) {
        if (bar->is_highlighted&(1<<1))
            compOp=GCompOpAssignInverted;
        else
            compOp=GCompOpAssign;
        graphics_context_set_compositing_mode(ctx,compOp);
        graphics_draw_bitmap_in_rect (ctx,bar->icons[1],GRect(11-bar->icons[1]->bounds.size.w/2,me->frame.size.h/2-bar->icons[1]->bounds.size.h/2,bar->icons[1]->bounds.size.w,bar->icons[1]->bounds.size.h));
    }
    if (bar->icons[2]!=0) {
        if (bar->is_highlighted&(1<<2))
            compOp=GCompOpAssignInverted;
        else
            compOp=GCompOpAssign;
        graphics_context_set_compositing_mode(ctx,compOp);
        graphics_draw_bitmap_in_rect (ctx,bar->icons[2],GRect(11-bar->icons[2]->bounds.size.w/2,me->frame.size.h-20-11-bar->icons[2]->bounds.size.h,bar->icons[2]->bounds.size.w,bar->icons[2]->bounds.size.h));
    }
}
