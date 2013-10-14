#include "globals.h"

static TimerEvent *timers = NULL;

static PebbleAppHandlers* appHandlers=0;

void add_timer(TimerEvent *timer) {

	TimerEvent *tp = timers;

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
	
	return;
}


bool remove_timer(TimerEvent *timer) {
	
	TimerEvent *tp = timers;
	
	
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


TimerEvent* search_timer_with_handle(uint32_t handle) {
	
	TimerEvent *tp = timers;
	
	while ( (tp != NULL) && (tp->handle != handle)) {
		tp = tp->next;
		if (tp == NULL) {
			printf("ERROR: Timer handle not found!");
			return NULL;
		}
	}
	
	return tp;
}


TimerEvent* find_expired_timer() {
	TimerEvent* tp = timers;

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


void fire_timers() {
	
	TimerEvent *timer = NULL;
	
	if (appHandlers==0)
        appHandlers=getAppHandlers();
	if (appHandlers->timer_handler==NULL)
		return;    
	
	//find all rexpired timers, call timer handler on them, and remove them
	while ( (timer = find_expired_timer()) != NULL ) {
		(appHandlers->timer_handler)(NULL, timer->handle, timer->cookie);
	    
		remove_timer(timer);
	}
	
	return;
}


void print_timers() {
	
	TimerEvent *tp = timers;
	uint32_t current_time = SDL_GetTicks();
	
	
	printf("Current time: %u\n",current_time);
	
	
	//find the first expired timer in the list
	while (tp != NULL) {
		printf("Timer: %d, %u\n", tp->handle, tp->timeout);
		
		tp = tp->next;
	}
	
	
}


void create_new_timer(uint32_t timeout_ms, uint32_t cookie, uint32_t handle) {

	TimerEvent *timer = malloc(sizeof(TimerEvent));
	timer->timeout=SDL_GetTicks()+timeout_ms;
	timer->cookie = cookie;
	timer->handle = handle;
	timer->next = NULL;

	add_timer(timer);

//	printf("New Timer: %u, %u, %u, %u\n", handle, timeout_ms, cookie, timer->timeout);

//	print_timers();
	
}
