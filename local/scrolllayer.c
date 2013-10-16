#include "globals.h"

GRect scroll_layer_get_new_content_rect (ScrollLayer* scroll_layer,GPoint offset) {
    GRect to={{0,0},scroll_layer_get_content_size(scroll_layer)};
    to.size.h-=scroll_layer->layer.frame.size.h;
    offset.y*=-1;
    gpoint_move_into_rect(&offset,&to);
    offset.y*=-1;
    to.size.h+=scroll_layer->layer.frame.size.h;
    to.origin=offset;
    return to;
}

void scroll_layer_shadow_layer_update_func (Layer* me,GContext* ctx) {
    ScrollLayer* scroll_layer=(ScrollLayer*)me->parent;
    SDL_Surface* scroll_shadow=getSimulatorImage(SIM_IMG_SCROLL_SHADOW);
    GPoint topOffset=getTopOffset();
    SDL_Surface* screen=getTopScreen ();
    GPoint offset=scroll_layer_get_content_offset (scroll_layer);
    GSize size=scroll_layer_get_content_size (scroll_layer);
    GSize frameSize=me->frame.size;
    SDL_Rect src,dst;
    if (offset.y<0) {
        src=(SDL_Rect){0,15,144,15};
        dst=(SDL_Rect){topOffset.x,topOffset.y,144,15};
        SDL_gfxBlitRGBA(scroll_shadow,&src,screen,&dst);
    }
    if (offset.y-scroll_layer->layer.frame.size.h!=-size.h) {
        src=(SDL_Rect){0,0,144,15};
        dst=(SDL_Rect){topOffset.x,topOffset.y+frameSize.h-15,144,15};
        SDL_gfxBlitRGBA(scroll_shadow,&src,screen,&dst);
    }
}

void scroll_layer_click_config_provider (ClickConfig** click_configs,void* context) {
    ScrollLayer* scroll_layer=(ScrollLayer*)context;
    click_configs[BUTTON_ID_UP]->context=context;
    click_configs[BUTTON_ID_UP]->click.handler=(ClickHandler)&scroll_layer_scroll_up_click_handler;
    click_configs[BUTTON_ID_UP]->click.repeat_interval_ms=SCROLL_LAYER_CLICK_DELAY;
    click_configs[BUTTON_ID_DOWN]->context=context;
    click_configs[BUTTON_ID_DOWN]->click.handler=(ClickHandler)&scroll_layer_scroll_down_click_handler;
    click_configs[BUTTON_ID_DOWN]->click.repeat_interval_ms=SCROLL_LAYER_CLICK_DELAY;
    click_configs[BUTTON_ID_SELECT]->context=scroll_layer->context;
    if (scroll_layer->callbacks.click_config_provider!=0)
        scroll_layer->callbacks.click_config_provider(click_configs,context);
}

void scroll_layer_init(ScrollLayer *scroll_layer, GRect frame) {
    layer_init (&scroll_layer->layer,frame);
    layer_init (&scroll_layer->content_sublayer,(GRect){{0,0},frame.size});
    layer_init (&scroll_layer->shadow_sublayer,(GRect){{0,0},frame.size});
    layer_set_update_proc(&scroll_layer->shadow_sublayer,&scroll_layer_shadow_layer_update_func);
    layer_add_child (&scroll_layer->layer,&scroll_layer->content_sublayer);
    layer_add_child (&scroll_layer->layer,&scroll_layer->shadow_sublayer);
    scroll_layer->callbacks.click_config_provider=0;
    scroll_layer->callbacks.content_offset_changed_handler=0;
    scroll_layer->context=0;
    //no initalization for the animation
}

void scroll_layer_add_child(ScrollLayer *scroll_layer, Layer *child) {
    layer_add_child (&scroll_layer->content_sublayer,child);
}

void scroll_layer_set_click_config_onto_window(ScrollLayer *scroll_layer, struct Window *window) {
    window_set_click_config_provider_with_context(window,&scroll_layer_click_config_provider,scroll_layer);
}

void scroll_layer_set_callbacks(ScrollLayer *scroll_layer, ScrollLayerCallbacks callbacks) {
    scroll_layer->callbacks=callbacks;
}

void scroll_layer_set_context(ScrollLayer *scroll_layer, void *context) {
    scroll_layer->context=context;
}

void scroll_layer_set_content_offset(ScrollLayer *scroll_layer, GPoint offset, bool animated) {
    if (gpoint_equal(&scroll_layer->content_sublayer.frame.origin,&offset))
        return;
    offset.x=0;
    GPoint oldOffset=scroll_layer_get_content_offset(scroll_layer);
    GRect to=scroll_layer_get_new_content_rect(scroll_layer,offset);
    if (animated) {
        property_animation_init_layer_frame(&scroll_layer->animation,&scroll_layer->content_sublayer,0,&to);
        double duration=to.origin.y-oldOffset.y;
        if (duration<25||duration>-25)
            duration=abs(duration)*SCROLL_LAYER_SCROLL_SPEED;
        else
            duration=1;
        animation_set_duration((Animation*)&scroll_layer->animation,duration);
        animation_schedule((Animation*)&scroll_layer->animation);
    }
    else {
        scroll_layer->content_sublayer.frame.origin=offset;
        layer_mark_dirty((Layer*)scroll_layer);
    }
    if (scroll_layer->callbacks.content_offset_changed_handler!=0)
        scroll_layer->callbacks.content_offset_changed_handler(scroll_layer,scroll_layer->context);
}

GPoint scroll_layer_get_content_offset(ScrollLayer *scroll_layer) {
    return scroll_layer->content_sublayer.frame.origin;
}

void scroll_layer_set_content_size(ScrollLayer *scroll_layer, GSize size) {
    layer_set_frame((Layer*)&scroll_layer->content_sublayer,(GRect){scroll_layer->content_sublayer.frame.origin,size});
}

GSize scroll_layer_get_content_size(ScrollLayer *scroll_layer) {
    return scroll_layer->content_sublayer.frame.size;
}

void scroll_layer_set_frame(ScrollLayer *scroll_layer, GRect rect) {
    layer_set_frame((Layer*)scroll_layer,rect);
    layer_set_frame(&scroll_layer->shadow_sublayer,(GRect){{0,0},rect.size});
}

void scroll_layer_scroll_up_click_handler(ClickRecognizerRef recognizer, ScrollLayer *scroll_layer) {
    GPoint offset=scroll_layer_get_content_offset(scroll_layer);
    offset.y+=SCROLL_LAYER_SCROLL_AMOUNT;
    scroll_layer_set_content_offset(scroll_layer,offset,true);
}

void scroll_layer_scroll_down_click_handler(ClickRecognizerRef recognizer, ScrollLayer *scroll_layer) {
    GPoint offset=scroll_layer_get_content_offset(scroll_layer);
    offset.y-=SCROLL_LAYER_SCROLL_AMOUNT;
    scroll_layer_set_content_offset(scroll_layer,offset,true);
}
