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
    int lsize;
};
struct SystemFont systemFonts [SYSTEM_FONT_COUNT]= {
    {0,FONT_KEY_GOTHIC_14,"./simdata/systemFonts/gothic.ttf",8,14},
    {0,FONT_KEY_GOTHIC_14_BOLD,"./simdata/systemFonts/gothic-bold.ttf",8,14}, // 1 bold
    {0,FONT_KEY_GOTHIC_18,"./simdata/systemFonts/gothic.ttf",10,18},
    {0,FONT_KEY_GOTHIC_18_BOLD,"./simdata/systemFonts/gothic-bold.ttf",10,18}, // 3 bold
    {0,FONT_KEY_GOTHIC_24,"./simdata/systemFonts/gothic.ttf",14,24},
    {0,FONT_KEY_GOTHIC_24_BOLD,"./simdata/systemFonts/gothic-bold.ttf",14,24}, // 5 bold
    {0,FONT_KEY_GOTHIC_28,"./simdata/systemFonts/gothic.ttf",18,28},
    {0,FONT_KEY_GOTHIC_28_BOLD,"./simdata/systemFonts/gothic-bold.ttf",18,28}, // 7 bold
    {0,FONT_KEY_BITHAM_30_BLACK,"./simdata/systemFonts/bitham-black.ttf",30,30},
    {0,FONT_KEY_BITHAM_42_BOLD,"./simdata/systemFonts/bitham-bold.ttf",42,42}, // 9 bold
    {0,FONT_KEY_BITHAM_42_LIGHT,"./simdata/systemFonts/bitham-light.ttf",42,42},
    {0,FONT_KEY_BITHAM_42_MEDIUM_NUMBERS,"./simdata/systemFonts/bitham-medium-numbers.ttf",42,42},
    {0,FONT_KEY_BITHAM_34_MEDIUM_NUMBERS,"./simdata/systemFonts/bitham-medium-numbers.ttf",34,34},
    {0,FONT_KEY_BITHAM_34_LIGHT_SUBSET,"./simdata/systemFonts/bitham-light-subset.ttf",34,34},
    {0,FONT_KEY_BITHAM_18_LIGHT_SUBSET,"./simdata/systemFonts/bitham-light-subset.ttf",18,18},
    {0,FONT_KEY_ROBOTO_CONDENSED_21,"./simdata/systemFonts/roboto-condensed.ttf",21,21},
    {0,FONT_KEY_ROBOTO_BOLD_SUBSET_49,"./simdata/systemFonts/roboto-bold.ttf",49,49},
    {0,FONT_KEY_DROID_SERIF_28_BOLD,"./simdata/systemFonts/droid-serif-bold.ttf",28,28},
};

int lineHeightFromFont(GFont font) {
  for (int i=0; i<SYSTEM_FONT_COUNT; i++) {
    if (systemFonts[i].font == font) 
      return systemFonts[i].lsize;
  }
  return 18;
}

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
            if (systemFonts[i].font==0) {
                systemFonts[i].font=TTF_OpenFont (systemFonts[i].file,systemFonts[i].psize);
                if (systemFonts[i].font==0) {
                    printf ("[WARN] Didn't found system font: %s\n",systemFonts[i].file);
                } else {
                    switch (i) {
                    case 1:
                    case 3:
                    case 5:
                    case 7:
                    case 9:
                        TTF_SetFontStyle(systemFonts[i].font, TTF_STYLE_BOLD);
                        break;
                    }
                }
            }
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

#define TEXT_LAYER_GET Layer* text_layer=(Layer*)l
#define TEXT_GET TEXT_LAYER_GET;TextLayerData* text=(TextLayerData*)layer_get_data(text_layer);

void text_layer_update_func (Layer* l,GContext* ctx) {
    TEXT_GET;
    GRect rect=GRect(0,0,l->frame.size.w,l->frame.size.h);
    graphics_context_set_fill_color(ctx,text->background_color);
    graphics_fill_rect (ctx,rect,0,0);
    graphics_context_set_text_color(ctx,text->text_color);
    graphics_draw_text (ctx,text->text,text->font,rect,text->overflow_mode,text->text_alignment,0);
}

TextLayer* text_layer_create(GRect frame) {
    Layer* layer=layer_create_with_data(frame,sizeof(TextLayerData));
    TextLayerData* text_layer=(TextLayerData*)layer_get_data(layer);
    layer_set_update_proc (layer,text_layer_update_func);
    text_layer->font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    text_layer->text_alignment = GTextAlignmentLeft;
    text_layer->text_color = GColorBlack;
    text_layer->background_color = GColorWhite;
    text_layer->overflow_mode = GTextOverflowModeWordWrap;
    text_layer->should_cache_layout = false;
    return (TextLayer*)layer;
}

void text_layer_destroy (TextLayer* layer) {
    if (layer)
        layer_destroy((Layer*)layer);
}

const char *text_layer_get_text(TextLayer *l) {
    TEXT_GET;
    return text->text;
}

void text_layer_set_text(TextLayer *l, const char *txt) {
    TEXT_GET;
    text->text = txt;
    layer_mark_dirty (text_layer);
}

void text_layer_set_font(TextLayer *l, GFont font) {
    TEXT_GET;
    text->font = font;
    layer_mark_dirty (text_layer);
}

void text_layer_set_text_color(TextLayer *l, GColor color) {
    TEXT_GET;
    text->text_color = color;
    layer_mark_dirty (text_layer);
}

void text_layer_set_background_color(TextLayer *l, GColor color) {
    TEXT_GET;
    text->background_color = color;
    layer_mark_dirty (text_layer);
}

void text_layer_set_overflow_mode(TextLayer *l, GTextOverflowMode line_mode) {
    TEXT_GET
    text->overflow_mode = line_mode;
    layer_mark_dirty (text_layer);
}

void text_layer_set_text_alignment(TextLayer *l, GTextAlignment text_alignment) {
    TEXT_GET;
    text->text_alignment = text_alignment;
    layer_mark_dirty (text_layer);
}

void text_layer_set_size(TextLayer *l, const GSize max_size) {
    TEXT_LAYER_GET;
    GRect frame=layer_get_frame (text_layer);
    frame.size=max_size;
    layer_set_frame (text_layer,frame);
    layer_mark_dirty (text_layer);
}

Layer* text_layer_get_layer (TextLayer* l) {
    return (Layer*)l;
}

GSize text_layer_get_content_size (TextLayer *l) {
    TEXT_GET;
    Layer *ll = (Layer*)l;
    GRect rect=GRect(0,0,ll->frame.size.w,ll->frame.size.h);
    GSize s = _graphics_draw_text (NULL,text->text,text->font,rect,text->overflow_mode,text->text_alignment,0,1);
    return s;
}
