#include "globals.h"

#define SCROLL_LAYER_GET Layer* scroll_layer=(Layer*)l
#define SCROLL_GET SCROLL_LAYER_GET;ScrollLayerData* scroll=(ScrollLayerData*)layer_get_data(scroll_layer)

Layer* scroll_layer_get_layer (const ScrollLayer* l) {
    return l;
}

GRect scroll_layer_get_new_content_rect (ScrollLayer* l,GPoint offset) {
    SCROLL_LAYER_GET;
    GRect to={{0,0},scroll_layer_get_content_size(l)};
    to.size.h-=scroll_layer->frame.size.h;
    offset.y*=-1;
    gpoint_move_into_rect(&offset,&to);
    offset.y*=-1;
    to.size.h+=scroll_layer->frame.size.h;
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
    if (offset.y-((Layer*)scroll_layer)->frame.size.h!=-size.h) {
        src=(SDL_Rect){0,0,144,15};
        dst=(SDL_Rect){topOffset.x,topOffset.y+frameSize.h-15,144,15};
        SDL_gfxBlitRGBA(scroll_shadow,&src,screen,&dst);
    }
}

void scroll_layer_click_config_provider (void* context) {
    window_single_repeating_click_subscribe(BUTTON_ID_UP,SCROLL_LAYER_CLICK_DELAY,scroll_layer_scroll_up_click_handler);
    window_set_click_context(BUTTON_ID_UP,context);
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN,SCROLL_LAYER_CLICK_DELAY,scroll_layer_scroll_down_click_handler);
    window_set_click_context(BUTTON_ID_DOWN,context);
    ScrollLayerData* scroll=(ScrollLayerData*)layer_get_data((Layer*)context);
    window_set_click_context(BUTTON_ID_SELECT,(scroll->context==0?context:scroll->context));
    if (scroll->callbacks.click_config_provider!=0)
        scroll->callbacks.click_config_provider(context);
}

ScrollLayer* scroll_layer_create(GRect frame) {
    Layer* layer=layer_create_with_data(frame,sizeof(ScrollLayerData));
    if (!layer)
        return 0;
    ScrollLayerData* scroll=(ScrollLayerData*)layer_get_data(layer);
    scroll->content_sublayer=layer_create ((GRect){{0,0},frame.size});
    if (!scroll->content_sublayer) {
        free(layer);
        return 0;
    }
    scroll->shadow_sublayer=layer_create ((GRect){{0,0},frame.size});
    if (!scroll->shadow_sublayer) {
        free(scroll->content_sublayer);
        free(layer);
        return 0;
    }
    scroll->animation=0;
    layer_set_update_proc(scroll->shadow_sublayer,scroll_layer_shadow_layer_update_func);
    layer_add_child (layer,scroll->content_sublayer);
    layer_add_child (layer,scroll->shadow_sublayer);
    scroll->callbacks.click_config_provider=0;
    scroll->callbacks.content_offset_changed_handler=0;
    scroll->context=0;
    return (ScrollLayer*)layer;
}

void scroll_layer_destroy (ScrollLayer* l) {
    if (l!=0) {
        SCROLL_GET;
        if (scroll->animation)
            property_animation_destroy(scroll->animation);
        layer_destroy(scroll->content_sublayer);
        layer_destroy(scroll->shadow_sublayer);
        layer_destroy(scroll_layer);
    }
}

void scroll_layer_add_child(ScrollLayer *l, Layer *child) {
    SCROLL_GET;
    layer_add_child (scroll->content_sublayer,child);
}

void scroll_layer_set_click_config_onto_window(ScrollLayer *l, struct Window *window) {
    window_set_click_config_provider_with_context(window,scroll_layer_click_config_provider,l);
}

void scroll_layer_set_callbacks(ScrollLayer *l, ScrollLayerCallbacks callbacks) {
    SCROLL_GET;
    scroll->callbacks=callbacks;
}

void scroll_layer_set_context(ScrollLayer *l, void *context) {
    SCROLL_GET;
    scroll->context=context;
}

void scroll_layer_set_content_offset(ScrollLayer *l, GPoint offset, bool animated) {
    SCROLL_GET;
    if (gpoint_equal(&scroll->content_sublayer->frame.origin,&offset))
        return;
    offset.x=0;
    GPoint oldOffset=scroll_layer_get_content_offset(l);
    GRect to=scroll_layer_get_new_content_rect(l,offset);
    if (animated) {
        if (scroll->animation)
            property_animation_destroy(scroll->animation);
        scroll->animation=property_animation_create_layer_frame(scroll->content_sublayer,0,&to);
        double duration=to.origin.y-oldOffset.y;
        if (duration<25||duration>-25)
            duration=abs(duration)*SCROLL_LAYER_SCROLL_SPEED;
        else
            duration=1;
        animation_set_duration((Animation*)scroll->animation,duration);
        animation_schedule((Animation*)scroll->animation);
    }
    else {
        scroll->content_sublayer->frame.origin=offset;
        layer_mark_dirty(scroll_layer);
    }
    if (scroll->callbacks.content_offset_changed_handler!=0)
        scroll->callbacks.content_offset_changed_handler(l,scroll->context);
}

GPoint scroll_layer_get_content_offset(ScrollLayer *l) {
    SCROLL_GET;
    return scroll->content_sublayer->frame.origin;
}

void scroll_layer_set_content_size(ScrollLayer *l, GSize size) {
    SCROLL_GET;
    layer_set_frame(scroll->content_sublayer,(GRect){scroll->content_sublayer->frame.origin,size});
}

GSize scroll_layer_get_content_size(const ScrollLayer *l) {
    SCROLL_GET;
    return scroll->content_sublayer->frame.size;
}

void scroll_layer_set_frame(ScrollLayer *l, GRect rect) {
    SCROLL_GET;
    layer_set_frame(scroll_layer,rect);
    layer_set_frame(scroll->shadow_sublayer,(GRect){{0,0},rect.size});
}

void scroll_layer_scroll_up_click_handler(ClickRecognizerRef recognizer, void *l) {
    ScrollLayer* scroll_layer=(ScrollLayer*)l;
    GPoint offset=scroll_layer_get_content_offset(scroll_layer);
    offset.y+=SCROLL_LAYER_SCROLL_AMOUNT;
    scroll_layer_set_content_offset(scroll_layer,offset,true);
}

void scroll_layer_scroll_down_click_handler(ClickRecognizerRef recognizer, void *l) {
    ScrollLayer* scroll_layer=(ScrollLayer*)l;
    GPoint offset=scroll_layer_get_content_offset(scroll_layer);
    offset.y-=SCROLL_LAYER_SCROLL_AMOUNT;
    scroll_layer_set_content_offset(scroll_layer,offset,true);
}

void scroll_layer_set_shadow_hidden (ScrollLayer* l,bool hidden) {
    SCROLL_GET;
    layer_set_hidden(scroll->shadow_sublayer,hidden);
}

bool scroll_layer_get_shadow_hidden (const ScrollLayer* l) {
    SCROLL_GET;
    return layer_get_hidden(scroll->shadow_sublayer);
}
