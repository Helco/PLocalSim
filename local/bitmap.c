#include "globals.h"

typedef struct __attribute__((__packed__)) {
    uint16_t pitch;
    uint16_t unknown;
    uint32_t type;
    uint16_t width;
    uint16_t height;
} GBitmapHeader;

void gbitmap_init_as_sub_bitmap(GBitmap *sub_bitmap, const GBitmap *base_bitmap, GRect sub_rect) {
    grect_clip (&sub_rect,&base_bitmap->bounds);
    sub_bitmap->addr=base_bitmap->addr;
    sub_bitmap->info_flags=base_bitmap->info_flags;
    sub_bitmap->row_size_bytes=base_bitmap->row_size_bytes;
    sub_bitmap->bounds=sub_rect;
}

void gbitmap_init_with_data (GBitmap* bitmap,const uint8_t* data) {
    GBitmapHeader* header=(GBitmapHeader*)data;
    bitmap->addr=((uint8_t*)data)+(sizeof(GBitmapHeader));
    bitmap->row_size_bytes=header->pitch;
    bitmap->info_flags=0;
    bitmap->bounds=GRect(0,0,header->width,header->height);
}

void* gbitmap_init_from_resource (GBitmap* bitmap,int resource_id) {
    printf ("[DEBUG] Load resource ID:%d as image\n",resource_id);
    char name[MAX_RESOURCE_NAME];
    copyResName(name,resource_id);
    FILE* f=fopen(name,"rb");
    if (!f) {
        printf("[ERROR] Couldn't load \"%s\"(%s)!\n",name,SDL_GetError());
        return 0;
    }
    fseek(f,0,SEEK_END);
    size_t len=ftell(f);
    fseek(f,0,SEEK_SET);
    void* data=malloc(len);
    if (!data) {
        printf("[ERROR] Couldn't load \"%s\"(Memory allocation failed)!\n",name);
        return 0;
    }
    fread(data,1,len,f);
    gbitmap_init_with_data (bitmap,data);
    return data;
}

bool bmp_init_container(int resource_id, BmpContainer *c) {
    c->data=gbitmap_init_from_resource (&c->bmp,resource_id);
    bitmap_layer_init (&c->layer,c->bmp.bounds);
    bitmap_layer_set_bitmap (&c->layer,&c->bmp);
    return c->data!=0;
}

void bmp_deinit_container(BmpContainer *c) {
    if (c->data!=0) {
        free (c->data);
        c->data=0;
    }
}

bool heap_bitmap_init(HeapBitmap *c, int resource_id) {
    c->data=gbitmap_init_from_resource(&c->bmp,resource_id);
    return c->data!=0;
}

void heap_bitmap_deinit(HeapBitmap *c) {
    if (c->data!=0) {
        free (c->data);
        c->data=0;
    }
}

bool rotbmp_init_container(int resource_id, RotBmpContainer *c) {
    c->data=gbitmap_init_from_resource (&c->bmp,resource_id);
    c->layer.bitmap=&c->bmp;
    layer_init ((Layer*)&c->layer,c->bmp.bounds);
    return c->data!=0;
}

void rotbmp_deinit_container(RotBmpContainer *c) {
    if (c->data!=0) {
        free (c->data);
        c->data=0;
    }
}

bool rotbmp_pair_init_container(int white_resource_id, int black_resource_id, RotBmpPairContainer *c) {
    c->white_data=gbitmap_init_from_resource(&c->white_bmp,white_resource_id);
    if (!c->white_data)
        return false;
    c->black_data=gbitmap_init_from_resource(&c->black_bmp,black_resource_id);
    if (!c->black_data)
        return false;
    c->layer.white_layer.bitmap=&c->white_bmp;
    c->layer.black_layer.bitmap=&c->black_bmp;
    layer_init ((Layer*)&c->layer,c->white_bmp.bounds);
    layer_init ((Layer*)&c->layer.white_layer,c->white_bmp.bounds);
    layer_init ((Layer*)&c->layer.black_layer,c->white_bmp.bounds);
    return true;
}

void rotbmp_pair_deinit_container(RotBmpPairContainer *c) {
    if (c->white_data!=0) {
        free (c->white_data);
        free (c->black_data);
        c->white_data=0;
        c->black_data=0;
    }
}

void bitmap_layer_update_func (Layer* me,GContext* ctx) {
    GRect rect=GRect(0,0,me->frame.size.w,me->frame.size.h);
    BitmapLayer* bitmapLayer=(BitmapLayer*)me;
    graphics_context_set_fill_color(ctx,bitmapLayer->background_color);
    graphics_fill_rect (ctx,rect,0,0);
    rect.origin=me->frame.origin;
    grect_align (&rect,&me->frame,bitmapLayer->alignment,me->clips);
    rect.origin.x-=me->frame.origin.x;
    rect.origin.y-=me->frame.origin.y;
    graphics_draw_bitmap_in_rect (ctx,bitmapLayer->bitmap,rect);
}

void bitmap_layer_init(BitmapLayer *image, GRect frame) {
    layer_init ((Layer*)image,frame);
    layer_set_update_proc((Layer*)image,bitmap_layer_update_func);
    image->bitmap=0;
    image->background_color=GColorClear;
    image->alignment=GAlignCenter;
    image->compositing_mode=GCompOpAssign;
}

void bitmap_layer_set_bitmap(BitmapLayer *image, const GBitmap *bitmap) {
    image->bitmap=bitmap;
}

void bitmap_layer_set_alignment(BitmapLayer *image, GAlign alignment) {
    image->alignment=alignment;
}

void bitmap_layer_set_background_color(BitmapLayer *image, GColor color) {
    image->background_color=color;
}

void bitmap_layer_set_compositing_mode(BitmapLayer *image, GCompOp mode) {
    image->compositing_mode=mode;
}

//#todo #sprint1
void rotbmp_pair_layer_set_src_ic(RotBmpPairLayer *pair, GPoint ic);

//#todo #sprint1
void rotbmp_pair_layer_set_angle(RotBmpPairLayer *pair, int32_t angle);
