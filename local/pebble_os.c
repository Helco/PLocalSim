#include "globals.h"

static bool clock24hStyle=true;
static int timer_number = 0;



AppTimerHandle app_timer_send_event(AppContextRef app_ctx, uint32_t timeout_ms, uint32_t cookie) {

	uint32_t handle = timer_number++;

	create_new_timer(timeout_ms, cookie, handle);

	return handle;
}


bool app_timer_cancel_event(AppContextRef app_ctx_ref, AppTimerHandle handle) {

	return remove_timer(search_timer_with_handle(handle));

}


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

void inverter_layer_init(InverterLayer *inverter, GRect frame) {
    layer_init ((Layer*)inverter,frame);
    layer_set_update_proc ((Layer*)inverter,inverter_layer_update);
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

void resource_init_current_app(ResVersionHandle version) {
    printf("[DEBUG] [NOOP] Successfully loaded resource map!\n");
}

void copyResName(char* name,int id) {
    memset(name,0,MAX_RESOURCE_NAME);
    sprintf (name,RESOURCE_NAME_BASE,id-1);
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

void get_time(PblTm *_time) {
    time_t raw;
    struct tm *now;

    time(&raw);
    now = localtime(&raw);

    _time->tm_sec = now->tm_sec;
    _time->tm_min = now->tm_min;
    _time->tm_hour = now->tm_hour;
    _time->tm_mday = now->tm_mday;
    _time->tm_mon = now->tm_mon;
    _time->tm_year = now->tm_year;
    _time->tm_wday = now->tm_wday;
    _time->tm_yday = now->tm_yday;
    _time->tm_isdst = now->tm_isdst;
}

void psleep(int millis) {
#ifdef WIN32
    Sleep ((uint32_t)millis);
#else
    sleep((uint32_t)millis);
#endif
}


void make_time(const PblTm *now, struct tm *time) {
    time->tm_sec = now->tm_sec;
    time->tm_min = now->tm_min;
    time->tm_hour = now->tm_hour;
    time->tm_mday = now->tm_mday;
    time->tm_mon = now->tm_mon;
    time->tm_year = now->tm_year;
    time->tm_wday = now->tm_wday;
    time->tm_yday = now->tm_yday;
    time->tm_isdst = now->tm_isdst;
}

void string_format_time(char *ptr, size_t maxsize, const char *format, const PblTm *timeptr) {
    struct tm tm_time;
    make_time(timeptr, &tm_time);
    mktime(&tm_time);

    size_t actual = strftime(ptr, maxsize, format, &tm_time);
    if(actual == 0)
        printf("[WARN] strftime failed - %d:%s\n", maxsize, format);

}

bool clock_is_24h_style(void) {
    return clock24hStyle;
}

void toggle_24h_style () {
    clock24hStyle=!clock24hStyle;
}

void clock_copy_time_string(char *buffer, uint8_t size) {
    PblTm tim;
    get_time (&tim);
    if (clock_is_24h_style())
        string_format_time (buffer,size,"%H:%M",&tim);
    else
        string_format_time (buffer,size,"%I:%M",&tim);
}

uint16_t time_ms (time_t* tloc,uint16_t* out_ms) {
    uint16_t ms=SDL_GetTicks();
    ms-=ms/1000; //this is not pretty accurate...
    if (tloc)
        time (tloc);
    if (out_ms)
        *out_ms=ms;
    return ms;
}

void menu_cell_basic_draw(GContext *ctx, const Layer *cell_layer, const char *title, const char *subtitle, GBitmap *icon);
void menu_cell_title_draw(GContext *ctx, const Layer *cell_layer, const char *title);
void menu_cell_basic_header_draw(GContext *ctx, const Layer *cell_layer, const char *title);
void menu_layer_init(MenuLayer *menu_layer, GRect frame);
Layer *menu_layer_get_layer(MenuLayer *menu_layer);
void menu_layer_set_callbacks(MenuLayer *menu_layer, void *callback_context, MenuLayerCallbacks callbacks);
void menu_layer_set_click_config_onto_window(MenuLayer *menu_layer, struct Window *window);
void menu_layer_set_selected_next(MenuLayer *menu_layer, bool up, MenuRowAlign scroll_align, bool animated);
void menu_layer_set_selected_index(MenuLayer *menu_layer, MenuIndex index, MenuRowAlign scroll_align, bool animated);
void menu_layer_reload_data(MenuLayer *menu_layer);
int16_t menu_index_compare(MenuIndex *a, MenuIndex *b);

void simple_menu_layer_init(SimpleMenuLayer *simple_menu, GRect frame, Window *window, const SimpleMenuSection *sections, int num_sections, void *callback_context);
Layer *simple_menu_layer_get_layer(SimpleMenuLayer *simple_menu);
int simple_menu_layer_get_selected_index(SimpleMenuLayer *simple_menu);
void simple_menu_layer_set_selected_index(SimpleMenuLayer *simple_menu, int index, bool animated);

AppMessageResult app_message_register_callbacks(AppMessageCallbacksNode *callbacks_node);
AppMessageResult app_message_deregister_callbacks(AppMessageCallbacksNode *callbacks_node);
AppMessageResult app_message_out_get(DictionaryIterator **iter_out);
AppMessageResult app_message_out_send(void);
AppMessageResult app_message_out_release(void);
void app_sync_init(AppSync *s, uint8_t *buffer, const uint16_t buffer_size, const Tuplet * const keys_and_initial_values, const uint8_t count, AppSyncTupleChangedCallback tuple_changed_callback, AppSyncErrorCallback error_callback, void *context);
void app_sync_deinit(AppSync *s);
AppMessageResult app_sync_set(AppSync *s, const Tuplet * const keys_and_values_to_update, const uint8_t count);
const Tuple *app_sync_get(const AppSync *s, const uint32_t key);

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
int number_window_get_value(NumberWindow *numberwindow);
