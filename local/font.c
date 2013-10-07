#include "globals.h"

enum SystemFontID {
    SYSTEM_FONT_GOTHIC_14=0,
    SYSTEM_FONT_GOTHIC_14_BOLD,
    SYSTEM_FONT_GOTHIC_18,
    SYSTEM_FONT_GOTHIC_18_BOLD,
    SYSTEM_FONT_GOTHIC_24,
    SYSTEM_FONT_GOTHIC_24_BOLD,
    SYSTEM_FONT_GOTHIC_28,
    SYSTEM_FONT_GOTHIC_28_BOLD,
    SYSTEM_FONT_BITHAM_30,
    SYSTEM_FONT_BITHAM_42_BOLD,
    SYSTEM_FONT_BITHAM_42_LIGHT,
    SYSTEM_FONT_BITHAM_42_MEDIUM_NUMBERS,
    SYSTEM_FONT_BITHAM_34_MEDIUM_NUMBERS,
    SYSTEM_FONT_BITHAM_34_LIGHT_SUBSET,
    SYSTEM_FONT_BITHAM_18_LIGHT_SUBSET,
    SYSTEM_FONT_ROBOTO_CONDENSED_21,
    SYSTEM_FONT_ROBOTO_BOLD_SUBSET_49,
    SYSTEM_DROID_SERIF_28_BOLD,
    SYSTEM_FONT_COUNT
};
struct SystemFont {
    TTF_Font* font;
    const char* key;
    const char* file;
    int psize;
};
struct SystemFont systemFonts [SYSTEM_FONT_COUNT]= {
    {0,FONT_KEY_GOTHIC_14,"./simdata/systemFonts/gothic.ttf",14},
    {0,FONT_KEY_GOTHIC_14_BOLD,"./simdata/systemFonts/gothic-bold.ttf",14},
    {0,FONT_KEY_GOTHIC_18,"./simdata/systemFonts/gothic.ttf",18},
    {0,FONT_KEY_GOTHIC_18_BOLD,"./simdata/systemFonts/gothic-bold.ttf",18},
    {0,FONT_KEY_GOTHIC_24,"./simdata/systemFonts/gothic.ttf",24},
    {0,FONT_KEY_GOTHIC_24_BOLD,"./simdata/systemFonts/gothic-bold.ttf",24},
    {0,FONT_KEY_GOTHIC_28,"./simdata/systemFonts/gothic.ttf",28},
    {0,FONT_KEY_GOTHIC_28_BOLD,"./simdata/systemFonts/gothic-bold.ttf",28},
    {0,FONT_KEY_BITHAM_30_BLACK,"./simdata/systemFonts/bitham-black.ttf",30},
    {0,FONT_KEY_BITHAM_42_BOLD,"./simdata/systemFonts/bitham-bold.ttf",42},
    {0,FONT_KEY_BITHAM_42_LIGHT,"./simdata/systemFonts/bitham-light.ttf",42},
    {0,FONT_KEY_BITHAM_42_MEDIUM_NUMBERS,"./simdata/systemFonts/bitham-medium-numbers.ttf",42},
    {0,FONT_KEY_BITHAM_34_MEDIUM_NUMBERS,"./simdata/systemFonts/bitham-medium-numbers.ttf",34},
    {0,FONT_KEY_BITHAM_34_LIGHT_SUBSET,"./simdata/systemFonts/bitham-light-subset.ttf",34},
    {0,FONT_KEY_BITHAM_18_LIGHT_SUBSET,"./simdata/systemFonts/bitham-light-subset.ttf",18},
    {0,FONT_KEY_ROBOTO_CONDENSED_21,"./simdata/systemFonts/roboto-condensed.ttf",21},
    {0,FONT_KEY_ROBOTO_BOLD_SUBSET_49,"./simdata/systemFonts/roboto-bold.ttf",49},
    {0,FONT_KEY_DROID_SERIF_28_BOLD,"./simdata/systemFonts/droid-serif-bold.ttf",28},
};

void unloadSystemFonts() {
    int i=0;
    for (; i<SYSTEM_FONT_COUNT; i++) {
        if (systemFonts[i].font!=0) {
            TTF_CloseFont(systemFonts[i].font);
            systemFonts[i].font=0;
        }
    }
}

GFont fonts_get_system_font(const char *font_key) {
    int i;
    for (i=0; i<SYSTEM_FONT_COUNT; i++) {
        if (strcmp(font_key,systemFonts[i].key)==0) {
            if (systemFonts[i].font==0)
                systemFonts[i].font=TTF_OpenFont (systemFonts[i].file,systemFonts[i].psize);
            if (systemFonts[i].font==0)
                printf ("[WARN] Didn't found system font: %s\n",systemFonts[i].file);
            return (GFont)systemFonts[i].font;
        }
    }
    printf ("[WARN] Invalid system font: %s\n",font_key);
    return 0;
}

GFont fonts_load_custom_font(ResHandle res) {
    char name[MAX_RESOURCE_NAME];
    copyResName(name,(int)res);
    FILE* f=fopen (name,"rb");
    if (!f) {
        printf ("[WARN] Couldn't load custom font!\n");
        return 0;
    }
    int height;
    if (fread(&height,1,sizeof(int),f)==0) {
        printf ("[WARN] Couldn't read custom font height!\n");
        return 0;
    }
    fclose(f);
    strcat(name,"_f");
    TTF_Font* font=TTF_OpenFont (name,height);
    if (!font) {
        printf ("[WARN] Couldn't load custom font (\"%s\",%d,%s)!\n",name,height,TTF_GetError());
        return 0;
    }
    return (GFont)font;
}

void fonts_unload_custom_font(GFont font) {
    TTF_CloseFont((TTF_Font*)font);
}

void text_layer_update_func (Layer* me,GContext* ctx) {
    GRect rect=GRect(0,0,me->frame.size.w,me->frame.size.h);
    TextLayer* textLayer=(TextLayer*)me;
    graphics_context_set_fill_color(ctx,textLayer->background_color);
    graphics_fill_rect (ctx,rect,0,0);
    graphics_context_set_text_color(ctx,textLayer->text_color);
    graphics_text_draw (ctx,textLayer->text,textLayer->font,rect,textLayer->overflow_mode,textLayer->text_alignment,0);
}

void text_layer_init(TextLayer *text_layer, GRect frame) {
    layer_init ((Layer*)text_layer,frame);
    layer_set_update_proc ((Layer*)text_layer,text_layer_update_func);

    //Defaults taken from the SDK's documentation.
    text_layer->font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    text_layer->text_alignment = GTextAlignmentLeft;
    text_layer->text_color = 0;
    text_layer->background_color = 1;
    text_layer->overflow_mode = GTextOverflowModeWordWrap;
    text_layer->should_cache_layout = false;
}

const char *text_layer_get_text(TextLayer *text_layer) {
    return text_layer->text;
}

void text_layer_set_text(TextLayer *text_layer, const char *text) {
    text_layer->text = text;
    layer_mark_dirty ((Layer*)text_layer);
}

void text_layer_set_font(TextLayer *text_layer, GFont font) {
    text_layer->font = font;
    layer_mark_dirty ((Layer*)text_layer);
}

void text_layer_set_text_color(TextLayer *text_layer, GColor color) {
    text_layer->text_color = color;
    layer_mark_dirty ((Layer*)text_layer);
}

void text_layer_set_background_color(TextLayer *text_layer, GColor color) {
    text_layer->background_color = color;
    layer_mark_dirty ((Layer*)text_layer);
}

void text_layer_set_overflow_mode(TextLayer *text_layer, GTextOverflowMode line_mode) {
    text_layer->overflow_mode = line_mode;
    layer_mark_dirty ((Layer*)text_layer);
}

void text_layer_set_text_alignment(TextLayer *text_layer, GTextAlignment text_alignment) {
    text_layer->text_alignment = text_alignment;
    layer_mark_dirty ((Layer*)text_layer);
}

GSize graphics_text_layout_get_max_used_size(GContext *ctx, const char *text, const GFont font, const GRect box, const GTextOverflowMode overflow_mode, const GTextAlignment alignment, GTextLayoutCacheRef layout) {
    //TODO: Implement proper
    return box.size;
}

void text_layer_set_size(TextLayer *text_layer, const GSize max_size) {
    GRect frame=layer_get_frame ((Layer*)text_layer);
    frame.size=max_size;
    layer_set_frame ((Layer*)text_layer,frame);
    layer_mark_dirty ((Layer*)text_layer);
}
