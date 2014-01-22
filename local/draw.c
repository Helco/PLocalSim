#include "globals.h"

static GContext current_graphics_context={
    .fill_color=GColorBlack,
    .stroke_color=GColorBlack,
    .text_color=GColorWhite,
    .compositing_mode=GCompOpAssign
};

GContext *app_get_current_graphics_context(void) {
    return &current_graphics_context;
}

uint32_t getRawColor(uint8_t color) {
    switch(color) {
    case 0:
        return r_black;
    case 1:
        return r_white;
    default:
        return r_clear;
    }
}

SDL_Color s_white = {255, 255, 255, SDL_ALPHA_OPAQUE};
SDL_Color s_black = {0, 0, 0, SDL_ALPHA_OPAQUE};
SDL_Color s_clear = {255, 255, 255, SDL_ALPHA_TRANSPARENT};

SDL_Color getColor(uint8_t color) {
    switch(color) {
    case GColorBlack:
        return s_black;
    case GColorWhite:
        return s_white;
    default:
        return s_clear;
    }
}

void graphics_context_set_stroke_color(GContext *ctx, GColor color) {
    ctx->stroke_color = color;
}

void graphics_context_set_fill_color(GContext *ctx, GColor color) {
    ctx->fill_color = color;
}

void graphics_context_set_text_color(GContext *ctx, GColor color) {
    ctx->text_color = color;
}

void graphics_context_set_compositing_mode(GContext *ctx, GCompOp mode) {
    ctx->compositing_mode = mode;
}

void graphics_draw_pixel(GContext *ctx, GPoint point) {
    GPoint topOffset=getTopOffset ();
    pixelColor(getTopScreen(), topOffset.x+point.x, topOffset.y+point.y, getRawColor(ctx->stroke_color));
}

void graphics_draw_line(GContext *ctx, GPoint p0, GPoint p1) {
    GPoint topOffset=getTopOffset ();
    lineColor(getTopScreen(), topOffset.x+p0.x, topOffset.y+p0.y, topOffset.x+p1.x, topOffset.y+p1.y, getRawColor(ctx->stroke_color));
}

void graphics_draw_rect(GContext *ctx, GRect rect) {
    GPoint topOffset=getTopOffset ();
    rectangleColor (getTopScreen(),topOffset.x+rect.origin.x,topOffset.y+rect.origin.y,topOffset.x+rect.origin.x+rect.size.w,topOffset.y+rect.origin.y+rect.size.h,getRawColor(ctx->stroke_color));
}

void graphics_fill_rect(GContext *ctx, GRect rect, uint16_t corner_radius, GCornerMask corner_mask) {
    //TODO: corner_mask
    GPoint topOffset=getTopOffset ();
    if (corner_radius>0)
        roundedBoxColor (getTopScreen(),topOffset.x+rect.origin.x,topOffset.y+rect.origin.y,topOffset.x+rect.origin.x+rect.size.w,topOffset.y+rect.origin.y+rect.size.h,corner_radius,getRawColor(ctx->fill_color));
    else
        boxColor (getTopScreen(),topOffset.x+rect.origin.x,topOffset.y+rect.origin.y,topOffset.x+rect.origin.x+rect.size.w,topOffset.y+rect.origin.y+rect.size.h,getRawColor(ctx->fill_color));
}

void graphics_draw_circle(GContext *ctx, GPoint p, uint16_t radius) {
    GPoint topOffset=getTopOffset ();
    circleColor(getTopScreen(), topOffset.x+p.x, topOffset.y+p.y, radius, getRawColor(ctx->stroke_color));
}

void graphics_fill_circle(GContext *ctx, GPoint p, uint16_t radius) {
    GPoint topOffset=getTopOffset ();
    filledCircleColor(getTopScreen(), topOffset.x+p.x, topOffset.y+p.y, radius, getRawColor(ctx->fill_color));
}

void graphics_draw_round_rect(GContext *ctx, GRect rect, uint16_t radius) {
    GPoint topOffset=getTopOffset ();
    roundedRectangleColor(getTopScreen(),topOffset.x+rect.origin.x,topOffset.y+rect.origin.y,topOffset.x+rect.origin.x+rect.size.w,topOffset.y+rect.origin.y+rect.size.h,radius,getRawColor(ctx->stroke_color));
}

void _gpath_to_sdl(GPath *path,Sint16** pointsX,Sint16** pointsY) {
    GPoint topOffset=getTopOffset ();
    *pointsX=(Sint16*)malloc(sizeof(Sint16)*path->num_points);
    *pointsY=(Sint16*)malloc(sizeof(Sint16)*path->num_points);

    double s = sin(((double)path->rotation/TRIG_MAX_ANGLE) * 2*PI );
    double c = cos(((double)path->rotation/TRIG_MAX_ANGLE) * 2*PI);
    int p;
    for(p=0; p<path->num_points; p++) {
        double x, y, nx;
        x = path->points[p].x+0.5;
        y = path->points[p].y+0.5;
        nx = x * c - y * s;
        y = x * s + y * c;
        x = nx;
        x += path->offset.x;
        y += path->offset.y;

        (*pointsX)[p] = (Sint16)(x+topOffset.x);
        (*pointsY)[p] = (Sint16)(y+topOffset.y);
    }
}

void gpath_draw_outline(GContext *ctx, GPath *path) {
    Sint16* pointsX,* pointsY;
    _gpath_to_sdl (path,&pointsX,&pointsY);
    polygonColor(getTopScreen(), pointsX, pointsY, path->num_points, getRawColor(ctx->stroke_color));
    free(pointsX);
    free(pointsY);
}

void gpath_draw_filled(GContext *ctx, GPath *path) {
    Sint16* pointsX,* pointsY;
    _gpath_to_sdl (path,&pointsX,&pointsY);
    filledPolygonColor(getTopScreen(), pointsX, pointsY, path->num_points, getRawColor(ctx->fill_color));
    free(pointsX);
    free(pointsY);
}

//This is because the graphics_draw_* functions are too big to not be controlled by something bigger
//-1,0 means Start
//0,-1 means End
//x,x means getPixel at x,y
typedef uint32_t (*graphics_draw_something_callback) (void* what,int16_t x,int16_t y);

void graphics_draw_something_in_rect_to(GContext *ctx, void* what,graphics_draw_something_callback callback, GRect rect,SDL_Surface* screen) {
    // TODO: verify composite mode implementations
    // TODO: bitmap->info_flags?
    GCompOp compositing_mode = ctx->compositing_mode;
    GPoint topOffset=getTopOffset ();
    SDL_Rect clipRect;
    SDL_GetClipRect(screen,&clipRect);
    LOCK(screen);
    int16_t x,y;
    callback (what,-1,0);
    for(x=0; x < rect.size.w; x++) {
        int16_t dst_x = x + rect.origin.x+topOffset.x;
        if(dst_x < clipRect.x) continue;
        else if(dst_x >= clipRect.x+clipRect.w) break;


        for(y=0; y < rect.size.h; y++) {
            int16_t dst_y = y + rect.origin.y+topOffset.y;
            if(dst_y < clipRect.y) continue;
            else if(dst_y >= clipRect.y+clipRect.h) break;

            uint32_t src_c = callback(what,x,y);
            uint32_t *dst_c = (uint32_t *)(((uint8_t*)screen->pixels) + screen->pitch * dst_y + 4 * dst_x);

            switch(compositing_mode) {
            case GCompOpAssign:
                *dst_c = src_c;
                break;
            case GCompOpAssignInverted:
                *dst_c = ~(src_c) | 0x000000ff;
                break;
            case GCompOpOr:
                if (src_c == r_white)
                    *dst_c = r_white;
                break;
            case GCompOpAnd:
                if (src_c == r_black)
                    *dst_c = r_black;
                break;
            case GCompOpClear:
                if (src_c == r_white)
                    *dst_c = r_black;
                break;
            case GCompOpSet:
                if (src_c == r_black)
                    *dst_c = r_white;
                break;
            }
        }
    }
    callback(what,0,-1);
    UNLOCK(screen);
}

uint32_t graphics_draw_bitmap_callback (void* what,int16_t x,int16_t y) {
    if (x>=0&&y>=0) {
        GBitmap* bitmap=(GBitmap*)what;
        uint32_t src_c=(bitmap->bounds.origin.x+(x%bitmap->bounds.size.w));
        src_c = (*(((uint8_t*)bitmap->addr)+bitmap->row_size_bytes*(bitmap->bounds.origin.y+(y%bitmap->bounds.size.h))+src_c/8))&(1<<(src_c%8)); //(bitmap_base + line_start + byte_indedx))
        src_c = (src_c>0?r_white:r_black);
        return src_c;
    }
    return 0;
}

void graphics_draw_bitmap_in_rect (GContext* ctx,const GBitmap* bitmap,GRect rect) {
    graphics_draw_something_in_rect_to (ctx,(void*)bitmap,graphics_draw_bitmap_callback,rect,getTopScreen ());
}

void graphics_draw_bitmap_in_rect_to (GContext* ctx,const GBitmap* bitmap,GRect rect,SDL_Surface* to) {
    graphics_draw_something_in_rect_to (ctx,(void*)bitmap,graphics_draw_bitmap_callback,rect,to);
}

uint32_t graphics_draw_surface_callback (void* what,int16_t x,int16_t y) {
    SDL_Surface* sur=(SDL_Surface*) what;
    if (x<0) {
        LOCK(sur);
    }
    else if (y<0) {
        UNLOCK(sur);
    }
    else {
        uint8_t* address=((uint8_t*)sur->pixels)+(y%sur->h)*sur->pitch+(x%sur->w)*4;
        uint32_t src_c=*((uint32_t*)address);
        return src_c;
    }
    return 0;
}

void graphics_draw_surface_in_rect (GContext* ctx,SDL_Surface* sur,GRect rect) {
    graphics_draw_something_in_rect_to (ctx,sur,graphics_draw_surface_callback,rect,getTopScreen());
}

void graphics_draw_surface_in_rect_to (GContext* ctx,SDL_Surface* sur,GRect rect,SDL_Surface* to) {
    graphics_draw_something_in_rect_to (ctx,sur,graphics_draw_surface_callback,rect,to);
}

typedef struct _WrapResult {
    int lineLen;
    const char* newString;
    int addPoints;
} _WrapResult;

typedef _WrapResult (*TextWrapper)(const char* line,const int maxWidth,const GFont font);
_WrapResult wrap_words (const char* line,const int maxWidth,const GFont font);
_WrapResult wrap_points (const char* line,const int maxWidth,const GFont font);

GSize _graphics_draw_text(GContext *ctx, const char *text, const GFont font, const GRect box, const GTextOverflowMode overflow_mode, const GTextAlignment alignment, const GTextLayoutCacheRef layout, int getSizeOnly) {
    GSize s = { 0, 0 };
    char *buffer = strdup(text); //SHIT!! I need to mark the end of the string but I can't use the original parameter...
    TextWrapper textWrapper=(overflow_mode==GTextOverflowModeWordWrap?wrap_words:wrap_points);
    int lineHeight=0,usedHeight=5;
    SDL_Surface* lineSurface,*lineSurfaceTemp;
    SDL_Surface* textSurface=SDL_CreateRGBSurface (SDL_SWSURFACE|SDL_SRCALPHA,box.size.w,box.size.h,32,0xff000000,0x00ff0000,0x0000ff00,0x000000ff);
    SDL_FillRect (textSurface,0,0);
    SDL_Surface* pointsSurface=0; //this will only be initalised when it's needed
    SDL_Rect dstRect,srcRect;
    SDL_Color color=  getColor(getSizeOnly ? 0 : ctx->text_color);
    GPoint topOffset=getTopOffset ();
    _WrapResult wrap;
    if (text==0)
        return s;
    while (*text!=0&&usedHeight<box.size.h) {
        wrap=textWrapper (text,box.size.w,font);
        memcpy(buffer,text,wrap.lineLen);
        buffer[wrap.lineLen]=0;
        text=wrap.newString;
        if (wrap.lineLen==0) {
            usedHeight+=lineHeight;
            continue;
        }
        lineSurfaceTemp=TTF_RenderUTF8_Solid ((TTF_Font*)font,buffer,color);
        if (lineSurfaceTemp==0) {
            printf("[WARN] TTF_RenderUTF8_Solid: %s\n",TTF_GetError ());
            free(buffer);
            return s;
        }
        lineSurface=SDL_ConvertSurface(lineSurfaceTemp,textSurface->format,SDL_SWSURFACE|SDL_SRCALPHA);
        SDL_FreeSurface(lineSurfaceTemp);
        if (lineHeight==0)
            lineHeight=lineHeightFromFont(font);
        //prepare blitting
        srcRect=((SDL_Rect) {
            0,0,lineSurface->w,lineSurface->h
        });
        if (srcRect.h+usedHeight>box.size.h)
            srcRect.h=box.size.h-usedHeight;
        dstRect=srcRect;
        dstRect.y=usedHeight;
        //set text alignment
        if (alignment==GTextAlignmentCenter)
            dstRect.x=box.size.w/2-(dstRect.w+wrap.addPoints)/2;
        else if (alignment==GTextAlignmentRight)
            dstRect.x=box.size.w-(dstRect.w+wrap.addPoints);
        //else
        //  srcRect.x=0;
        //blit line to text
        if (!getSizeOnly)
            SDL_gfxBlitRGBA(lineSurface,&srcRect,textSurface,&dstRect);
        SDL_FreeSurface(lineSurface);
        if (wrap.addPoints>0) {
            if (pointsSurface==0) {
                pointsSurface=TTF_RenderUTF8_Solid ((TTF_Font*)font,"...",color);
                if (pointsSurface==0) {
                    printf("[WARN] TTF_RenderUTF8_Solid: %s\n",TTF_GetError ());
                    free(buffer);
                    return s;
                }
            }
            srcRect.x=0;
            srcRect.w=pointsSurface->w;
            dstRect.x+=dstRect.w;
            dstRect.w=pointsSurface->w;
            if (!getSizeOnly)
                SDL_BlitSurface(pointsSurface,&srcRect,textSurface,&dstRect);
        }
        usedHeight+=lineHeight;
    }
    srcRect=((SDL_Rect) {
        0,0,box.size.w,box.size.h
    });
    dstRect=((SDL_Rect) {
        box.origin.x+topOffset.x,box.origin.y+topOffset.y,box.size.w,box.size.h
    });
    if (!getSizeOnly)
        SDL_gfxBlitRGBA(textSurface,&srcRect,getTopScreen(),&dstRect);
    SDL_FreeSurface(textSurface);
    if (pointsSurface!=0)
        SDL_FreeSurface(pointsSurface);
    free(buffer);
    s.w = box.size.w;
    s.h = usedHeight;
    return s;
}

void graphics_draw_text(GContext *ctx, const char *text, const GFont font, const GRect box, const GTextOverflowMode overflow_mode, const GTextAlignment alignment, const GTextLayoutCacheRef layout) {
    _graphics_draw_text(ctx, text, font, box, overflow_mode, alignment, layout, 0);
}

GSize _wrap_getStringSize (const char* start,const char* end,const GFont font) {
    int sx,ex,sy;
    TTF_SizeText (font,start,&sx,&sy);
    TTF_SizeText (font,end,&ex,0);
    return GSize(sx-ex,sy);
}

int _wrap_getStringWidth (const char* start,const char* end,const GFont font) { //this function is needed as strings without a 0-end-mark have to be measured
    return _wrap_getStringSize(start,end,font).w;
}

GSize graphics_text_layout_get_max_used_size(GContext *ctx, const char *text, const GFont font, const GRect box, const GTextOverflowMode overflow_mode, const GTextAlignment alignment, GTextLayoutCacheRef layout) {
    TextWrapper textWrapper=(overflow_mode==GTextOverflowModeWordWrap?wrap_words:wrap_points);
    GSize usedSize={0,0};
    GSize lineSize;
    _WrapResult wrap;
    if (text==0)
        return usedSize;
    while (*text!=0&&usedSize.h<box.size.h) {
        wrap=textWrapper (text,box.size.w,font);
        lineSize=_wrap_getStringSize(text,wrap.newString,font);
        usedSize.w=max(usedSize.w,lineSize.w);
        usedSize.h+=lineSize.h;
        text=wrap.newString;
    }
    usedSize.w=min(box.size.w,usedSize.w);
    usedSize.h=min(box.size.h,usedSize.h);
    return usedSize;
}

int _wrap_maxLineLength (const char* string,const int maxWidth,const GFont font) {
    int usedWidth=0;
    int len=0;
    const char* stringPtr=string;
    while (*stringPtr!='\n'&&*stringPtr!=0) {
        usedWidth=_wrap_getStringWidth(string,stringPtr+1,font);
        if (usedWidth>=maxWidth)
            return len ? len: 1;
        stringPtr++;
        len++;
    }
    if (*stringPtr=='\n')
        len++;
    return len;
}

_WrapResult wrap_words (const char* string,const int maxWidth,const GFont font) {
    _WrapResult result;
    int frontWidth,backWidth,wordStart;
    const char* backString,* backStringPtr;
    result.lineLen=_wrap_maxLineLength (string,maxWidth,font);
    result.addPoints=0;
    if (isalpha(string[result.lineLen])!=0&&isalpha(string[result.lineLen-1])!=0) {
        //get wordStart
        for (wordStart=result.lineLen-1; wordStart>=0; wordStart--) {
            if (isalpha(string[wordStart])==0)
                break;
        }
        if (wordStart>=0) {
            wordStart++;
            //get frontWidth
            frontWidth=_wrap_getStringWidth(string,string+wordStart,font);
            //get backWidth
            backString=string+result.lineLen;
            backStringPtr=backString;
            backWidth=0;
            while (isalpha(*backStringPtr)) {
                backWidth=_wrap_getStringWidth(backString,backStringPtr+1,font);
                if (backWidth>frontWidth)
                    break;
                backStringPtr++;
            }
            if (backWidth<=frontWidth) //I know this looks a bit wierd but a good optimizer can help
                result.lineLen=wordStart; //the word is short enough to be in another line.
        }
    }
    result.newString=string+result.lineLen;
    if (string[result.lineLen-1]=='\n')
        result.lineLen--; //SDL_ttf wants to render the '\n'
    return result;
}

_WrapResult wrap_points (const char* string,const int maxWidth,const GFont font) {
    _WrapResult result;
    const char* pointsStr="...";
    int pointsWidth;
    result.lineLen=_wrap_maxLineLength (string,maxWidth,font);
    if (string[result.lineLen]==0||string[result.lineLen-1]=='\n') {
        result.newString=string+result.lineLen;
        result.addPoints=0;
        if (string[result.lineLen-1]=='\n')
            result.lineLen--;
        return result;
    } else {
        //Remove characters until the three points fit
        pointsWidth=_wrap_getStringWidth(pointsStr,pointsStr+3,font);
        do {
            result.lineLen--;
        } while (pointsWidth+_wrap_getStringWidth(string,string+result.lineLen,font)>maxWidth);
        //Prepare the result
        result.addPoints=pointsWidth;
        result.newString=string+result.lineLen;
        while (*result.newString!=0&&*result.newString!='\n') //set the new string to the end of the line/string
            result.newString++;
        return result;
    }
}
