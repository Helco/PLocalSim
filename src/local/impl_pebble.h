#ifndef __IMPL_PEBBLE_H__
#define __IMPL_PEBBLE_H__
#include "pebble.h"
#include "pls_unwarn_unsupported_functions.h"
#undef tzname

//these structure are based from the ones in pebbleSDK 1.2 and modified to fit the new pebbleSDK
typedef struct ClickConfig {
    void *context;
    struct click {
        ClickHandler handler;
        uint16_t repeat_interval_ms;
    } click;
    struct multi_click {
        uint8_t min;
        uint8_t max;
        bool last_click_only;
        ClickHandler handler;
        uint16_t timeout;
    } multi_click;
    struct long_click {
        uint16_t delay_ms;
        ClickHandler handler;
        ClickHandler release_handler;
    } long_click;
    struct raw {
        ClickHandler up_handler;
        ClickHandler down_handler;
        void *context;
    } raw;
} ClickConfig;
typedef struct Layer {
    GRect bounds;
    GRect frame;
    bool clips : 1;
    bool hidden : 1;
    struct Layer *next_sibling;
    struct Layer *parent;
    struct Layer *first_child;
    struct Window *window;
    LayerUpdateProc update_proc;
    void* extra_data;
} Layer;
typedef struct ActionBarLayerData {
    ClickConfigProvider click_config_provider;
    void* context;
    GColor background_color;
    const GBitmap* icons [3];
    uint16_t is_highlighted;
} ActionBarLayerData;
typedef struct {
    const GBitmap *bitmap;
    GColor background_color : 2;
    GAlign alignment : 4;
    GCompOp compositing_mode : 3;
} BitmapLayerData;
typedef struct {
    const GBitmap *bitmap;
    GColor corner_clip_color;
    int32_t rotation;
    GPoint src_ic;
    GPoint dest_ic;
    GCompOp compositing_mode;
} RotBitmapLayerData;
typedef struct {
    const char *text;
    GFont font;
    GTextLayoutCacheRef layout_cache;
    GColor text_color : 2;
    GColor background_color : 2;
    GTextOverflowMode overflow_mode : 2;
    GTextAlignment text_alignment : 2;
    bool should_cache_layout : 1;
} TextLayerData;
typedef struct
{
  Layer* content_sublayer;
  Layer* shadow_sublayer;
  PropertyAnimation* animation;
  ScrollLayerCallbacks callbacks;
  void *context;
} ScrollLayerData;
typedef struct Window {
    Layer* layer;
    const GBitmap *status_bar_icon;
    WindowHandlers window_handlers;
    ClickConfigProvider click_config_provider;
    void *click_config_context;
    void *user_data;
    GColor background_color : 2;
    bool is_loaded : 1;
    bool is_fullscreen : 1;
} Window;

#endif // __IMPL_PEBBLE_H__
