#include "globals.h"

void layer_mark_dirty(Layer *layer) {
    if (layer!=0&&window_stack_get_top_window()==layer->window)
        markDirty (true);
}

void layer_remove_from_parent(Layer *child) {
    if (child==0||child->parent==0)
        return;
    child->window=0;
    Layer *cursor = child->parent->first_child;
    if(cursor == child||cursor==0) { //why cursor==0? big_time has this behaviour :(
        child->parent->first_child = child->next_sibling;
    } else {
        while(cursor->next_sibling != child) {
            cursor = cursor->next_sibling;
        }
        cursor->next_sibling = child->next_sibling;
    }
    layer_mark_dirty (child->parent);
    child->parent = NULL;
    child->next_sibling=0;
}

void layer_add_child(Layer *parent, Layer *child) {
    layer_mark_dirty (parent);
    child->parent=parent;
    child->window=parent->window;
    if (parent==child)
        return; //big_time .... why does the original sdk allow this?!
    else if(parent->first_child == NULL) {
        parent->first_child = child;
    } else {
        parent = parent->first_child;
        while(parent->next_sibling != NULL) {
            if (parent->next_sibling==child) //big_time .... why does the original sdk allow this?!
                return;
            parent = parent->next_sibling;
        }
        parent->next_sibling = child;
    }
}

GRect layer_get_frame(Layer *layer) {
    return layer->frame;
}

void layer_set_frame(Layer *layer, GRect frame) {
    if (!grect_equal(&layer->frame,&frame)) {
        layer->frame = frame;
        //extend bounds
        if (layer->bounds.origin.x+layer->bounds.size.w<layer->frame.size.w)
            layer->bounds.size.w=layer->frame.size.w-layer->bounds.origin.x;
        if (layer->bounds.origin.y+layer->bounds.size.h<layer->frame.size.h)
            layer->bounds.size.h=layer->frame.size.h-layer->bounds.origin.y;
        layer_mark_dirty (layer);
    }
}

void layer_set_hidden(Layer *layer, bool hidden) {
    if (layer->hidden!=hidden) {
        layer->hidden = hidden;
        layer_mark_dirty(layer->parent);
    }
}

//#verify
void layer_init(Layer *layer, GRect frame) {
    layer->frame = frame;
    layer->bounds=GRect(0,0,frame.size.w,frame.size.h);
    layer->clips=true;
    layer->hidden=false;
    layer->parent=0;
    layer->first_child=0;
    layer->window=0;
    layer->update_proc=0;
}

void layer_set_bounds(Layer *layer, GRect bounds) {
    if (!grect_equal(&layer->bounds,&bounds)) {
        layer->bounds = bounds;
        layer_mark_dirty(layer);
    }
}

GRect layer_get_bounds(Layer *layer) {
    return layer->bounds;
}

void layer_set_update_proc(Layer *layer, LayerUpdateProc update_proc) {
    layer->update_proc = update_proc;
}

struct Window *layer_get_window(Layer *layer) {
    return layer->window;
}

void layer_remove_child_layers(Layer *parent) {
    Layer *cursor = parent->first_child;
    while(cursor != NULL) {
        cursor->parent = NULL;
        cursor = cursor->next_sibling;
    }
    parent->first_child = NULL;
    layer_mark_dirty (parent);
}


void layer_insert_below_sibling(Layer *layer_to_insert, Layer *below_sibling_layer) {
    layer_to_insert->next_sibling = below_sibling_layer->next_sibling;
    below_sibling_layer->next_sibling = layer_to_insert;
    layer_to_insert->parent = below_sibling_layer->parent;
    layer_to_insert->window = below_sibling_layer->window;
    layer_mark_dirty (below_sibling_layer);
}

void layer_insert_above_sibling(Layer *layer_to_insert, Layer *above_sibling_layer) {
    if(above_sibling_layer->parent->first_child == above_sibling_layer) {
        above_sibling_layer->parent->first_child = layer_to_insert;
    } else {
        Layer *cursor = above_sibling_layer->parent->first_child;
        while(cursor->next_sibling != above_sibling_layer) {
            cursor = cursor->next_sibling;
        }

        cursor->next_sibling = layer_to_insert;
    }

    layer_to_insert->next_sibling = above_sibling_layer;
    layer_to_insert->parent = above_sibling_layer->parent;
    layer_to_insert->window = above_sibling_layer->window;
    layer_mark_dirty(layer_to_insert);
}


bool layer_get_hidden(Layer *layer) {
    return layer->hidden;
}

void layer_set_clips(Layer *layer, bool clips) {
    layer->clips = clips;
    layer_mark_dirty (layer);
}

bool layer_get_clips(Layer *layer) {
    return layer->clips;
}
