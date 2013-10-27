#include "globals.h"

void window_init(Window *window, const char *debug_name) {
    printf("[INFO] Loading window: %s\n", debug_name);
    window->debug_name=debug_name;
    window->is_fullscreen=((APP_INFO.flags & APP_INFO_WATCH_FACE) == APP_INFO_WATCH_FACE);
    window->overrides_back_button=false;
    window->is_loaded=false;
    window->on_screen=false;
    window->is_render_scheduled=false;
    window->background_color=GColorBlack;
    window->user_data=0;
    window->click_config_context=0;
    window->status_bar_icon=0;
    window->click_config_provider=0;
    window->input_handlers.buttons.up=0;
    window->input_handlers.buttons.down=0;
    layer_init(&window->layer, GRect(0, 0, 144, 168));
    // This seems to be causing problems:
    //layer_init (&window->layer, (window->is_fullscreen ? GRect(0,0,144,168) : GRect(0, 16, 144, 152))); 
    window->layer.window=window;
    window->window_handlers.load=0;
    window->window_handlers.unload=0;
    window->window_handlers.appear=0;
    window->window_handlers.disappear=0;
}

void window_set_click_config_provider(Window *window, ClickConfigProvider click_config_provider) {
    window->click_config_provider = click_config_provider;
    if (window_stack_get_top_window()==window)
        buttonsUpdateWindow (window);
}

void window_set_background_color(Window *window, GColor background_color) {
    if (window->background_color!=background_color) {
        window->background_color=background_color;
        layer_mark_dirty (&window->layer);
    }
}

void window_set_fullscreen(Window *window, bool enabled) {
    window->is_fullscreen = enabled;

    if (enabled==true)
        window->layer.bounds.origin.y=0;
    else
        window->layer.bounds.origin.y=16;
}

void window_deinit(Window *window) {
    if (window->window_handlers.unload!=0)
        window->window_handlers.unload (window);
    if (window_stack_contains_window (window))
        window_stack_remove(window,false);
}

void window_set_click_config_provider_with_context(Window *window, ClickConfigProvider click_config_provider, void *context) {
    window->click_config_provider=click_config_provider;
    window->click_config_context=context;
    if (window_stack_get_top_window()==window)
        buttonsUpdateWindow (window);
}

ClickConfigProvider window_get_click_config_provider(Window *window) {
    return window->click_config_provider;
}

void window_set_window_handlers(Window *window, WindowHandlers handlers) {
    window->window_handlers=handlers;
}

struct Layer *window_get_root_layer(Window *window) {
    return &window->layer;
}

bool window_get_fullscreen(Window *window) {
    return window->is_fullscreen;
}

void window_set_status_bar_icon(Window *window, const GBitmap *icon) {
    window->status_bar_icon=icon;
}

bool window_is_loaded(Window *window) {
    return window->is_loaded;
}

void window_stack_push(Window *window, bool animated) {
    pushWindow(window);
}

Window *window_stack_pop(bool animated) {
    Window* w=window_stack_get_top_window ();
    popWindow ();
    return w; //no documentation about return?
}

void window_stack_pop_all(const bool animated) {
    while (window_stack_get_top_window ())
        popWindow ();
}
