#include "globals.h"

struct AppTimer
{
	uint32_t timeout;
	AppTimerCallback callback;
	void* context;
	struct AppTimer *next;
};

static AppTimer *timers = NULL;

void add_timer(AppTimer *timer) {
	AppTimer *tp = timers;

	//add first timer to the list
	if (timers == NULL) {
		timers = timer;
		return;
	}

	//add the timer to the end of the list
	while (tp->next != NULL) {
		tp = tp->next;
	}

	tp->next = timer;
	timer->next = 0;
}

bool remove_timer(AppTimer*timer) {
	AppTimer *tp = timers;

	if (timer == NULL)
		return false;

	// if first element, set head of list to next pointer
	if (tp == timer) {
		timers = tp->next;
		free(timer);
		return true;
	}

	// find the timer that's supposed to be deleted from the list
	while ( (tp != NULL) && (tp->next != timer)) {
		if (tp->next == NULL) {
			printf("ERROR: Timer could not be removed. Timer handle not existent!");
			return false;
		}
		tp = tp->next;
	}

	//remove timer from list if it exists
	if (tp == NULL) {
		printf("ERROR: Timer could not be removed. Timer handle not existent!");
		return false;
	} else {
		tp->next = tp->next->next;
		free(timer);
	}

	return true;
}

AppTimer* find_expired_timer() {
	AppTimer* tp = timers;

	//find the first expired timer in the list
	while (tp != NULL) {
		//check if timer has expired
		if (SDL_GetTicks() >= tp->timeout) {
			return tp;
		}

		tp = tp->next;
	}

	return NULL;
}

bool is_timer_registered (AppTimer* timer) {
    AppTimer* tp=timers;
    while (tp!=0&&tp!=timer)
        tp=tp->next;
    return tp==timer;
}

void service_timers () {
	AppTimer *timer = NULL;
	//find all rexpired timers, call timer handler on them, and remove them
	while ( (timer = find_expired_timer()) != NULL ) {
		timer->callback(timer->context);
		remove_timer(timer);
	}
}

AppTimer* app_timer_register (uint32_t timeout_ms,AppTimerCallback callback,void* callback_data) {
    if (callback==0)
        return 0;
    AppTimer* timer=(AppTimer*)malloc(sizeof(AppTimer));
    if (timer==0) {
        printf ("[ERROR] Timer memory allocation failed!\n");
        return 0;
    }
    timer->timeout=SDL_GetTicks()+timeout_ms;
    timer->callback=callback;
    timer->context=callback_data;
    timer->next = NULL;
    add_timer(timer);
    return timer;
}

bool app_timer_reschedule (AppTimer* timer_handle,uint32_t new_timeout_ms) {
    if (is_timer_registered(timer_handle)) {
        timer_handle->timeout=SDL_GetTicks()+new_timeout_ms;
        return true;
    }
    else
        return false;
}

void app_timer_cancel(AppTimer* timer_handle) {
    remove_timer(timer_handle);
}
