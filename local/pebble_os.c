#include "globals.h"

static bool clock24hStyle=true;

void inverter_layer_update (Layer* me,GContext* ctx) {
    SDL_Surface* myScreen=getTopScreen ();
    SDL_Rect rect;
    uint32_t* pixel;
    int x,y;
    meltScreens();
    SDL_GetClipRect(myScreen,&rect);
    LOCK(myScreen);
    if (rect.w>0&&rect.h>0) {
        for (y=rect.y;y<rect.y+rect.h;y++) {
            for (x=rect.x;x<rect.x+rect.w;x++) {
                pixel=(uint32_t*)(((uint8_t*)myScreen->pixels)+y*myScreen->pitch+4*x);
                *pixel=~(*pixel) | 0x000000ff;
            }
        }
    }
    UNLOCK(myScreen);
}

InverterLayer* inverter_layer_create(GRect frame) {
    Layer* layer=layer_create(frame);
    if (!layer)
        return 0;
    layer_set_update_proc(layer,inverter_layer_update);
    return (InverterLayer*) layer;
}

void inverter_layer_destroy (InverterLayer* layer) {
    if (layer)
        layer_destroy((Layer*)layer);
}

Layer* inverter_layer_get_layer (InverterLayer* layer) {
    return (Layer*)layer;
}

void app_log(uint8_t log_level, const char *src_filename, int src_line_number, const char *fmt, ...) {
    static const char* logLevels []={"ERROR","WARNING","INFO","DEBUG","DBG-VERBOSE"};
    va_list args;
    log_level=log_level/50-(log_level>100?1:0); //To map log_level to the element in the array ^
    printf ("[LOG][%s][%s:%d] ",logLevels[log_level],src_filename,src_line_number);
    if (logFile!=0)
        fprintf (logFile,"[%s][%s:%d] ",logLevels[log_level],src_filename,src_line_number);
    va_start(args,fmt);
    vfprintf (stdout,fmt,args);
    printf ("\n");
    va_end (args);
    if (logFile!=0) {
        va_start(args,fmt);
        vfprintf (logFile,fmt,args);
        fprintf (logFile,"\n");
        va_end (args);
    }
}

void copyResName(char* name,int id) {
    memset(name,0,MAX_RESOURCE_NAME);
    sprintf (name,RESOURCE_NAME_BASE,id);
}

ResHandle resource_get_handle(uint32_t file_id) {
    return (ResHandle)file_id;
}

size_t resource_load(ResHandle h, uint8_t *buffer, size_t max_length) {
    char name[MAX_RESOURCE_NAME];
    copyResName(name,(int)h);
    FILE* f=fopen (name,"rb");
    if (f==0) {
        printf ("[WARN] Couldn't load raw resource\n");
        return 0;
    }
    max_length=fread(buffer,1,max_length,f);
    fclose(f);
    return max_length;
}

size_t resource_load_byte_range(ResHandle h, uint32_t start_bytes, uint8_t *data, size_t num_bytes) {
    char name[MAX_RESOURCE_NAME];
    copyResName(name,(int)h);
    FILE* f=fopen(name,"rb");
    if (f==0) {
        printf ("[WARN] Couldn't load raw resource\n");
        return 0;
    }
    fseek(f,start_bytes,SEEK_SET);
    num_bytes=fread(data,1,num_bytes,f);
    fclose(f);
    return num_bytes;
}

size_t resource_size(ResHandle h) {
    size_t len;
    char name[MAX_RESOURCE_NAME];
    copyResName(name,(int)h);
    FILE* f=fopen (name,"rb");
    if (f==0) {
        printf ("[WARN] Couldn't load raw resource\n");
        return 0;
    }
    fseek(f,0,SEEK_END);
    len=ftell (f);
    fclose(f);
    return len;
}

void psleep(int millis) {
    SDL_Delay(millis);
}

bool clock_is_24h_style(void) {
    return clock24hStyle;
}

void toggle_24h_style () {
    clock24hStyle=!clock24hStyle;
}

void clock_copy_time_string(char *buffer, uint8_t size) {
    time_t timeSec=time(0);
    struct tm* tim=localtime(&timeSec);
    if (clock_is_24h_style())
        strftime (buffer,size,"%H:%M",tim);
    else
        strftime(buffer,size,"%I:%M",tim);
}

uint16_t time_ms (time_t* tloc,uint16_t* out_ms) {
    uint32_t ms=SDL_GetTicks();
    ms-=ms/1000; //this is not pretty accurate...
    if (tloc)
        time (tloc);
    if (out_ms)
        *out_ms=(uint16_t)ms;
    return ms;
}

void menu_cell_basic_draw(GContext *ctx, const Layer *cell_layer, const char *title, const char *subtitle, GBitmap *icon);
void menu_cell_title_draw(GContext *ctx, const Layer *cell_layer, const char *title);
void menu_cell_basic_header_draw(GContext *ctx, const Layer *cell_layer, const char *title);
void menu_layer_init(MenuLayer *menu_layer, GRect frame);
Layer *menu_layer_get_layer(const MenuLayer *menu_layer);
void menu_layer_set_callbacks(MenuLayer *menu_layer, void *callback_context, MenuLayerCallbacks callbacks);
void menu_layer_set_click_config_onto_window(MenuLayer *menu_layer, struct Window *window);
void menu_layer_set_selected_next(MenuLayer *menu_layer, bool up, MenuRowAlign scroll_align, bool animated);
void menu_layer_set_selected_index(MenuLayer *menu_layer, MenuIndex index, MenuRowAlign scroll_align, bool animated);
void menu_layer_reload_data(MenuLayer *menu_layer);
int16_t menu_index_compare(MenuIndex *a, MenuIndex *b);

void simple_menu_layer_init(SimpleMenuLayer *simple_menu, GRect frame, Window *window, const SimpleMenuSection *sections, int num_sections, void *callback_context);
Layer *simple_menu_layer_get_layer(const SimpleMenuLayer *simple_menu);
int simple_menu_layer_get_selected_index(const SimpleMenuLayer *simple_menu);
void simple_menu_layer_set_selected_index(SimpleMenuLayer *simple_menu, int index, bool animated);

uint32_t dict_calc_buffer_size(const uint8_t tuple_count, ...);
DictionaryResult dict_write_begin(DictionaryIterator *iter, uint8_t * const buffer, const uint16_t size);
DictionaryResult dict_write_data(DictionaryIterator *iter, const uint32_t key, const uint8_t * const data, const uint16_t size);
DictionaryResult dict_write_cstring(DictionaryIterator *iter, const uint32_t key, const char * const cstring);
DictionaryResult dict_write_int(DictionaryIterator *iter, const uint32_t key, const void *integer, const uint8_t width_bytes, const bool is_signed);
DictionaryResult dict_write_uint8(DictionaryIterator *iter, const uint32_t key, const uint8_t value);
DictionaryResult dict_write_uint16(DictionaryIterator *iter, const uint32_t key, const uint16_t value);
DictionaryResult dict_write_uint32(DictionaryIterator *iter, const uint32_t key, const uint32_t value);
DictionaryResult dict_write_int8(DictionaryIterator *iter, const uint32_t key, const int8_t value);
DictionaryResult dict_write_int16(DictionaryIterator *iter, const uint32_t key, const int16_t value);
DictionaryResult dict_write_int32(DictionaryIterator *iter, const uint32_t key, const int32_t value);
uint32_t dict_write_end(DictionaryIterator *iter);
Tuple *dict_read_begin_from_buffer(DictionaryIterator *iter, const uint8_t * const buffer, const uint16_t size);
Tuple *dict_read_next(DictionaryIterator *iter);
Tuple *dict_read_first(DictionaryIterator *iter);
DictionaryResult dict_serialize_tuplets(DictionarySerializeCallback callback, void *context, const uint8_t tuplets_count, const Tuplet * const tuplets);
DictionaryResult dict_serialize_tuplets_to_buffer(const uint8_t tuplets_count, const Tuplet * const tuplets, uint8_t *buffer, uint32_t *size_in_out);
DictionaryResult dict_serialize_tuplets_to_buffer_with_iter(const uint8_t tuplets_count, const Tuplet * const tuplets, DictionaryIterator *iter, uint8_t *buffer, uint32_t *size_in_out);
DictionaryResult dict_write_tuplet(DictionaryIterator *iter, const Tuplet * const tuplet);
uint32_t dict_calc_buffer_size_from_tuplets(const uint8_t tuplets_count, const Tuplet * const tuplets);
DictionaryResult dict_merge(DictionaryIterator *dest, uint32_t *dest_max_size_in_out, DictionaryIterator *source, const bool update_existing_keys_only, const DictionaryKeyUpdatedCallback key_callback, void *context);
Tuple *dict_find(const DictionaryIterator *iter, const uint32_t key);

void number_window_init(NumberWindow *numberwindow, const char *label, NumberWindowCallbacks callbacks, void *callback_context);
void number_window_set_label(NumberWindow *nw, const char *label);
void number_window_set_max(NumberWindow *numberwindow, int max);
void number_window_set_min(NumberWindow *numberwindow, int min);
void number_window_set_value(NumberWindow *numberwindow, int value);
void number_window_set_step_size(NumberWindow *numberwindow, int step);
int number_window_get_value(const NumberWindow *numberwindow);
