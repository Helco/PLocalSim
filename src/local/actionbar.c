#include "globals.h"

#define ACTION_BAR_SPACING 3
#define ACTION_BAR_LAYER_GET Layer* action_bar_layer=(Layer*)l
#define ACTION_BAR_GET ACTION_BAR_LAYER_GET;ActionBarLayerData* action_bar=((ActionBarLayerData*)layer_get_data(action_bar_layer))

void action_bar_click_config_provider (void* context);
void action_bar_click_handler_up (ClickRecognizerRef recognizer,void* context);
void action_bar_click_handler_down (ClickRecognizerRef recognizer,void* context);
void action_bar_update_handler (Layer* me,GContext* ctx);

ActionBarLayer* action_bar_layer_create(void) {
    Layer* layer=layer_create_with_data(GRect(0,0,PBL_SCREEN_WIDTH,PBL_SCREEN_HEIGHT),sizeof(ActionBarLayerData));
    ActionBarLayerData* action_bar=(ActionBarLayerData*)layer_get_data(layer);
    layer_set_update_proc (layer,action_bar_update_handler);
    action_bar->icons[0]=0;
    action_bar->icons[1]=0;
    action_bar->icons[2]=0;
    action_bar->context=0;
    action_bar->click_config_provider=0;
    action_bar->is_highlighted=0;
    action_bar->background_color=GColorBlack;
    return (ActionBarLayer*)layer;
}

void action_bar_layer_destroy (ActionBarLayer* layer) {
    if (layer)
        layer_destroy((Layer*)layer);
}

void action_bar_layer_set_context(ActionBarLayer *l, void *context) {
    ACTION_BAR_GET;
    action_bar->context=context;
}

void action_bar_layer_set_click_config_provider(ActionBarLayer *l, ClickConfigProvider click_config_provider) {
    ACTION_BAR_GET;
    action_bar->click_config_provider=click_config_provider;
    if (click_config_provider!=0 && //there IS a config provider
        action_bar_layer->window!=0 && //the action bar is added to a window
        window_stack_get_top_window()==action_bar_layer->window) //this window is currently visible
        click_config_provider (action_bar->context);
}

void action_bar_layer_set_icon(ActionBarLayer *l, ButtonId button_id, const GBitmap *icon) {
    ACTION_BAR_GET;
    bool hasSet=true;
    switch (button_id) {
        case(BUTTON_ID_UP):{action_bar->icons[0]=icon;}break;
        case(BUTTON_ID_SELECT):{action_bar->icons[1]=icon;}break;
        case(BUTTON_ID_DOWN):{action_bar->icons[2]=icon;}break;
        default:{hasSet=false;}break;
    }
    if (hasSet && //button_id is valid
        action_bar->click_config_provider!=0 && //there is a click config provider
        action_bar_layer->window!=0 && //the action bar is added to a window
        window_stack_get_top_window()==action_bar_layer->window) //this window is currently visible
        action_bar->click_config_provider (action_bar->context);
}

void action_bar_layer_clear_icon(ActionBarLayer *l, ButtonId button_id) {
    action_bar_layer_set_icon (l,button_id,0);
}

void action_bar_layer_set_background_color(ActionBarLayer *l, GColor background_color) {
    ACTION_BAR_GET;
    action_bar->background_color=background_color;
    layer_mark_dirty (action_bar_layer);
}

void action_bar_layer_add_to_window(ActionBarLayer *l, struct Window *window) {
    ACTION_BAR_LAYER_GET;
    GRect p=window->layer->frame;
    layer_insert_below_sibling(action_bar_layer,window->layer);
    layer_set_frame (action_bar_layer,GRect(p.size.w-ACTION_BAR_WIDTH,
                                              ACTION_BAR_SPACING,
                                              ACTION_BAR_WIDTH,
                                              p.size.h-2*ACTION_BAR_SPACING));
    window_set_click_config_provider_with_context  (window,action_bar_click_config_provider,l);
    //action_bar_layer->window=window; //is already set in layer_insert_below_sibling
}

void action_bar_layer_remove_from_window(ActionBarLayer *l) {
    ACTION_BAR_GET;
    if (action_bar_layer->window!=0) {
        Layer* cursor=action_bar_layer->window->layer;
        while (cursor->next_sibling!=0) {
            if (cursor->next_sibling==((Layer*)action_bar)) {
                cursor->next_sibling=action_bar_layer->next_sibling;
                break;
            }
            cursor=cursor->next_sibling;
        }
        if (cursor->next_sibling==0)
            return;
        window_set_click_config_provider_with_context(action_bar_layer->window,0,0);
        if (window_stack_get_top_window()==action_bar_layer->window) {
            ClickConfig** clickConfig=getClickConfig ();
            int i;
            for (i=BUTTON_ID_UP;i<=BUTTON_ID_DOWN;i++) {
                if (clickConfig[i]->raw.up_handler==action_bar_click_handler_up)
                    clickConfig[i]->raw.up_handler=0;
                if (clickConfig[i]->raw.down_handler==action_bar_click_handler_down)
                    clickConfig[i]->raw.down_handler=0;
            }
        }
        action_bar_layer->window=0;
    }
}

void action_bar_click_config_provider (void* l) {
    ACTION_BAR_GET;
    for (ButtonId id=BUTTON_ID_UP;id<=BUTTON_ID_DOWN;id++)
        window_raw_click_subscribe(BUTTON_ID_UP,action_bar_click_handler_down,action_bar_click_handler_up,l);
    if (action_bar->click_config_provider)
        action_bar->click_config_provider(action_bar->context);
}

void action_bar_click_handler_up (ClickRecognizerRef rec,void* l) {
    ACTION_BAR_GET;
    uint16_t mask;
    switch (click_recognizer_get_button_id (rec))
    {
        case(BUTTON_ID_UP):{mask=0;}break;
        case(BUTTON_ID_SELECT):{mask=1;}break;
        case(BUTTON_ID_DOWN):{mask=2;}break;
        default:{return;}break;
    }
    mask=(1<<mask)^((unsigned)~0);
    action_bar->is_highlighted&=mask;
    layer_mark_dirty(action_bar_layer);
}

void action_bar_click_handler_down (ClickRecognizerRef rec,void* l) {
    ACTION_BAR_GET;
    uint16_t mask;
    switch (click_recognizer_get_button_id (rec))
    {
        case(BUTTON_ID_UP):{mask=0;}break;
        case(BUTTON_ID_SELECT):{mask=1;}break;
        case(BUTTON_ID_DOWN):{mask=2;}break;
        default:{return;}break;
    }
    action_bar->is_highlighted|=1<<mask;
    layer_mark_dirty(action_bar_layer);
}

void action_bar_update_handler (Layer* l,GContext* ctx) {
    ACTION_BAR_GET;
    ActionBarLayerData* bar=action_bar;
    GCompOp compOp;
    //TODO: implement corner_mask in graphics_fill_rect
    graphics_context_set_fill_color (ctx,bar->background_color);
    graphics_fill_rect (ctx,GRect(0,0,l->frame.size.w+3,l->frame.size.h),3,0);
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
        graphics_draw_bitmap_in_rect (ctx,bar->icons[1],GRect(11-bar->icons[1]->bounds.size.w/2,l->frame.size.h/2-bar->icons[1]->bounds.size.h/2,bar->icons[1]->bounds.size.w,bar->icons[1]->bounds.size.h));
    }
    if (bar->icons[2]!=0) {
        if (bar->is_highlighted&(1<<2))
            compOp=GCompOpAssignInverted;
        else
            compOp=GCompOpAssign;
        graphics_context_set_compositing_mode(ctx,compOp);
        graphics_draw_bitmap_in_rect (ctx,bar->icons[2],GRect(11-bar->icons[2]->bounds.size.w/2,l->frame.size.h-20-11-bar->icons[2]->bounds.size.h,bar->icons[2]->bounds.size.w,bar->icons[2]->bounds.size.h));
    }
}
