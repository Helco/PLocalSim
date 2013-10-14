#include "globals.h"
#include <time.h>
#include <sys/time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif



static TimerEvent *timers = NULL;

static PebbleAppHandlers* appHandlers=0;


struct timespec get_current_time() {
	struct timespec ts;

	#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	ts.tv_sec = mts.tv_sec;
	ts.tv_nsec = mts.tv_nsec;

	#else
	clock_gettime(CLOCK_REALTIME, &ts);
	#endif


	return ts;
}



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
	
	TimerEvent *tp = timers;
	struct timespec current_time = get_current_time();
	
	long long tci = current_time.tv_sec * 1000000000 + current_time.tv_nsec;	//current time
	long long tpi;																//timer expired time
	
	//find the first expired timer in the list
	while (tp != NULL) {
		tpi = tp->elapsed_time.tv_sec * 1000000000 + tp->elapsed_time.tv_nsec;

		//check if timer has expired
		if ( tpi <= tci ) {
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
    
	
	//find all rexpired timers, call timer handler on them, and remove them
	while ( (timer = find_expired_timer()) != NULL ) {
		if (appHandlers->timer_handler!=NULL) 
	        (appHandlers->timer_handler)(NULL, timer->handle, timer->cookie);
	    
		remove_timer(timer);
	}
	
	return;
}


void print_timers() {
	
	TimerEvent *tp = timers;
	struct timespec current_time = get_current_time();
	
	
	printf("Current time: %lld.%.9ld\n", (long long)current_time.tv_sec, current_time.tv_nsec);
	
	
	//find the first expired timer in the list
	while (tp != NULL) {
		printf("Timer: %d, %lld.%.9ld\n", tp->handle, (long long)tp->elapsed_time.tv_sec, tp->elapsed_time.tv_nsec);
		
		tp = tp->next;
	}
	
	
}


void create_new_timer(uint32_t timeout_ms, uint32_t cookie, uint32_t handle) {
	struct timespec ts, te;
	long long tsi, tei;
	
	//current time
	ts = get_current_time();
	tsi = 1000000000 * ts.tv_sec + ts.tv_nsec;
	
	//determine expiration time for timer
	tei = tsi + 1000000 * timeout_ms;
		
	te.tv_sec = tei/(long long)1000000000;
	te.tv_nsec = tei - (tei/(long long)1000000000)*1000000000;
	
	TimerEvent *timer = malloc(sizeof(TimerEvent));
	timer->elapsed_time = te;
	timer->cookie = cookie;
	timer->handle = handle;
	timer->next = NULL;

	add_timer(timer);

//	printf("New Timer: %d, %d, %d, %lld.%.9ld, %lld.%.9ld\n", handle, timeout_ms, cookie, (long long)ts.tv_sec, ts.tv_nsec, (long long)te.tv_sec, te.tv_nsec);

//	print_timers();
	
}