#include "globals.h"

static bool bluetooth_state = true;
static bool bluetooth_notify = false;
static BluetoothConnectionHandler bluetooth_callback = NULL;

void toggle_bluetooth_connection () {
	bluetooth_state = !bluetooth_state;
	bluetooth_notify = true;
}

bool bluetooth_connection_service_peek(void) {
	return bluetooth_state;
}

void bluetooth_connection_service_subscribe(BluetoothConnectionHandler handler) {
	bluetooth_callback = handler;
	bluetooth_notify = true;
}

void bluetooth_connection_service_unsubscribe(void) {
	bluetooth_callback = NULL;
}

void service_bluetooth() {
	if( bluetooth_notify && bluetooth_callback != NULL ) {
		bluetooth_callback(bluetooth_state);
		bluetooth_notify = false;
	}
}
