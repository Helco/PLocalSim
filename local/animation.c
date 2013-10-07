#include "globals.h"

static Animation* firstScheduledAnimation=0;

void animation_init(struct Animation *animation) {
    animation->list_node.next=0;
    animation->list_node.prev=0;
    animation->abs_start_time_ms=0;
    animation->is_completed=true;
    //SDK defaults
    animation->duration_ms=250;
    animation->curve= AnimationCurveEaseInOut;
    animation->delay_ms=0;
    animation->handlers.started=0;
    animation->handlers.stopped=0;
    animation->context=0;
    animation->implementation=0;
}

void animation_set_delay(struct Animation *animation, uint32_t delay_ms) {
    animation->delay_ms=delay_ms;
}

void animation_set_duration(struct Animation *animation, uint32_t duration_ms) {
    animation->duration_ms=duration_ms;
}

void animation_set_curve(struct Animation *animation, AnimationCurve curve) {
    animation->curve=curve;
}

void animation_set_handlers(struct Animation *animation, AnimationHandlers callbacks, void *context) {
    animation->handlers=callbacks;
    animation->context=context;
}

void animation_set_implementation(struct Animation *animation, const AnimationImplementation *implementation) {
    animation->implementation=implementation;
}

void *animation_get_context(struct Animation *animation) {
    return animation->context;
}

void animation_schedule(struct Animation *animation) {
    if (animation->implementation==0)
        return;
    if (animation_is_scheduled(animation))
        animation_unschedule(animation);
    animation->is_completed=true; //Yes this is meant to be true! That is how the engine determines if a animation is in "delay" state
    animation->abs_start_time_ms=SDL_GetTicks();
    animation->list_node.next=(ListNode*)firstScheduledAnimation;
    animation->list_node.prev=0;
    if (firstScheduledAnimation!=0)
        firstScheduledAnimation->list_node.prev=(ListNode*)animation;
    firstScheduledAnimation=animation;
    if (animation->implementation->setup!=0)
        animation->implementation->setup (animation);
}

void animation_unschedule(struct Animation *animation) {
    if (animation->implementation==0)
        return;
    if (animation->handlers.stopped!=0)
        animation->handlers.stopped(animation,animation->is_completed,animation->context);
    if (animation->implementation->teardown!=0)
        animation->implementation->teardown(animation);
    animation->is_completed=true;
    animation->abs_start_time_ms=0;
    if (animation==firstScheduledAnimation) {
        firstScheduledAnimation=(Animation*)animation->list_node.next;
        if (firstScheduledAnimation!=0)
            firstScheduledAnimation->list_node.prev=0;
    }
    else {
        animation->list_node.prev->next=animation->list_node.next;
        if (animation->list_node.next!=0)
            animation->list_node.next->prev=animation->list_node.prev;
    }
    animation->list_node.next=0;
    animation->list_node.prev=0;
}

void animation_unschedule_all(void) {
    while (firstScheduledAnimation!=0)
        animation_unschedule(firstScheduledAnimation);
}

bool animation_is_scheduled(struct Animation *animation) {
    Animation* cursor=firstScheduledAnimation;
    while (cursor!=0) {
        if (cursor==animation)
            return true;
        cursor=(Animation*)cursor->list_node.next;
    }
    return false;
}

void updateAnimations () {
    Animation* cursor=firstScheduledAnimation,* tempCursor;
    bool isDirty=false;
    uint32_t time=0;
    while (cursor!=0) {
        if (cursor->is_completed) { //"delay" state
            if (SDL_GetTicks()-cursor->abs_start_time_ms>=cursor->delay_ms) {
                cursor->abs_start_time_ms=SDL_GetTicks();
                cursor->is_completed=false;
                if (cursor->handlers.started!=0)
                    cursor->handlers.started(cursor,cursor->context);
            }
        }
        else {
            if (cursor->duration_ms==ANIMATION_DURATION_INFINITE)
                time=ANIMATION_DURATION_INFINITE;
            else if (SDL_GetTicks()-cursor->abs_start_time_ms>=cursor->duration_ms) {
                isDirty=true;
                cursor->is_completed=true;
                tempCursor=cursor;
                cursor=(Animation*)cursor->list_node.next;
                animation_unschedule(tempCursor);
                continue;
            }
            else
                time=(ANIMATION_NORMALIZED_MIN + (((ANIMATION_NORMALIZED_MAX - ANIMATION_NORMALIZED_MIN) * (SDL_GetTicks()-cursor->abs_start_time_ms)) / cursor->duration_ms));
            cursor->implementation->update (cursor,time);
            isDirty=true;
        }
        cursor=(Animation*)cursor->list_node.next;
    }
    if (isDirty==true)
        markDirty (true);
}

void property_animation_init (PropertyAnimation* animation,const PropertyAnimationImplementation* implementation,void* subject,void* from_value,void* to_value)
{
    animation_init(&animation->animation);
    animation->animation.implementation=(AnimationImplementation*)implementation;
    animation->subject=subject;
    if (implementation->base.update==(AnimationUpdateImplementation)property_animation_update_int16) {
        if (from_value==0)
            animation->values.from.int16=implementation->accessors.getter.int16 (subject);
        else
            animation->values.from.int16=*(int16_t*)from_value;
        if (to_value==0)
            animation->values.to.int16=implementation->accessors.getter.int16 (subject);
        else
            animation->values.to.int16=*(int16_t*)to_value;
    }
    else if (implementation->base.update==(AnimationUpdateImplementation)property_animation_update_gpoint) {
        if (from_value==0)
            animation->values.from.gpoint=implementation->accessors.getter.gpoint (subject);
        else
            animation->values.from.gpoint=*(GPoint*)from_value;
        if (to_value==0)
            animation->values.to.gpoint=implementation->accessors.getter.gpoint (subject);
        else
            animation->values.to.gpoint=*(GPoint*)to_value;
    }
    else if (implementation->base.update==(AnimationUpdateImplementation)property_animation_update_grect) {
        if (from_value==0)
            animation->values.from.grect=implementation->accessors.getter.grect (subject);
        else
            animation->values.from.grect=*(GRect*)from_value;
        if (to_value==0)
            animation->values.to.grect=implementation->accessors.getter.grect (subject);
        else
            animation->values.to.grect=*(GRect*)to_value;
    }
    else
        printf ("[WARN] Invalid property animation implementation!\n");
}

#define PROPERTY_ANIMATION_INT16_VALUE(from,to,time) from+(int16_t)((to-from)*((float)time/ANIMATION_NORMALIZED_MAX));

void property_animation_update_int16 (PropertyAnimation* animation,const uint32_t time)
{
    int16_t value=PROPERTY_ANIMATION_INT16_VALUE(animation->values.from.int16,animation->values.to.int16,time);
    PropertyAnimationImplementation* implementation=(PropertyAnimationImplementation*)animation->animation.implementation;
    implementation->accessors.setter.int16(animation->subject,value);
}

void property_animation_update_gpoint (PropertyAnimation* animation,const uint32_t time)
{
    GPoint value;
    PropertyAnimationImplementation* implementation=(PropertyAnimationImplementation*)animation->animation.implementation;
    value.x=PROPERTY_ANIMATION_INT16_VALUE(animation->values.from.gpoint.x,animation->values.to.gpoint.x,time);
    value.y=PROPERTY_ANIMATION_INT16_VALUE(animation->values.from.gpoint.y,animation->values.to.gpoint.y,time);
    implementation->accessors.setter.gpoint(animation->subject,value);
}

void property_animation_update_grect (PropertyAnimation* animation,const uint32_t time)
{
    GRect value;
    PropertyAnimationImplementation* implementation=(PropertyAnimationImplementation*)animation->animation.implementation;
    value.origin.x=PROPERTY_ANIMATION_INT16_VALUE(animation->values.from.grect.origin.x,animation->values.to.grect.origin.x,time);
    value.origin.y=PROPERTY_ANIMATION_INT16_VALUE(animation->values.from.grect.origin.y,animation->values.to.grect.origin.y,time);
    value.size.w=PROPERTY_ANIMATION_INT16_VALUE(animation->values.from.grect.size.w,animation->values.to.grect.size.w,time);
    value.size.h=PROPERTY_ANIMATION_INT16_VALUE(animation->values.from.grect.size.h,animation->values.to.grect.size.h,time);
    implementation->accessors.setter.grect(animation->subject,value);
}

GRectReturn layerFrameGetter (void* subject)
{
    return ((Layer*)subject)->frame;
}

void layerFrameSetter (void* subject,GRect rect)
{
    ((Layer*)subject)->frame=rect;
    layer_mark_dirty((Layer*)subject);
}

static PropertyAnimationImplementation layer_property_animation_implementation={
    .base={
        .update=(AnimationUpdateImplementation)property_animation_update_grect
    },
    .accessors={
        .getter={ .grect=layerFrameGetter},
        .setter={ .grect=layerFrameSetter}
    }
};

void property_animation_init_layer_frame (PropertyAnimation* animation,Layer* layer,GRect* from,GRect* to)
{
    property_animation_init (animation,&layer_property_animation_implementation,layer,from,to);
}
