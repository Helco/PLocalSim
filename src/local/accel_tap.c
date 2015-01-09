#include "globals.h"

//
// This file implements the accel_tap_service part of AccelerometerService
//
// Thrust can be simulated in X/Y/Z axis with +/- direction
//

static AccelTapHandler tap_handler = NULL;
static AccelAxisType tap_axis = ACCEL_AXIS_X;
static int32_t tap_direction = 0;
static bool tap_notify = false;

// simulator access

void accel_do_tap_on_axis(AccelAxisType axis, int32_t direction) {
	tap_axis = axis;
	if(direction >= 1) {
		tap_direction = 1;
	}
	else {
		if(direction <= -1) {
			tap_direction = -1;
		}
		else {
			tap_direction = 0;
		}
	}
	tap_notify = true;
}

// pebble API part

void accel_tap_service_subscribe(AccelTapHandler handler) {
	tap_handler = handler;
}

void accel_tap_service_unsubscribe(void) {
	tap_handler = NULL;
	tap_notify = false;
}

void service_accel_tap() {
	if(tap_notify && tap_handler != NULL) {
		tap_handler(tap_axis, tap_direction);
		tap_axis = ACCEL_AXIS_X;
		tap_direction = 0;
		tap_notify = false;
	}
}
