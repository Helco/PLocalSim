#include "globals.h"

int32_t atan2_lookup (int16_t y, int16_t x) {
    return (int32_t)(atan2((double)y, (double)x) / (2*PI) * TRIG_MAX_ANGLE);
}

int32_t sin_lookup(int32_t angle) {
    return (int32_t)(sin((double)angle/TRIG_MAX_RATIO* 2*PI)*TRIG_MAX_RATIO);
}

int32_t cos_lookup(int32_t angle) {
    return (int32_t)(cos((double)angle/TRIG_MAX_RATIO* 2*PI)*TRIG_MAX_RATIO);
}

GPath* gpath_create(const GPathInfo *init) {
    GPath* path=(GPath*)malloc(sizeof(GPath));
    if (!path) {
        printf ("[ERROR] Memory allocation failed!\n");
        return 0;
    }
    path->points=(GPoint*)malloc(sizeof(GPoint)*init->num_points);
    if (!path->points) {
        printf ("[ERROR] Memory allocation failed!\n");
        free(path);
        return 0;
    }
    path->rotation=0;
    path->offset=GPointZero;
    path->num_points = init->num_points;
    memcpy(path->points,init->points,sizeof(GPoint)*init->num_points);
    return path;
}

void gpath_destroy (GPath* path) {
    if (path) {
        if (path->points)
            free(path->points);
        free(path);
    }
}

void gpath_move_to(GPath *path, GPoint point) {
    path->offset = point;
}

void gpath_rotate_to(GPath *path, int32_t angle) {
    path->rotation = angle%TRIG_MAX_ANGLE;
}

GPoint grect_center_point(const GRect *rect) {
    GPoint output;
    output.x = rect->origin.x + (rect->size.w)/2;
    output.y = rect->origin.y + (rect->size.h)/2;

    return output;
}

bool gpoint_equal(const GPoint * const point_a, const GPoint * const point_b) {
    return (point_a->x==point_b->x&&point_a->y==point_b->y);
}

bool gsize_equal(const GSize *size_a, const GSize *size_b) {
    return (size_a->w==size_b->w&&size_a->h==size_b->h);
}

bool grect_equal (const GRect* const a,const GRect* const b) {
    if (a==0||b==0)
        return false;
    return (a->origin.x==b->origin.x&&
            a->origin.y==b->origin.y&&
            a->size.w==b->size.w&&
            a->size.h==b->size.h);
}

bool grect_contains_point(const GRect *rect,const GPoint *point) {
    return (point->x>=rect->origin.x&&
            point->x<=rect->origin.x+rect->size.w&&
            point->y>=rect->origin.y&&
            point->y<=rect->origin.y+rect->size.h);
}

GRect grect_crop(GRect rect, const int crop_size_px) {
    rect.origin.x+=crop_size_px;
    rect.origin.y+=crop_size_px;
    rect.size.w-=2*crop_size_px;
    rect.size.h-=2*crop_size_px;
    return rect;
}

bool grect_is_empty(const GRect * const rect) {
    return (rect->size.w==0&&rect->size.h==0);
}

void grect_standardize(GRect *rect) {
    if (rect->size.w<0) {
        rect->origin.x+=rect->size.w;
        rect->size.w=-rect->size.w;
    }
    if (rect->size.h<0) {
        rect->origin.y-=rect->size.h;
        rect->size.h=-rect->size.h;
    }
}

void grect_clip(GRect * const rect_to_clip, const GRect * const rect_clipper) {
    int16_t x,y;
    if (rect_to_clip==0||rect_clipper==0)
        return;
    if (rect_to_clip->origin.x<rect_clipper->origin.x) {
        x=rect_clipper->origin.x-rect_to_clip->origin.x;
        rect_to_clip->origin.x=rect_clipper->origin.x;
        rect_to_clip->size.w-=x;
    }
    if (rect_to_clip->origin.y<rect_clipper->origin.y) {
        y=rect_clipper->origin.y-rect_to_clip->origin.y;
        rect_to_clip->origin.y=rect_clipper->origin.y;
        rect_to_clip->size.h-=y;
    }
    if (rect_to_clip->origin.x+rect_to_clip->size.w>rect_clipper->origin.x+rect_clipper->size.w)
        rect_to_clip->size.w=rect_clipper->size.w-rect_to_clip->origin.x;
    if (rect_to_clip->origin.y+rect_to_clip->size.h>rect_clipper->origin.y+rect_clipper->size.h)
        rect_to_clip->size.h=rect_clipper->size.h-rect_to_clip->origin.y;
}

void grect_align(GRect *rect, const GRect *inside_rect, const GAlign alignment, const bool clip) {
    if (alignment==GAlignLeft||alignment==GAlignTopLeft||alignment==GAlignBottomLeft) //overall X: left alignment
        rect->origin.x=inside_rect->origin.x;
    else if (alignment==GAlignRight||alignment==GAlignTopRight||alignment==GAlignBottomRight) //overall X: right alignment
        rect->origin.x=inside_rect->origin.x+inside_rect->size.w-rect->size.w;
    else if (alignment==GAlignCenter||alignment==GAlignTop||alignment==GAlignBottom) //overall X: center alignment
        rect->origin.x=inside_rect->origin.x+inside_rect->size.w/2-rect->size.w/2;
    if (alignment==GAlignTop||alignment==GAlignTopLeft||alignment==GAlignTopRight) //overall Y: top alignment
        rect->origin.y=inside_rect->origin.y;
    else if (alignment==GAlignBottom||alignment==GAlignBottomLeft||alignment==GAlignTopRight) //overall Y: bottom alignment
        rect->origin.y=inside_rect->origin.y+inside_rect->size.h-rect->size.h;
    else if (alignment==GAlignCenter||alignment==GAlignLeft||alignment==GAlignRight) //overall Y: center alignment
        rect->origin.y=inside_rect->origin.y+inside_rect->size.h/2-rect->size.h/2;
    if (clip)
        grect_clip (rect,inside_rect);
}

void sdlrect_clip(SDL_Rect * const rect_to_clip, const SDL_Rect * const rect_clipper) {
    int16_t x,y;
    if (rect_to_clip==0||rect_clipper==0)
        return;
    if (rect_to_clip->x<rect_clipper->x) {
        x=rect_clipper->x-rect_to_clip->x;
        rect_to_clip->x=rect_clipper->x;
        rect_to_clip->w-=x;
    }
    if (rect_to_clip->y<rect_clipper->y) {
        y=rect_clipper->y-rect_to_clip->y;
        rect_to_clip->y=rect_clipper->y;
        rect_to_clip->h-=y;
    }
    if (rect_to_clip->x+rect_to_clip->w>rect_clipper->x+rect_clipper->w) {
        rect_to_clip->w=rect_clipper->w-rect_to_clip->x;
    }
    if (rect_to_clip->y+rect_to_clip->h>rect_clipper->y+rect_clipper->h) {
        rect_to_clip->h=rect_clipper->h-rect_to_clip->y;
    }
}

GPoint getPivotRotationOffset(GSize rectOrig,GSize rectRotated,GPoint pivot,double angle) {
    GPoint offset;
    pivot.x-=rectOrig.w/2;
    pivot.y-=rectOrig.h/2;
    angle*=PI/180.0;
    double s=sin(angle);
    double c=cos(angle);
    offset.x=pivot.x*c - pivot.y*s + rectRotated.w/2;
    offset.y=pivot.x*s + pivot.y*c + rectRotated.h/2;
    return offset;
}

void gpoint_move_into_rect (GPoint* const point,const GRect* const rect) {
    point->x=clamp(rect->origin.x,point->x,rect->origin.x+rect->size.w);
    point->y=clamp(rect->origin.y,point->y,rect->origin.y+rect->size.h);
}
