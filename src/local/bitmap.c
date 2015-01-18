#include "globals.h"

typedef struct __attribute__((__packed__)) {
    uint16_t pitch;
    uint16_t info_flags;
    uint32_t type;
    uint16_t width;
    uint16_t height;
} GBitmapHeader;

#ifdef WIN32
	typedef struct __attribute__ ((__packed__)) {
	  void *addr;
	  uint16_t row_size_bytes;
	  union {
		uint16_t flags;
		struct {
		  bool is_heap_allocated:1;
		  uint16_t reserved:11;
		  uint8_t version:4;
		}data;
	  }info;
	} WIN32_GBitmap;
	#define GBITMAP_SET_INFO_FLAGS(bmpPtr,set) ((WIN32_GBitmap*)bmpPtr)->info.flags=set
	#define GBITMAP_SET_IS_HEAP_ALLOCATED(bmpPtr,is) ((WIN32_GBitmap*)bmpPtr)->info.data.is_heap_allocated=is
	#define GBITMAP_GET_INFO_FLAGS(bmpPtr) (((WIN32_GBitmap*)bmpPtr)->info.flags)
	#define GBITMAP_GET_IS_HEAP_ALLOCATED(bmpPtr) (((WIN32_GBitmap*)bmpPtr)->info.data.is_heap_allocated)
#else
	#define GBITMAP_SET_INFO_FLAGS(bmpPtr,set) bmpPtr->info_flags=set
	#define GBITMAP_SET_IS_HEAP_ALLOCATED(bmpPtr,is) bmpPtr->is_heap_allocated=is
	#define GBITMAP_GET_INFO_FLAGS(bmpPtr) (bmpPtr->info_flags)
	#define GBITMAP_GET_IS_HEAP_ALLOCATED(bmpPtr) (bmpPtr->is_heap_allocated)
#endif

GBitmap* gbitmap_create_as_sub_bitmap(const GBitmap *base_bitmap, GRect sub_rect) {
    GBitmap* sub_bitmap=(GBitmap*)malloc(sizeof(GBitmap));
    if (!sub_bitmap) {
        printf ("Memory allocation failed!\n");
        return 0;
    }
    grect_clip (&sub_rect,&base_bitmap->bounds);
    sub_bitmap->addr=base_bitmap->addr;
    GBITMAP_SET_INFO_FLAGS(sub_bitmap,GBITMAP_GET_INFO_FLAGS(base_bitmap));
    GBITMAP_SET_IS_HEAP_ALLOCATED(sub_bitmap,false);
    sub_bitmap->row_size_bytes=base_bitmap->row_size_bytes;
    sub_bitmap->bounds=sub_rect;
    return sub_bitmap;
}

GBitmap* gbitmap_create_with_data (const uint8_t* data) {
    GBitmap* bitmap=(GBitmap*)malloc(sizeof(GBitmap));
    if (!bitmap) {
        printf ("Memory allocation failed!\n");
        return 0;
    }
    GBitmapHeader* header=(GBitmapHeader*)data;
    bitmap->addr=malloc(header->pitch*header->height);
    if (!bitmap->addr) {
        printf ("Memory allocation failed!\n");
        return 0;
    }
    memcpy(bitmap->addr,((uint8_t*)data)+sizeof(GBitmapHeader),header->pitch*header->height);
    bitmap->row_size_bytes=header->pitch;
    GBITMAP_SET_INFO_FLAGS(bitmap,header->info_flags);
    GBITMAP_SET_IS_HEAP_ALLOCATED(bitmap,true);
    bitmap->bounds=GRect(0,0,header->width,header->height);
    free((uint8_t*)data); //TODO: Verify this!
    return bitmap;
}

GBitmap* gbitmap_create_with_resource (uint32_t resource_id) {
    printf ("[DEBUG] Load resource ID:%d as image\n",resource_id);
    char name[MAX_RESOURCE_NAME];
    copyResName(name, RES_ID_TO_HANDLE(resource_id));
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
    size_t readLen=fread(data,1,len,f);
    fclose(f);
    if (len!=readLen) {
        printf ("[ERROR] Read error!\n");
        free(data);
        return 0;
    }
    return gbitmap_create_with_data (data);
}

void gbitmap_destroy(GBitmap* bitmap) {
    if (bitmap&&bitmap->addr&&GBITMAP_GET_IS_HEAP_ALLOCATED(bitmap)) {
        free(bitmap->addr);
        bitmap->addr=0;
    }
}

#define BITMAP_LAYER_GET Layer* bitmap_layer=(Layer*)l
#define BITMAP_GET BITMAP_LAYER_GET;BitmapLayerData* bitmap=(BitmapLayerData*)layer_get_data(bitmap_layer)
#define ROT_BITMAP_LAYER_GET Layer* rotbitmap_layer=(Layer*)l
#define ROT_BITMAP_GET BITMAP_LAYER_GET;RotBitmapLayerData* rotbitmap=(RotBitmapLayerData*)layer_get_data(bitmap_layer)

void bitmap_layer_update_func (Layer* l,GContext* ctx) {
    BITMAP_GET;
    GRect rect=GRect(0,0,l->frame.size.w,l->frame.size.h);
    graphics_context_set_fill_color(ctx,bitmap->background_color);
    graphics_fill_rect (ctx,rect,0,0);
    rect.origin=l->frame.origin;
    grect_align (&rect,&l->frame,bitmap->alignment,l->clips);
    rect.origin.x-=l->frame.origin.x;
    rect.origin.y-=l->frame.origin.y;
    graphics_context_set_compositing_mode (ctx,bitmap->compositing_mode);
    graphics_draw_bitmap_in_rect (ctx,bitmap->bitmap,rect);
}

BitmapLayer * bitmap_layer_create(GRect frame) {
    Layer* layer=layer_create_with_data(frame,sizeof(BitmapLayerData));
    if (!layer)
        return 0;
    BitmapLayerData* bitmap=(BitmapLayerData*)layer_get_data(layer);
    layer_set_update_proc(layer,bitmap_layer_update_func);
    bitmap->bitmap=0;
    bitmap->background_color=GColorClear;
    bitmap->alignment=GAlignCenter;
    bitmap->compositing_mode=GCompOpAssign;
    return (BitmapLayer*)layer;
}

void bitmap_layer_destroy (BitmapLayer* layer) {
    if (layer)
        layer_destroy((Layer*)layer);
}

Layer* bitmap_layer_get_layer (const BitmapLayer* layer) {
    return (Layer*)layer;
}

void bitmap_layer_set_bitmap(BitmapLayer *l, const GBitmap *image) {
    BITMAP_GET;
    bitmap->bitmap=image;
}

void bitmap_layer_set_alignment(BitmapLayer *l, GAlign alignment) {
    BITMAP_GET;
    bitmap->alignment=alignment;
}

void bitmap_layer_set_background_color(BitmapLayer *l, GColor color) {
    BITMAP_GET;
    bitmap->background_color=color;
}

void bitmap_layer_set_compositing_mode(BitmapLayer *l, GCompOp mode) {
    BITMAP_GET;
    bitmap->compositing_mode=mode;
}

void rot_bitmap_layer_update_func (Layer* l,GContext* ctx) {
    ROT_BITMAP_GET;
    GPoint topOffset=getTopOffset ();
    setTopOffset(GPoint(0,0));
    GRect rect=GRect(0,0,l->frame.size.w,l->frame.size.h);

    SDL_Surface* sur=createSurface(rotbitmap->bitmap->bounds.size.w,rotbitmap->bitmap->bounds.size.h);
    SDL_FillRect(sur,0,0);
    graphics_context_set_compositing_mode (ctx,GCompOpAssign);
    graphics_draw_bitmap_in_rect_to (ctx,rotbitmap->bitmap,rect,sur);
    double angle=(double)rotbitmap->rotation/TRIG_MAX_ANGLE*360.0;
    SDL_Surface* rotated=rotozoomSurface(sur,-angle,1.0,SMOOTHING_OFF);
    SDL_FreeSurface(sur);
    GPoint offset=getPivotRotationOffset(rotbitmap->bitmap->bounds.size,GSize(rotated->w,rotated->h),rotbitmap->src_ic,angle);

    rotbitmap->dest_ic.x=l->frame.size.w/2; //TODO: Verify this
    rotbitmap->dest_ic.y=l->frame.size.h/2;

    setTopOffset(topOffset);
    if (rotbitmap->corner_clip_color!=GColorClear) {
        graphics_context_set_fill_color(ctx,rotbitmap->corner_clip_color);
        graphics_fill_rect(ctx,GRect(0,0,l->frame.size.w,l->frame.size.h),0,0);
    }
    graphics_context_set_compositing_mode (ctx,rotbitmap->compositing_mode);
    graphics_draw_surface_in_rect (ctx,rotated,GRect(rotbitmap->dest_ic.x-offset.x,rotbitmap->dest_ic.y-offset.y,rotated->w,rotated->h));
    SDL_FreeSurface(rotated);
}

RotBitmapLayer* rot_bitmap_layer_create (GBitmap* bitmap) {
    Layer* layer=layer_create_with_data(GRect(0,0,bitmap->bounds.size.w,bitmap->bounds.size.h),sizeof(RotBitmapLayerData));
    RotBitmapLayerData* rotbitmap=(RotBitmapLayerData*)layer_get_data(layer);
    layer_set_update_proc(layer,rot_bitmap_layer_update_func);
    rotbitmap->bitmap=bitmap;
    rotbitmap->corner_clip_color=GColorClear;
    rotbitmap->rotation=0;
    rotbitmap->src_ic=GPoint(0,0);
    rotbitmap->dest_ic=GPoint(bitmap->bounds.size.w/2,bitmap->bounds.size.h/2);
    rotbitmap->compositing_mode=GCompOpAssign;
    return (RotBitmapLayer*)layer;
}

void rot_bitmap_layer_destroy (RotBitmapLayer* layer) {
    if (layer!=0)
        layer_destroy((Layer*)layer);
}

void rot_bitmap_layer_set_corner_clip_color (RotBitmapLayer* l,GColor color) {
    ROT_BITMAP_GET;
    rotbitmap->corner_clip_color=color;
}

void rot_bitmap_layer_set_angle(RotBitmapLayer* l,int32_t angle) {
    ROT_BITMAP_GET;
    rotbitmap->rotation=angle;
}

void rot_bitmap_layer_increment_angle (RotBitmapLayer* l,int32_t angle_change) {
    ROT_BITMAP_GET;
    rotbitmap->rotation+=angle_change;
}

void rot_bitmap_set_src_ic(RotBitmapLayer* l,GPoint ic) {
    ROT_BITMAP_GET;
    rotbitmap->src_ic=ic;
}

void rot_bitmap_set_compositing_mode(RotBitmapLayer* l,GCompOp mode) {
    ROT_BITMAP_GET;
    rotbitmap->compositing_mode=mode;
}
