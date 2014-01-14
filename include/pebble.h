#pragma once

#include "pebble_fonts.h"
#include "src/resource_ids.auto.h"

#define PBL_APP_INFO(...) _Pragma("message \"\n\n \
  *** PBL_APP_INFO has been replaced with appinfo.json\n \
  Try updating your project with `pebble convert-project`\n \
  Visit our developer guides to learn more about appinfo.json:\n \
  http://developer.getpebble.com/2/guides/creating-pebble-watchapps.html\n \""); \
  _Pragma("GCC error \"PBL_APP_INFO has been replaced with appinfo.json\"");

#define PBL_APP_INFO_SIMPLE PBL_APP_INFO

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif

//! Status codes
typedef enum StatusCode {
  //! Operation completed successfully.
  S_SUCCESS = 0,

  //! An error occurred (no description).
  E_ERROR = -1,

  //! No idea what went wrong.
  E_UNKNOWN = -2,

  //! There was a generic internal logic error.
  E_INTERNAL = -3,

  //! The function was not called correctly.
  E_INVALID_ARGUMENT = -4,

  //! Insufficient allocatable memory available.
  E_OUT_OF_MEMORY = -5,

  //! Insufficient long-term storage available.
  E_OUT_OF_STORAGE = -6,

  //! Insufficient resources available.
  E_OUT_OF_RESOURCES = -7,

  //! Argument out of range (may be dynamic).
  E_RANGE = -8,

  //! Target of operation does not exist.
  E_DOES_NOT_EXIST = -9,

  //! Operation not allowed (may depend on state).
  E_INVALID_OPERATION = -10,

  //! Another operation prevented this one.
  E_BUSY = -11,


  //! Equivalent of boolean true.
  S_TRUE = 1,

  //! Equivalent of boolean false.
  S_FALSE = 0,

  //! For list-style requests.  At end of list.
  S_NO_MORE_ITEMS = 2,

  //! No action was taken as none was required.
  S_NO_ACTION_REQUIRED = 3,

} StatusCode;

typedef int32_t status_t;

#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

typedef struct ListNode {
  struct ListNode* next;
  struct ListNode* prev;
} ListNode;

#define IS_SIGNED(var) (((__typeof__(var)) - 1) < 0)

//! @addtogroup UI
//! @{

//! @addtogroup Clicks
//! \brief Handling button click interactions
//!
//! Click handlers work by consuming raw button up and button down events and distilling those
//! events into configurable user interactions. These interactions can be any one of the following:
//!
//! * Single-click. Detects a single click, that is, a button down event followed by button up event.
//! It also offers hold-to-repeat functionality.
//! * Multi-click. Detects double-clicking, triple-clicking and other arbitrary click counts.
//! It can fire its event handler on all of the matched clicks, or just the last.
//! * Long-click. Detects long clicks, that is, press-and-hold.
//! * Raw. Simply forwards the raw button events. It is provided as a way to use both the higher level
//! "clicks" processing and the raw button events at the same time.
//!
//! You set up click handlers in each application window in order to process button input and call the
//! necessary event handlers.
//!
//! A Pebble window handles various click inputs, like short and long clicks, hold-to-repeat clicks
//! and double clicks, by setting a Click Configuration Provider callback function.
//!
//! For convenience, a click callback provides a \ref ClickRecognizer and a user-specified context.
//! The \ref ClickRecognizer can be used to determine which button triggered the callback.
//!
//! Note that you could have different buttons that share the same callback.
//! However, you should use the \ref ClickRecognizer to differentiate actions based on the triggering button.
//!
//! You can also have multiple types of handlers on the same button: for example, click and long
//! click on the Select button.
//!
//! Refer to the \htmlinclude UiFramework.html (chapter "Clicks") for a conceptual
//! overview of clicks and relevant code examples. For SDK code examples, refer to
//!   * Examples/todolist-demo
//!
//! <h3>Usage example</h3>
//! For example, you first associate a click config provider callback with your window. Your callback
//! will provide your click configuration to Pebble OS.
//! \code{.c}
//! void app_init(void) {
//!   ...
//!   window_set_click_config_provider(&window, (ClickConfigProvider) config_provider);
//!   ...
//! }
//! \endcode
//! Then in the callback, you set your desired configuration for each button:
//! \code{.c}
//! void config_provider(Window *window) {
//!  // single click / repeat-on-hold config:
//!   window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
//!   window_single_repeating_click_subscribe(BUTTON_ID_SELECT, 1000, select_single_click_handler);
//!
//!   // multi click config:
//!   window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 10, 0, true, select_multi_click_handler);
//!
//!   // long click config:
//!   window_long_click_subscribe(BUTTON_ID_SELECT, 700, select_long_click_handler, select_long_click_release_handler);
//! }
//! \endcode
//! Now you implement the handlers for each click you've subscribed to and set up:
//! \code{.c}
//! void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
//!   ... called on single click ...
//!   Window *window = (Window *)context; // This context defaults to the window, but may be changed with \ref window_set_click_context.
//! }
//! void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
//!   ... called on single click, and every 1000ms of being held ...
//!   Window *window = (Window *)context; // This context defaults to the window, but may be changed with \ref window_set_click_context.
//! }
//!
//! void select_multi_click_handler(ClickRecognizerRef recognizer, void *context) {
//!   ... called for multi-clicks ...
//!   Window *window = (Window *)context; // This context defaults to the window, but may be changed with \ref window_set_click_context.
//!   const uint16_t count = click_number_of_clicks_counted(recognizer);
//! }
//!
//! void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
//!   ... called on long click start ...
//!   Window *window = (Window *)context; // This context defaults to the window, but may be changed with \ref window_set_click_context.
//! }
//!
//! void select_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {
//!   ... called when long click is released ...
//!   Window *window = (Window *)context; // This context defaults to the window, but may be changed with \ref window_set_click_context.
//! }
//! \endcode
//! See Examples/watchapps/pebble_arcade/src/entry.c for an example of click usage.
//! @note The Back button can't be re-configured. It is hard-wired to pop to the previous window on
//! the \ref WindowStack.
//!
//! @{

//! Button ID values
//! @see \ref click_recognizer_get_button_id()
typedef enum {
  //! Back button
  BUTTON_ID_BACK = 0,
  //! Up button
  BUTTON_ID_UP,
  //! Select (middle) button
  BUTTON_ID_SELECT,
  //! Down button
  BUTTON_ID_DOWN,
  //! Total number of buttons
  NUM_BUTTONS
} ButtonId;

//! @} // group Clicks

//! @} // group UI

//! @addtogroup Foundation
//! @{

//! @addtogroup Math
//! @{

//! The largest value that can result from a call to \ref sin_lookup or \ref cos_lookup.
//! For a code example, see the detailed description at the top of this chapter: \ref Math
#define TRIG_MAX_RATIO 0xffff

//! Angle value that corresponds to 360 degrees or 2 PI radians
//! @see \ref sin_lookup
//! @see \ref cos_lookup
#define TRIG_MAX_ANGLE 0x10000

//! Look-up the sine of the given angle from a pre-computed table.
//! @param angle The angle for which to compute the cosine.
//! A the angle value is scaled linearly, such that a value of 0x10000 corresponds to 360 degrees or 2 PI radians.
int32_t sin_lookup(int32_t angle);

//! Look-up the cosine of the given angle from a pre-computed table.
//! This is equivalent to calling `sin_lookup(angle + MAX_ANGLE / 4)`.
//! @param angle The angle for which to compute the cosine.
//! A the angle value is scaled linearly, such that a value of 0x10000 corresponds to 360 degrees or 2 PI radians.
int32_t cos_lookup(int32_t angle);

//! Look-up the arctangent of a given x, y pair
//! A the angle value is scaled linearly, such that a value of 0x10000 corresponds to 360 degrees or 2 PI radians.
int32_t atan2_lookup(int16_t y, int16_t x);

//! @} // group Math

//! @addtogroup WallTime Wall Time
//!   \brief Functions, data structures and other things related to wall clock time.
//!
//! This module contains utilities to get the current time and create strings with formatted
//! dates and times.
//! @note When implementing a watchface or application that tells time, make sure to check
//! out the `tick_info` field of \ref PebbleAppHandlers.
//! @{

//! Time unit flags that can be used to create a bitmask for use in \ref PebbleAppTickInfo.
//! @see PebbleAppTickInfo
//! @see PebbleAppHandlers
typedef enum {
  //! Flag to represent the "seconds" time unit
  SECOND_UNIT = 1 << 0,
  //! Flag to represent the "minutes" time unit
  MINUTE_UNIT = 1 << 1,
  //! Flag to represent the "hours" time unit
  HOUR_UNIT = 1 << 2,
  //! Flag to represent the "days" time unit
  DAY_UNIT = 1 << 3,
  //! Flag to represent the "months" time unit
  MONTH_UNIT = 1 << 4,
  //! Flag to represent the "years" time unit
  YEAR_UNIT = 1 << 5
} TimeUnits;

//! Copies a time string into the buffer, formatted according to the user's time display preferences (such as 12h/24h
//! time).
//! Example results: "7:30" or "15:00"
//! @param[out] buffer A pointer to the buffer to copy the time string into
//! @param size The maximum size of buffer
void clock_copy_time_string(char *buffer, uint8_t size);

//! Gets the user's 12/24h clock style preference.
//! @return `true` if the user prefers 24h-style time display or `false` if the
//! user prefers 12h-style time display.
bool clock_is_24h_style(void);

//! @} // group WallTime

//! @addtogroup EventService
//! @{

//! @addtogroup BluetoothConnectionService
//! \brief Determine when Pebble is connected to the phone
//!
//! The BluetoothConnectionService allows your app to know whether Pebble is connected to the phone.
//! You can ask the system for this information at a given time or you can register
//! to receive events every time Pebble connects or disconnects to the phone.
//! @{

//! Callback type for bluetooth connection events
//! @param connected true on bluetooth connection, false on disconnection
typedef void (*BluetoothConnectionHandler)(bool connected);

//! Query the bluetooth connection service for the current connection status
//! @return true if connected, false otherwise
bool bluetooth_connection_service_peek(void);

//! Subscribe to the bluetooth event service. Once subscribed, the handler gets called
//! on every bluetooth connection event.
//! @param handler A callback to be executed on connection event
void bluetooth_connection_service_subscribe(BluetoothConnectionHandler handler);

//! Unsubscribe from the bluetooth event service. Once unsubscribed, the previously registered
//! handler will no longer be called.
void bluetooth_connection_service_unsubscribe(void);

//! @} // group BluetoothConnectionService

//! @addtogroup AppFocusService
//!
//!
//! \brief Handling app focus
//!
//! The AppFocusService is used for apps that require a high degree of user interactivity, like a game
//! when you need to know when to pause your app when a notification covers your app window.
//!
//! @{

//! Callback type for focus events
//! @param in_focus True if the app is in focus, false otherwise.
typedef void (*AppFocusHandler)(bool in_focus);

//! Subscribe to the focus event service. Once subscribed, the handler
//! gets called every time the app focus changes.
//! @note In focus events are triggered when the app is no longer covered by a
//! modal window.
//! @note Out focus events are triggered when the app becomes covered by a modal
//! window.
//! @param handler A callback to be executed on in-focus events.
void app_focus_service_subscribe(AppFocusHandler handler);

//! Unscribe from the focus event service. Once unsubscribed, the previously
//! registered handler will no longer be called.
void app_focus_service_unsubscribe(void);

//! @} // group AppFocusService

//! @addtogroup BatteryStateService
//!
//! \brief Determines when the battery state changes
//!
//! The BatteryStateService API lets you know when the battery state changes, that is,
//! its current charge level, whether it is plugged and charging. It uses the
//! BatteryChargeState structure to describe the current power state of Pebble.
//!
//! Refer to /Examples/watchfaces/classio-battery-connection,
//! which demonstrates using the battery state service in a watchface.
//! @{

//! Structure for retrieval of the battery charge state
typedef struct {
  //! A percentage (0-100) of how full the battery is
  uint8_t charge_percent;
  //! True if the battery is currently being charged. False if not.
  bool is_charging;
  //! True if the charger cable is connected. False if not.
  bool is_plugged;
} BatteryChargeState;

//! Callback type for battery state change events
//! @param charge the state of the battery \ref BatteryChargeState
typedef void (*BatteryStateHandler)(BatteryChargeState charge);

//! Subscribe to the battery state event service. Once subscribed, the handler gets called
//! on every battery state change
//! @param handler A callback to be executed on battery state change event
void battery_state_service_subscribe(BatteryStateHandler handler);

//! Unsubscribe from the battery state event service. Once unsubscribed, the previously registered
//! handler will no longer be called.
void battery_state_service_unsubscribe(void);

//! Peek at the last known battery state.
//! @return a \ref BatterChargeState containing the last known data
BatteryChargeState battery_state_service_peek(void);

//! @} // group BatteryStateService

//! @addtogroup AccelerometerService
//!
//! \brief Using the Pebble accelerometer
//!
//! The AccelerometerService enables the Pebble accelerometer to detect taps,
//! perform measures at a given frequency, and transmit samples in batches to save CPU time
//! and processing.
//!
//! For available code samples, see
//! Examples/watchapps/feature_accel_discs
//! @{

typedef struct __attribute__((__packed__)) {
  //! acceleration along the x axis
  int16_t x;
  //! acceleration along the y axis
  int16_t y;
  //! acceleration along the z axis
  int16_t z;

  //! true if the watch vibrated when this sample was collected
  bool did_vibrate;

  //! Timestamp, in milliseconds
  uint64_t timestamp;
} AccelData;

typedef enum {
  //! Accelerometer's X axis. The positive direction along the X axis goes
  //! toward the right of the watch.
  ACCEL_AXIS_X = 0,
  //! Accelerometer's Y axis. The positive direction along the Y axis goes
  //! toward the top of the watch.
  ACCEL_AXIS_Y = 1,
  //! Accelerometer's Z axis. The positive direction along the Z axis goes
  //! vertically out of the watchface.
  ACCEL_AXIS_Z = 2,
} AccelAxisType;

//! Callback type for accelerometer data events
//! @param data Pointer to the collected accelerometer samples.
//! @param num_samples the number of samples stored in data.
typedef void (*AccelDataHandler)(AccelData *data, uint32_t num_samples);

//! Callback type for accelerometer tap events
//! @param axis the axis on which a tap was registered (x, y, or z)
//! @param direction the direction (-1 or +1) of the tap
typedef void (*AccelTapHandler)(AccelAxisType axis, int32_t direction);

//! Valid accelerometer sampling rates, in Hz
typedef enum {
  ACCEL_SAMPLING_10HZ = 10,
  ACCEL_SAMPLING_25HZ = 25,
  ACCEL_SAMPLING_50HZ = 50,
  ACCEL_SAMPLING_100HZ = 100,
} AccelSamplingRate;

//! Peek at the last recorded reading.
//! @param[out] data a pointer to a pre-allocated AccelData item
//! @note Cannot be used when subscribed to accelerometer data events.
//! @return -1 if the accel is not running
//! @return -2 if subscribed to accelerometer events.
int accel_service_peek(AccelData *data);

//! Change the accelerometer sampling rate.
//! @param rate The sampling rate in Hz (1Hz, 10Hz, 25Hz, 50Hz, and 100Hz possible)
int accel_service_set_sampling_rate(AccelSamplingRate rate);

//! Change the number of samples buffered between each accelerometer data event
//! @param num_samples the number of samples to buffer, between 0 and 25.
int accel_service_set_samples_per_update(uint32_t num_samples);

//! Subscribe to the accelerometer data event service. Once subscribed, the handler
//! gets called every time there are new accelerometer samples available.
//! @note Cannot use \ref accel_service_peek() when subscribed to accelerometer data events.
//! @param handler A callback to be executed on accelerometer data events
//! @param samples_per_update the number of samples to buffer, between 0 and 25.
void accel_data_service_subscribe(uint32_t samples_per_update, AccelDataHandler handler);

//! Unsubscribe from the accelerometer data event service. Once unsubscribed,
//! the previously registered handler will no longer be called.
void accel_data_service_unsubscribe(void);

//! Subscribe to the accelerometer tap event service. Once subscribed, the handler
//! gets called on every tap event emitted by the accelerometer.
//! @param handler A callback to be executed on tap event
void accel_tap_service_subscribe(AccelTapHandler handler);

//! Unsubscribe from the accelerometer tap event service. Once unsubscribed,
//! the previously registered handler will no longer be called.
void accel_tap_service_unsubscribe(void);

//! @} // group AccelerometerService

//! @addtogroup TickTimerService
//! \brief Handling time components
//!
//! The TickTimerService allows your app to be called every time one Time component has changed.
//! This is extremely important for watchfaces. Your app can choose on which time component
//! change a tick should occur. Time components are defined by the TimeUnits enum in pebble.h.
//! @{

//! Callback type for tick timer events
//! @param tick_time the time at which the tick event was triggered
//! @param units_changed which unit change triggered this tick event
typedef void (*TickHandler)(struct tm *tick_time, TimeUnits units_changed);

//! Subscribe to the tick timer event service. Once subscribed, the handler gets called
//! on every requested unit change
//! @param handler A callback to be executed on tick event
//! @param tick_units the unit change we want to handle (seconds, minutes, hours)
void tick_timer_service_subscribe(TimeUnits tick_units, TickHandler handler);

//! Unsubscribe from the tick timer event service. Once unsubscribed, the previously registered
//! handler will no longer be called.
void tick_timer_service_unsubscribe(void);

//! @} // group TickTimerService

//! @} // group EventService

//! @addtogroup DataLogging
//! \brief Enables logging data asynchronously to a mobile app
//!
//! In Pebble OS, data logging is a data storage and transfer subsystem that allows watchapps to save
//! data on non-volatile storage devices when the phone is not available to process it. The API provides
//! your watchapp with a mechanism for short-term data buffering for asynchronous data transmission to
//! a mobile app.
//!
//! Using this API, your Pebble watchapp can create an arbitrary number of logs, but you’re limited in
//! the amount of storage space you can use. Note that approximately 500K is available for data
//! logging, which is shared among all watchapps that use it.
//!
//! Your app can log data to a session, either creating, adding or deleting data to that session.
//! The data is then sent to the associated phone application at the earliest convenience.
//! If a phone is available, the data is sent directly to the phone. Otherwise, it is saved to the
//! watch storage until the watch is connected to a phone.
//!
//!
//! For example:
//!
//! To create a data logging session for 4-byte unsigned integers with a tag of 0x1234, you would do this:
//! \code{.c}
//!
//! DataLoggingSessionRef logging_session = data_logging_create(0x1234, DATA_LOGGING_UINT, 4, false);
//!
//! // Fake creating some data and logging it to the session.
//! uint32_t data[] = { 1, 2, 3, 4 };
//! data_logging_log(logging_session, &data, 4);
//!
//! // Fake creating more data and logging that as well.
//! uint32_t data2[] = { 1, 2 };
//! data_logging_log(logging_session, &data, 2);
//!
//! // When we don't need to log anything else, we can close off the session.
//! data_logging_finish(logging_session);
//! \endcode
//!
//! For code samples, refer to Examples/data-logging-demo.
//! @{

typedef void *DataLoggingSessionRef;

//! The different types of session data that Pebble supports. This type describes the type of a singular item in the data
//! session. Every item in a given session is the same type and size.
typedef enum {
  //! Array of bytes. Remember that this is the type of a single item in the logging session, so using this
  //! type means you'll be logging multiple byte arrays (each a fixed length described by item_length) for the
  //! duration of the session.
  DATA_LOGGING_BYTE_ARRAY = 0,
  //! Unsigned integer. This may be a 1, 2, or 4 byte integer depending on the item_length parameter
  DATA_LOGGING_UINT = 2,
  //! Signed integer. This may be a 1, 2, or 4 byte integer depending on the item_length parameter
  DATA_LOGGING_INT = 3,
} DataLoggingItemType;

typedef enum {
  DATA_LOGGING_SUCCESS = 0,
  DATA_LOGGING_BUSY, //!< Someone else is writing to this logging session
  DATA_LOGGING_FULL, //!< No more space to save data
  DATA_LOGGING_NOT_FOUND, //!< The logging session does not exist
  DATA_LOGGING_CLOSED, //!< The logging session was made inactive
  DATA_LOGGING_INVALID_PARAMS //!< An invalid parameter was passed to one of the functions
} DataLoggingResult;

//! Create a new data logging session.
//!
//! @param tag A tag associated with the logging session.
//! @param item_type The type of data stored in this logging session
//! @param item_length The size of a single data item in bytes
//! @param resume True if we want to look for a logging session of the same tag and
//!   resume logging to it. If this is false and a session with the specified tag exists, that session will
//!   be closed and a new session will be opened.
//! @return An opaque reference to the data logging session
DataLoggingSessionRef
data_logging_create(uint32_t tag, DataLoggingItemType item_type, uint16_t item_length, bool resume);

//! Delete a data logging_session. Logging data is kept until it has successfully been transferred over to the
//! phone, but no data may be added to the session after this function is called.
//!
//! @param logging_session a reference to the data logging session previously allocated using data_logging_create
void data_logging_finish(DataLoggingSessionRef logging_session);

//! Add data to the data logging session. If a phone is available, the data is sent directly
//! to the phone. Otherwise, it is saved to the watch storage until the watch is connected to a phone.
//!
//! @param logging_session a reference to the data logging session you want to add the data to
//! @param data a pointer to the data buffer that contains multiple items
//! @param num_items the number of items to log. This means data must be at least (num_items * item_length)
//!                  long in bytes
//! @return DATA_LOGGING_SUCCESS on success
//!         DATA_LOGGING_NOT_FOUND if the logging session is invalid
//!         DATA_LOGGING_CLOSED if the sesion is not active
//!         DATA_LOGGING_BUSY if the sesion is not available for writing
//!         DATA_LOGGING_INVALID_PARAMS if num_items is 0 or data is NULL
DataLoggingResult
data_logging_log(DataLoggingSessionRef logging_session, const void *data, uint32_t num_items);

//! @} // group DataLogging

//! @addtogroup Logging Logging
//!   \brief Functions related to logging from apps.
//!
//! This module contains the functions necessary to log messages through
//! Bluetooth.
//! @note It is no longer necessary to enable app logging output from the "settings->about" menu on the Pebble for
//! them to be transmitted!  Instead use the "pebble logs" command included with the SDK to activate logs.  The logs
//! will appear right in your console. Logging
//! over Bluetooth is a fairly power hungry operation that non-developers will
//! not need when your apps are distributed.
//! @{

//! Log an app message.
//! Refer the <a class="el" href="http://linux.die.net/man/3/snprintf">snprintf manpage</a> for details about the C formatting string format.
//! @param log_level
//! @param src_filename The source file where the log originates from
//! @param src_line_number The line number in the source file where the log originates from
//! @param fmt A C formatting string
//! @param ... The arguments for the formatting string
//! \note We do not have floating point number support in formatting strings
void app_log(uint8_t log_level, const char* src_filename, int src_line_number, const char* fmt, ...)
    __attribute__((format(printf, 4, 5)));

#define APP_LOG(level, fmt, args...)                                \
  app_log(level, __FILE_NAME__, __LINE__, fmt, ## args)

//! Suggested log level values
typedef enum {
  //! Error level log message
  APP_LOG_LEVEL_ERROR = 1,
  //! Warning level log message
  APP_LOG_LEVEL_WARNING = 50,
  //! Info level log message
  APP_LOG_LEVEL_INFO = 100,
  //! Debug level log message
  APP_LOG_LEVEL_DEBUG = 200,
  //! Verbose Debug level log message
  APP_LOG_LEVEL_DEBUG_VERBOSE = 255,
} AppLogLevel;

//! @} // group Logging

//! @addtogroup Dictionary
//! \brief Data serialization utilities
//!
//!
//! Data residing in different parts of Pebble memory (RAM) may need to be gathered and assembled into
//! a single continuous block for transport over the network via Bluetooth. The process of gathering
//! and assembling this continuous block of data is called serialization.
//!
//! You use data serialization utilities, like Dictionary, Tuple and Tuplet data structures and accompanying
//! functions, to accomplish this task. No transformations are performed on the actual data, however.
//! These Pebble utilities simply help assemble the data into one continuous buffer according to a
//! specific format.
//!
//! \ref AppMessage uses these utilities--in particular, Dictionary--to send information between mobile
//! and Pebble watchapps.
//!
//! <h3>Writing key/value pairs</h3>
//! To write two key/value pairs, without using Tuplets, you would do this:
//! \code{.c}
//! // Byte array + key:
//! static const uint32_t SOME_DATA_KEY = 0xb00bf00b;
//! static const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
//!
//! // CString + key:
//! static const uint32_t SOME_STRING_KEY = 0xabbababe;
//! static const char *string = "Hello World";
//!
//! // Calculate the buffer size that is needed for the final Dictionary:
//! const uint8_t key_count = 2;
//! const uint32_t size = dict_calc_buffer_size(key_count, sizeof(data),
//!                                             strlen(string) + 1);
//!
//! // Stack-allocated buffer in which to create the Dictionary:
//! uint8_t buffer[size];
//!
//! // Iterator variable, keeps the state of the creation serialization process:
//! DictionaryIterator iter;
//!
//! // Begin:
//! dict_write_begin(&iter, buffer, sizeof(buffer));
//! // Write the Data:
//! dict_write_data(&iter, SOME_DATA_KEY, data, sizeof(data));
//! // Write the CString:
//! dict_write_cstring(&iter, SOME_STRING_KEY, string);
//! // End:
//! const uint32_t final_size = dict_write_end(&iter);
//!
//! // buffer now contains the serialized information
//!
//! \endcode
//!
//! <h3>Reading key/value pairs</h3>
//! To iterate over the key/value pairs in the dictionary that
//! was created in the previous example code, you would do this:
//!
//! \code{.c}
//! Tuple *tuple = dict_read_begin_from_buffer(&iter, buffer, final_size);
//! while (tuple) {
//!   switch (tuple->key) {
//!     case SOME_DATA_KEY:
//!       foo(tuple->value->data, tuple->length);
//!       break;
//!     case SOME_STRING_KEY:
//!       bar(tuple->value->cstring);
//!       break;
//!   }
//!   tuple = dict_read_next(&iter);
//! }
//! \endcode
//!
//! <h3>Tuple and Tuplet data structures</h3>
//! To understand the difference between Tuple and Tuplet data structures:
//! Tuple is the header for a serialized key/value pair, while Tuplet is a helper
//! data structure that references the value you want to serialize. This data
//! structure exists to make the creation of a Dictionary easier to write.
//! Use this mnemonic to remember the difference: TupleT(emplate), the Tuplet being
//! a template to create a Dictionary with Tuple structures.
//!
//! For example:
//! \code{.c}
//! Tuplet pairs[] = {
//!   TupletInteger(WEATHER_ICON_KEY, (uint8_t) 1),
//!   TupletCString(WEATHER_TEMPERATURE_KEY, "1234 Fahrenheit"),
//! };
//! uint8_t buffer[256];
//! uint32_t size = sizeof(buffer);
//! dict_serialize_tuplets_to_buffer(pairs, ARRAY_LENGTH(pairs), buffer, &size);
//!
//! // buffer now contains the serialized information
//! \endcode
//! @{

//! Return values for dictionary write/conversion functions.
typedef enum {
  //! The operation returned successfully
  DICT_OK = 0,
  //! There was not enough backing storage to complete the operation
  DICT_NOT_ENOUGH_STORAGE = 1 << 1,
  //! One or more arguments were invalid or uninitialized
  DICT_INVALID_ARGS = 1 << 2,
  //! The lengths and/or count of the dictionary its tuples are inconsistent
  DICT_INTERNAL_INCONSISTENCY = 1 << 3,
  //! A requested operation required additional memory to be allocated, but
  //! the allocation failed, likely due to insufficient remaining heap memory.
  DICT_MALLOC_FAILED = 1 << 4,
} DictionaryResult;

//! Values representing the type of data that the `value` field of a Tuple contains
typedef enum {
  //! The value is an array of bytes
  TUPLE_BYTE_ARRAY = 0,
  //! The value is a zero-terminated, UTF-8 C-string
  TUPLE_CSTRING = 1,
  //! The value is an unsigned integer. The tuple's `.length` field is used to
  //! determine the size of the integer (1, 2, or 4 bytes).
  TUPLE_UINT = 2,
  //! The value is a signed integer. The tuple's `.length` field is used to
  //! determine the size of the integer (1, 2, or 4 bytes).
  TUPLE_INT = 3,
} TupleType;

//! Data structure for one serialized key/value tuple
//! @note The structure is variable length! The length depends on the value data that the tuple
//! contains.
typedef struct __attribute__((__packed__)) {
  //! The key
  uint32_t key;
  //! The type of data that the `.value` fields contains.
  TupleType type:8;
  //! The length of `.value` in bytes
  uint16_t length;
  //! @brief The value itself.
  //!
  //! The different union fields are provided for convenience, avoiding the need for manual casts.
  //! @note The array length is of incomplete length on purpose, to facilitate
  //! variable length data and because a data length of zero is valid.
  //! @note __Important: The integers are little endian!__
  union {
    //! The byte array value. Valid when `.type` is \ref TUPLE_BYTE_ARRAY.
    uint8_t data[0];
    //! The C-string value. Valid when `.type` is \ref TUPLE_CSTRING.
    char cstring[0];
    //! The 8-bit unsigned integer value. Valid when `.type` is \ref TUPLE_UINT
    //! and `.length` is 1 byte.
    uint8_t uint8;
    //! The 16-bit unsigned integer value. Valid when `.type` is \ref TUPLE_UINT
    //! and `.length` is 2 byte.
    uint16_t uint16;
    //! The 32-bit unsigned integer value. Valid when `.type` is \ref TUPLE_UINT
    //! and `.length` is 4 byte.
    uint32_t uint32;
    //! The 8-bit signed integer value. Valid when `.type` is \ref TUPLE_INT
    //! and `.length` is 1 byte.
    int8_t int8;
    //! The 16-bit signed integer value. Valid when `.type` is \ref TUPLE_INT
    //! and `.length` is 2 byte.
    int16_t int16;
    //! The 32-bit signed integer value. Valid when `.type` is \ref TUPLE_INT
    //! and `.length` is 4 byte.
    int32_t int32;
  } value[];
} Tuple;

struct Dictionary;
typedef struct Dictionary Dictionary;

//! An iterator can be used to iterate over the key/value
//! tuples in an existing dictionary, using \ref dict_read_begin_from_buffer(),
//! \ref dict_read_first() and \ref dict_read_next().
//! An iterator can also be used to append key/value tuples to a dictionary,
//! for example using \ref dict_write_data() or \ref dict_write_cstring().
typedef struct {
  Dictionary *dictionary;  //!< The dictionary being iterated
  const void *end;  //!< Points to the first memory address after the last byte of the dictionary
  //! Points to the next Tuple in the dictionary. Given the end of the
  //! Dictionary has not yet been reached: when writing, the next key/value
  //! pair will be written at the cursor. When reading, the next call
  //! to \ref dict_read_next() will return the cursor.
  Tuple *cursor;
} DictionaryIterator;

//! Calculates the number of bytes that a dictionary will occupy, given
//! one or more value lengths that need to be stored in the dictionary.
//! @note The formula to calculate the size of a Dictionary in bytes is:
//! <pre>1 + (n * 7) + D1 + ... + Dn</pre>
//! Where `n` is the number of Tuples in the Dictionary and `Dx` are the sizes
//! of the values in the Tuples. The size of the Dictionary header is 1 byte.
//! The size of the header for each Tuple is 7 bytes.
//! @param tuple_count The total number of key/value pairs in the dictionary.
//! @param ... The sizes of each of the values that need to be
//! stored in the dictionary.
//! @return The total number of bytes of storage needed.
uint32_t dict_calc_buffer_size(const uint8_t tuple_count, ...);

//! Calculates the size of data that has been written to the dictionary.
//! AKA, the "dictionary size". Note that this is most likely different
//! than the size of the backing storage/backing buffer.
//! @param iter The dictionary iterator
//! @return The total number of bytes which have been written to the dictionary.
uint32_t dict_size(DictionaryIterator* iter);

//! Initializes the dictionary iterator with a given buffer and size,
//! resets and empties it, in preparation of writing key/value tuples.
//! @param iter The dictionary iterator
//! @param buffer The storage of the dictionary
//! @param size The storage size of the dictionary
//! @return \ref DICT_OK, \ref DICT_NOT_ENOUGH_STORAGE or \ref DICT_INVALID_ARGS
//! @see dict_calc_buffer_size
//! @see dict_write_end
DictionaryResult dict_write_begin(DictionaryIterator *iter, uint8_t * const buffer, const uint16_t size);

//! Adds a key with a byte array value pair to the dictionary.
//! @param iter The dictionary iterator
//! @param key The key
//! @param data Pointer to the byte array
//! @param size Length of the byte array
//! @return \ref DICT_OK, \ref DICT_NOT_ENOUGH_STORAGE or \ref DICT_INVALID_ARGS
//! @note The data will be copied into the backing storage of the dictionary.
//! @note There is _no_ checking for duplicate keys.
DictionaryResult dict_write_data(DictionaryIterator *iter, const uint32_t key, const uint8_t * const data, const uint16_t size);

//! Adds a key with a C string value pair to the dictionary.
//! @param iter The dictionary iterator
//! @param key The key
//! @param cstring Pointer to the zero-terminated C string
//! @return \ref DICT_OK, \ref DICT_NOT_ENOUGH_STORAGE or \ref DICT_INVALID_ARGS
//! @note The string will be copied into the backing storage of the dictionary.
//! @note There is _no_ checking for duplicate keys.
DictionaryResult dict_write_cstring(DictionaryIterator *iter, const uint32_t key, const char * const cstring);

//! Adds a key with an integer value pair to the dictionary.
//! @param iter The dictionary iterator
//! @param key The key
//! @param integer Pointer to the integer value
//! @param width_bytes The width of the integer value
//! @param is_signed Whether the integer's type is signed or not
//! @return \ref DICT_OK, \ref DICT_NOT_ENOUGH_STORAGE or \ref DICT_INVALID_ARGS
//! @note There is _no_ checking for duplicate keys.
DictionaryResult dict_write_int(DictionaryIterator *iter, const uint32_t key, const void *integer, const uint8_t width_bytes, const bool is_signed);

//! Adds a key with an unsigned, 8-bit integer value pair to the dictionary.
//! @param iter The dictionary iterator
//! @param key The key
//! @param value The unsigned, 8-bit integer value
//! @return \ref DICT_OK, \ref DICT_NOT_ENOUGH_STORAGE or \ref DICT_INVALID_ARGS
//! @note There is _no_ checking for duplicate keys.
//! @note There are counterpart functions for different signedness and widths,
//! `dict_write_uint16()`, `dict_write_uint32()`, `dict_write_int8()`,
//! `dict_write_int16()` and `dict_write_int32()`. The documentation is not
//! repeated for brevity's sake.
DictionaryResult dict_write_uint8(DictionaryIterator *iter, const uint32_t key, const uint8_t value);

DictionaryResult dict_write_uint16(DictionaryIterator *iter, const uint32_t key, const uint16_t value);

DictionaryResult dict_write_uint32(DictionaryIterator *iter, const uint32_t key, const uint32_t value);

DictionaryResult dict_write_int8(DictionaryIterator *iter, const uint32_t key, const int8_t value);

DictionaryResult dict_write_int16(DictionaryIterator *iter, const uint32_t key, const int16_t value);

DictionaryResult dict_write_int32(DictionaryIterator *iter, const uint32_t key, const int32_t value);

//! End a series of writing operations to a dictionary.
//! This must be called before reading back from the dictionary.
//! @param iter The dictionary iterator
//! @return The size in bytes of the finalized dictionary, or 0 if the parameters were invalid.
uint32_t dict_write_end(DictionaryIterator *iter);

//! Initializes the dictionary iterator with a given buffer and size,
//! in preparation of reading key/value tuples.
//! @param iter The dictionary iterator
//! @param buffer The storage of the dictionary
//! @param size The storage size of the dictionary
//! @return The first tuple in the dictionary, or NULL in case the dictionary was empty or if there was a parsing error.
Tuple * dict_read_begin_from_buffer(DictionaryIterator *iter, const uint8_t * const buffer, const uint16_t size);

//! Progresses the iterator to the next key/value pair.
//! @param iter The dictionary iterator
//! @return The next tuple in the dictionary, or NULL in case the end has been reached or if there was a parsing error.
Tuple * dict_read_next(DictionaryIterator *iter);

//! Resets the iterator back to the same state as a call to \ref dict_read_begin_from_buffer() would do.
//! @param iter The dictionary iterator
//! @return The first tuple in the dictionary, or NULL in case the dictionary was empty or if there was a parsing error.
Tuple * dict_read_first(DictionaryIterator *iter);

//! Non-serialized, template data structure for a key/value pair.
//! For strings and byte arrays, it only has a pointer to the actual data.
//! For integers, it provides storage for integers up to 32-bits wide.
//! The Tuplet data structure is useful when creating dictionaries from values
//! that are already stored in arbitrary buffers.
//! See also \ref Tuple, with is the header of a serialized key/value pair.
typedef struct Tuplet {
  //! The type of the Tuplet. This determines which of the struct fields in the
  //! anonymomous union are valid.
  TupleType type;
  //! The key.
  uint32_t key;
  //! Anonymous union containing the reference to the Tuplet's value, being
  //! either a byte array, c-string or integer. See documentation of `.bytes`,
  //! `.cstring` and `.integer` fields.
  union {
    //! Valid when `.type.` is \ref TUPLE_BYTE_ARRAY
    struct {
      //! Pointer to the data
      const uint8_t *data;
      //! Length of the data
      const uint16_t length;
    } bytes;
    //! Valid when `.type.` is \ref TUPLE_CSTRING
    struct {
      //! Pointer to the c-string data
      const char *data;
      //! Length of the c-string, including terminating zero.
      const uint16_t length;
    } cstring;
    //! Valid when `.type.` is \ref TUPLE_INT or \ref TUPLE_UINT
    struct {
      //! Actual storage of the integer.
      //! The signedness can be derived from the `.type` value.
      uint32_t storage;
      //! Width of the integer.
      const uint16_t width;
    } integer;
  }; //!< See documentation of `.bytes`, `.cstring` and `.integer` fields.
} Tuplet;

#define TupletBytes(_key, _data, _length) \
((const Tuplet) { .type = TUPLE_BYTE_ARRAY, .key = _key, .bytes = { .data = _data, .length = _length }})

#define TupletCString(_key, _cstring) \
((const Tuplet) { .type = TUPLE_CSTRING, .key = _key, .cstring = { .data = _cstring, .length = _cstring ? strlen(_cstring) + 1 : 0 }})

#define TupletInteger(_key, _integer) \
((const Tuplet) { .type = IS_SIGNED(_integer) ? TUPLE_INT : TUPLE_UINT, .key = _key, .integer = { .storage = _integer, .width = sizeof(_integer) }})

//! Callback for \ref dict_serialize_tuplets() utility.
//! @param data The data of the serialized dictionary
//! @param size The size of data
//! @param context The context pointer as passed in to \ref dict_serialize_tuplets()
//! @see dict_serialize_tuplets
typedef void (*DictionarySerializeCallback)(const uint8_t * const data, const uint16_t size, void *context);

//! Utility function that takes a list of Tuplets from which a dictionary
//! will be serialized, ready to transmit or store.
//! @note The callback will be called before the function returns, so the data that
//! that `context` points to, can be stack allocated.
//! @param callback The callback that will be called with the serialized data of the generated dictionary.
//! @param context Pointer to any application specific data that gets passed into the callback.
//! @param tuplets An array of Tuplets that need to be serialized into the dictionary.
//! @param tuplets_count The number of tuplets that follow.
//! @return \ref DICT_OK, \ref DICT_NOT_ENOUGH_STORAGE or \ref DICT_INVALID_ARGS
DictionaryResult dict_serialize_tuplets(DictionarySerializeCallback callback, void *context, const Tuplet * const tuplets, const uint8_t tuplets_count);

//! Utility function that takes an array of Tuplets and serializes them into
//! a dictionary with a given buffer and size.
//! @param tuplets The array of tuplets
//! @param tuplets_count The number of tuplets in the array
//! @param buffer The buffer in which to write the serialized dictionary
//! @param [in] size_in_out The available buffer size in bytes
//! @param [out] size_in_out The number of bytes written
//! @return \ref DICT_OK, \ref DICT_NOT_ENOUGH_STORAGE or \ref DICT_INVALID_ARGS
DictionaryResult dict_serialize_tuplets_to_buffer(const Tuplet * const tuplets, const uint8_t tuplets_count, uint8_t *buffer, uint32_t *size_in_out);

//! Serializes an array of Tuplets into a dictionary with a given buffer and size.
//! @param iter The dictionary iterator
//! @param tuplets The array of tuplets
//! @param tuplets_count The number of tuplets in the array
//! @param buffer The buffer in which to write the serialized dictionary
//! @param [in] size_in_out The available buffer size in bytes
//! @param [out] size_in_out The number of bytes written
//! @return \ref DICT_OK, \ref DICT_NOT_ENOUGH_STORAGE or \ref DICT_INVALID_ARGS
DictionaryResult dict_serialize_tuplets_to_buffer_with_iter(DictionaryIterator *iter, const Tuplet * const tuplets, const uint8_t tuplets_count, uint8_t *buffer, uint32_t *size_in_out);

//! Serializes a Tuplet and writes the resulting Tuple into a dictionary.
//! @param iter The dictionary iterator
//! @param tuplet The Tuplet describing the key/value pair to write
//! @return \ref DICT_OK, \ref DICT_NOT_ENOUGH_STORAGE or \ref DICT_INVALID_ARGS
DictionaryResult dict_write_tuplet(DictionaryIterator *iter, const Tuplet * const tuplet);

//! Calculates the number of bytes that a dictionary will occupy, given
//! one or more Tuplets that need to be stored in the dictionary.
//! @note See \ref dict_calc_buffer_size() for the formula for the calculation.
//! @param tuplets An array of Tuplets that need to be stored in the dictionary.
//! @param tuplets_count The total number of Tuplets that follow.
//! @return The total number of bytes of storage needed.
//! @see Tuplet
uint32_t dict_calc_buffer_size_from_tuplets(const Tuplet * const tuplets, const uint8_t tuplets_count);

//! Type of the callback used in \ref dict_merge()
//! @param key The key that is being updated.
//! @param new_tuple The new tuple. The tuple points to the actual, updated destination dictionary or NULL_TUPLE
//! in case there was an error (e.g. backing buffer was too small).
//! Therefore the Tuple can be used after the callback returns, until the destination dictionary
//! storage is free'd (by the application itself).
//! @param old_tuple The values that will be replaced with `new_tuple`. The key, value and type will be
//! equal to the previous tuple in the old destination dictionary, however the `old_tuple points
//! to a stack-allocated copy of the old data.
//! @param context Pointer to application specific data
//! The storage backing `old_tuple` can only be used during the callback and
//! will no longer be valid after the callback returns.
//! @see dict_merge
typedef void (*DictionaryKeyUpdatedCallback)(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context);

//! Merges entries from another "source" dictionary into a "destination" dictionary.
//! All Tuples from the source are written into the destination dictionary, while
//! updating the exsting Tuples with matching keys.
//! @param dest The destination dictionary to update
//! @param [in,out] dest_max_size_in_out In: the maximum size of buffer backing `dest`. Out: the final size of the updated dictionary.
//! @param source The source dictionary of which its Tuples will be used to update dest.
//! @param update_existing_keys_only Specify True if only the existing keys in `dest` should be updated.
//! @param key_callback The callback that will be called for each Tuple in the merged destination dictionary.
//! @param context Pointer to app specific data that will get passed in when `update_key_callback` is called.
//! @return \ref DICT_OK, \ref DICT_INVALID_ARGS, \ref DICT_NOT_ENOUGH_STORAGE
DictionaryResult dict_merge(DictionaryIterator *dest, uint32_t *dest_max_size_in_out,
                             DictionaryIterator *source,
                             const bool update_existing_keys_only,
                             const DictionaryKeyUpdatedCallback key_callback, void *context);

//! Tries to find a Tuple with specified key in a dictionary
//! @param iter Iterator to the dictionary to search in.
//! @param key The key for which to find a Tuple
//! @return Pointer to a found Tuple, or NULL if there was no Tuple with the specified key.
Tuple *dict_find(const DictionaryIterator *iter, const uint32_t key);

//! @} // group Dictionary

//! @addtogroup AppMessage
//!
//!
//!
//! \brief Bi-directional communication between phone apps and Pebble watchapps
//!
//! AppMessage is a bi-directional messaging subsystem that enables communication between phone apps
//! and Pebble watchapps. This is accomplished by allowing phone and watchapps to exchange arbitrary
//! sets of key/value pairs. The key/value pairs are stored in the form of a Dictionary, the layout
//! of which is left for the application developer to define.
//!
//! AppMessage implements a push-oriented messaging protocol, enabling your app to call functions and
//! methods to push messages from Pebble to phone and vice versa. The protocol is symmetric: both Pebble
//! and the phone can send messages. All messages are acknowledged. In this context, there is no
//! client-server model, as such.
//!
//! During the sending phase, one side initiates the communication by transferring a dictionary over the air.
//! The other side then receives this message and is given an opportunity to perform actions on that data.
//! As soon as possible, the other side is expected to reply to the message with a simple acknowledgment
//! that the message was received successfully.
//!
//! PebbleKit JavaScript provides you with a set of standard JavaScript APIs that let your app receive messages
//! from the watch, make HTTP requests, and send new messages to the watch. AppMessage APIs are used to send and
//! receive data. A Pebble watchapp can use the resources of the connected phone to fetch information from web services,
//! send information to web APIs, or store login credentials. On the JavaScript side, you communicate
//! with Pebble via a Pebble object exposed in the namespace.
//!
//! Messages always need to get either ACKnowledged or "NACK'ed," that is, not acknowledged.
//! If not, messages will result in a time-out failure. The AppMessage subsystem takes care of this implicitly.
//! In the phone libraries, this step is a bit more explicit.
//!
//! The Pebble watch interfaces make a distinction between the Inbox and the Outbox calls. The Inbox
//! receives messages from the phone on the watch; the Outbox sends messages from the watch to the phone.
//! These two buffers can be managed separately.
//!
//! <h4>Warning</h4>
//! A critical constraint of AppMessage is that messages are limited in size. An ingoing (outgoing) message
//! larger than the inbox (outbox) will not be transmitted and will generate an error. You can choose your
//! inbox and outbox size when you call app_message_open().
//!
//! Pebble SDK provides a static minimum guaranteed size (APP_MESSAGE_INBOX_SIZE_MINIMUM and APP_MESSAGE_OUTBOX_SIZE_MINIMUM).
//! Requesting a buffer of the minimum guaranteed size (or smaller) is always guaranteed to succeed on all
//! Pebbles in this SDK version or higher, and with every phone.
//!
//! In some context, Pebble might be able to provide your application with larger inbox/outbox.
//! You can call app_message_inbox_size_maximum() and app_message_outbox_size_maximum() in your code to get
//! the largest possible value you can use.
//!
//! To always get the largest buffer available, follow this best practice:
//!
//! app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum())
//!
//! AppMessage uses your application heap space. That means that the sizes you pick for the AppMessage
//! inbox and outbox buffers are important in optimizing your app’s performance. The more you use for
//! AppMessage, the less space you’ll have for the rest of your app.
//!
//! To register callbacks, you should call app_message_register_inbox_received(), app_message_register_inbox_received(),
//! app_message_register_outbox_sent(), app_message_register_outbox_failed().
//!
//! Pebble recommends that you call them before app_message_open() to ensure you do not miss a message
//! arriving between starting AppMessage and registering the callback. You can set a context that will be passed
//! to all the callbacks with app_message_set_context().
//!
//! In circumstances that may not be ideal, when using AppMessage several types of errors may occur.
//! For example:
//!
//! * The send can’t start because the system state won't allow for a success. Several reasons
//!  you're unable to perform a send: A send() is already occurring (only one is possible at a time) or Bluetooth
//!  is not enabled or connected.
//! * The send and receive occur, but the receiver can’t accept the message. For instance, there is no app
//!   that receives such a message.
//! * The send occurs, but the receiver either does not actually receive the message or can’t handle it
//!   in a timely fashion.
//! * In the case of a dropped message, the phone sends a message to the watchapp, while there is still
//!   an unprocessed message in the Inbox.
//!
//! Other errors are possible and described by AppMessageResult. A client of the AppMessage interface
//! should use the result codes to be more robust in the face of communication problems either in the field or while debugging.
//!
//! Refer to the \htmlinclude app-phone-communication.html for a conceptual overview and code usage.
//!
//! For code examples, refer to the SDK Examples that directly use App Message. These include:
//!   * Examples/todolist-demo
//!   * Examples/pebblekit-js/quotes
//!   * Examples/weather-demo
//! @{

//! AppMessage result codes.
typedef enum {
  //! All good, operation was successful.
  APP_MSG_OK = 0,

  //! The other end did not confirm receiving the sent data with an (n)ack in time.
  APP_MSG_SEND_TIMEOUT = 1 << 1,

  //! The other end rejected the sent data, with a "nack" reply.
  APP_MSG_SEND_REJECTED = 1 << 2,

  //! The other end was not connected.
  APP_MSG_NOT_CONNECTED = 1 << 3,

  //! The local application was not running.
  APP_MSG_APP_NOT_RUNNING = 1 << 4,

  //! The function was called with invalid arguments.
  APP_MSG_INVALID_ARGS = 1 << 5,

  //! There are pending (in or outbound) messages that need to be processed first before
  //! new ones can be received or sent.
  APP_MSG_BUSY = 1 << 6,

  //! The buffer was too small to contain the incoming message.
  APP_MSG_BUFFER_OVERFLOW = 1 << 7,

  //! The resource had already been released.
  APP_MSG_ALREADY_RELEASED = 1 << 9,

  //! The callback node was already registered, or its ListNode has not been initialized.
  APP_MSG_CALLBACK_ALREADY_REGISTERED = 1 << 10,

  //! The callback could not be deregistered, because it had not been registered before.
  APP_MSG_CALLBACK_NOT_REGISTERED = 1 << 11,

  //! The support library did not have sufficient application memory to perform the requested operation.
  APP_MSG_OUT_OF_MEMORY = 1 << 12,

  //! App message was closed
  APP_MSG_CLOSED = 1 << 13,

  //! An internal OS error prevented APP_MSG from completing an operation
  APP_MSG_INTERNAL_ERROR = 1 << 14,

} AppMessageResult;

//! Open AppMessage to transfers.
//!
//! Use \ref dict_calc_buffer_size_from_tuplets() or \ref dict_calc_buffer_size() to estimate the size you need.
//!
//! \param[in] size_inbound The required size for the Inbox buffer
//! \param[in] size_outbound The required size for the Outbox buffer
//!
//! \return A result code such as \ref APP_MSG_OK or \ref APP_MSG_OUT_OF_MEMORY.
//!
//! \note It is recommended that if the Inbox will be used, that at least the Inbox callbacks should be registered
//!   before this call.  Otherwise it is possible for an Inbox message to be NACK'ed without being seen by the
//!   application.
//!
AppMessageResult app_message_open(const uint32_t size_inbound, const uint32_t size_outbound);

//! Deregisters all callbacks and their context.
//!
void app_message_deregister_callbacks(void);

//! Called after an incoming message is received.
//!
//! \param[in] iterator
//!   The dictionary iterator to the received message.  Never NULL.  Note that the iterator cannot be modified or
//!   saved off.  The library may need to re-use the buffered space where this message is supplied.  Returning from
//!   the callback indicates to the library that the received message contents are no longer needed or have already
//!   been externalized outside its buffering space and iterator.
//!
//! \param[in] context
//!   Pointer to application data as specified when registering the callback.
//!
typedef void (*AppMessageInboxReceived)(DictionaryIterator *iterator, void *context);

//! Called after an incoming message is dropped.
//!
//! \param[in] result
//!   The reason why the message was dropped.  Some possibilities include \ref APP_MSG_BUSY and
//!   \ref APP_MSG_BUFFER_OVERFLOW.
//!
//! \param[in] context
//!   Pointer to application data as specified when registering the callback.
//!
//! Note that you can call app_message_outbox_begin() from this handler to prepare a new message.
//! This will invalidate the previous dictionary iterator; do not use it after calling app_message_outbox_begin().
//!
typedef void (*AppMessageInboxDropped)(AppMessageResult reason, void *context);

//! Called after an outbound message has been sent and the reply has been received.
//!
//! \param[in] iterator
//!   The dictionary iterator to the sent message.  The iterator will be in the final state that was sent.  Note that
//!   the iterator cannot be modified or saved off as the library will re-open the dictionary with dict_begin() after
//!   this callback returns.
//!
//! \param[in] context
//!   Pointer to application data as specified when registering the callback.
//!
typedef void (*AppMessageOutboxSent)(DictionaryIterator *iterator, void *context);

//! Called after an outbound message has not been sent successfully.
//!
//! \param[in] iterator
//!   The dictionary iterator to the sent message.  The iterator will be in the final state that was sent.  Note that
//!   the iterator cannot be modified or saved off as the library will re-open the dictionary with dict_begin() after
//!   this callback returns.
//!
//! \param[in] result
//!   The result of the operation.  Some possibilities for the value include \ref APP_MSG_SEND_TIMEOUT,
//!   \ref APP_MSG_SEND_REJECTED, \ref APP_MSG_NOT_CONNECTED, \ref APP_MSG_APP_NOT_RUNNING, and the combination
//!   `(APP_MSG_NOT_CONNECTED | APP_MSG_APP_NOT_RUNNING)`.
//!
//! \param context
//!   Pointer to application data as specified when registering the callback.
//!
//! Note that you can call app_message_outbox_begin() from this handler to prepare a new message.
//! This will invalidate the previous dictionary iterator; do not use it after calling app_message_outbox_begin().
//!
typedef void (*AppMessageOutboxFailed)(DictionaryIterator *iterator, AppMessageResult reason, void *context);

//! Gets the context that will be passed to all AppMessage callbacks.
//!
//! \return The current context on record.
//!
void *app_message_get_context(void);

//! Sets the context that will be passed to all AppMessage callbacks.
//!
//! \param[in] context The context that will be passed to all AppMessage callbacks.
//!
//! \return The previous context that was on record.
//!
void *app_message_set_context(void *context);

//! Registers a function that will be called after any Inbox message is received successfully.
//!
//! Only one callback may be registered at a time.  Each subsequent call to this function will replace the previous
//! callback.  The callback is optional; setting it to NULL will deregister the current callback and no function will
//! be called anymore.
//!
//! \param[in] received_callback The callback that will be called going forward; NULL to not have a callback.
//!
//! \return The previous callback (or NULL) that was on record.
//!
AppMessageInboxReceived app_message_register_inbox_received(AppMessageInboxReceived received_callback);

//! Registers a function that will be called after any Inbox message is received but dropped by the system.
//!
//! Only one callback may be registered at a time.  Each subsequent call to this function will replace the previous
//! callback.  The callback is optional; setting it to NULL will deregister the current callback and no function will
//! be called anymore.
//!
//! \param[in] dropped_callback The callback that will be called going forward; NULL to not have a callback.
//!
//! \return The previous callback (or NULL) that was on record.
//!
AppMessageInboxDropped app_message_register_inbox_dropped(AppMessageInboxDropped dropped_callback);

//! Registers a function that will be called after any Outbox message is sent and an ACK reply occurs in a timely
//! fashion.
//!
//! Only one callback may be registered at a time.  Each subsequent call to this function will replace the previous
//! callback.  The callback is optional; setting it to NULL will deregister the current callback and no function will
//! be called anymore.
//!
//! \param[in] sent_callback The callback that will be called going forward; NULL to not have a callback.
//!
//! \return The previous callback (or NULL) that was on record.
//!
AppMessageOutboxSent app_message_register_outbox_sent(AppMessageOutboxSent sent_callback);

//! Registers a function that will be called after any Outbox message is not sent with a timely ACK reply.
//! The call to \ref app_message_outbox_send() must have succeeded.
//!
//! Only one callback may be registered at a time.  Each subsequent call to this function will replace the previous
//! callback.  The callback is optional; setting it to NULL will deregister the current callback and no function will
//! be called anymore.
//!
//! \param[in] failed_callback The callback that will be called going forward; NULL to not have a callback.
//!
//! \return The previous callback (or NULL) that was on record.
//!
AppMessageOutboxFailed app_message_register_outbox_failed(AppMessageOutboxFailed failed_callback);

//! Programatically determine the inbox size maximum in the current configuration.
//!
//! \return The inbox size maximum on this firmware.
//!
//! \sa APP_MESSAGE_INBOX_SIZE_MINIMUM
//! \sa app_message_outbox_size_maximum()
//!
uint32_t app_message_inbox_size_maximum(void);

//! Programatically determine the outbox size maximum in the current configuration.
//!
//! \return The outbox size maximum on this firmware.
//!
//! \sa APP_MESSAGE_OUTBOX_SIZE_MINIMUM
//! \sa app_message_inbox_size_maximum()
//!
uint32_t app_message_outbox_size_maximum(void);

//! Begin writing to the Outbox's Dictionary buffer.
//!
//! \param[out] iterator Location to write the DictionaryIterator pointer.  This will be NULL on failure.
//!
//! \return A result code, including but not limited to \ref APP_MSG_OK, \ref APP_MSG_INVALID_ARGS or
//!   \ref APP_MSG_BUSY.
//!
//! \note After a successful call, one can add values to the dictionary using functions like \ref dict_write_data()
//!   and friends.
//!
//! \sa Dictionary
//!
AppMessageResult app_message_outbox_begin(DictionaryIterator **iterator);

//! Sends the outbound dictionary.
//!
//! \return A result code, including but not limited to \ref APP_MSG_OK or \ref APP_MSG_BUSY.  The APP_MSG_OK code does
//!         not mean that the message was sent successfully, but only that the start of processing was successful.
//!         Since this call is asynchronous, callbacks provide the final result instead.
//!
//! \sa AppMessageOutboxSent
//! \sa AppMessageOutboxFailed
//!
AppMessageResult app_message_outbox_send(void);

//! As long as the firmware maintains its current major version, inboxes of this size or smaller will be allowed.
//!
//! \sa app_message_inbox_size_maximum()
//! \sa APP_MESSAGE_OUTBOX_SIZE_MINIMUM
//!
#define APP_MESSAGE_INBOX_SIZE_MINIMUM 124

//! As long as the firmware maintains its current major version, outboxes of this size or smaller will be allowed.
//!
//! \sa app_message_outbox_size_maximum()
//! \sa APP_MESSAGE_INBOX_SIZE_MINIMUM
//!
#define APP_MESSAGE_OUTBOX_SIZE_MINIMUM 636

//! @} // group AppMessage

//! @addtogroup AppSync
//! \brief UI synchronization layer for AppMessage
//!
//! AppSync is a convenience layer that resides on top of \ref AppMessage, and serves
//! as a UI synchronization layer for AppMessage. In so doing, AppSync makes it easier
//! to drive the information displayed in the watchapp UI with messages sent by a phone app.
//!
//! AppSync maintains and updates a Dictionary, and provides your app with a callback
//! (AppSyncTupleChangedCallback) routine that is called whenever the Dictionary changes
//! and the app's UI is updated. Note that the app UI is not updated automatically.
//! To update the UI, you need to implement the callback.
//!
//! Pebble OS provides support for data serialization utilities, like Dictionary,
//! Tuple and Tuplet data structures and their accompanying functions. You use Tuplets to create
//! a Dictionary with Tuple structures.
//!
//! AppSync manages the storage and bookkeeping chores of the current Tuple values. AppSync copies
//! incoming AppMessage Tuples into this "current" Dictionary, so that the key/values remain available
//! for the UI to use. For example, it is safe to use a C-string value provided by AppSync and use it
//! directly in a text_layer_set_text() call.
//!
//! Your app needs to supply the buffer that AppSync uses for the "current" Dictionary when initializing AppSync.
//!
//! Refer to the \htmlinclude app-phone-communication.html for a conceptual overview and code usage.
//! @{

//! Called whenever a Tuple changes. This does not necessarily mean the value in
//! the Tuple has changed. When the internal "current" dictionary gets updated,
//! existing Tuples might get shuffled around in the backing buffer, even though
//! the values stay the same. In this callback, the client code gets the chance
//! to remove the old reference and start using the new one.
//! In this callback, your application MUST clean up any references to the
//! `old_tuple` of a PREVIOUS call to this callback (and replace it with the
//! `new_tuple` that is passed in with the current call).
//! @param key The key for which the Tuple was changed.
//! @param new_tuple The new tuple. The tuple points to the actual, updated
//! "current" dictionary, as backed by the buffer internal to the AppSync
//! struct. Therefore the Tuple can be used after the callback returns, until
//! the AppSync is deinited. In case there was an error (e.g. storage shortage),
//! this `new_tuple` can be \ref NULL_TUPLE.
//! @param old_tuple The values that will be replaced with `new_tuple`. The key,
//! value and type will be equal to the previous tuple in the old destination
//! dictionary; however, the `old_tuple` points to a stack-allocated copy of the
//! old data. This value will be \ref NULL_TUPLE when the initial values are
//! being set.
//! @param context Pointer to application specific data, as set using
//! \ref app_sync_init()
//! @see \ref app_sync_init()
typedef void (*AppSyncTupleChangedCallback)(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context);

//! Called whenever there was an error.
//! @param dict_error The dictionary result error code, if the error was
//! dictionary related.
//! @param app_message_error The app_message result error code, if the error
//! was app_message related.
//! @param context Pointer to application specific data, as set using
//! \ref app_sync_init()
//! @see \ref app_sync_init()
typedef void (*AppSyncErrorCallback)(DictionaryResult dict_error, AppMessageResult app_message_error, void *context);

typedef struct AppSync {
  DictionaryIterator current_iter;
  union {
    Dictionary *current;
    uint8_t *buffer;
  };
  uint16_t buffer_size;
  struct {
    AppSyncTupleChangedCallback value_changed;
    AppSyncErrorCallback error;
    void *context;
  } callback;
} AppSync;

//! Initialized an AppSync system with specific buffer size and initial keys and
//! values. The `callback.value_changed` callback will be called
//! __asynchronously__ with the initial keys and values, as to avoid duplicating
//! code to update your app's UI.
//! @param s The AppSync context to initialize
//! @param buffer The buffer that AppSync should use
//! @param buffer_size The size of the backing storage of the "current"
//! dictionary. Use \ref dict_calc_buffer_size_from_tuplets() to estimate the
//! size you need.
//! @param keys_and_initial_values An array of Tuplets with the initial keys and
//! values.
//! @param count The number of Tuplets in the `keys_and_initial_values` array.
//! @param tuple_changed_callback The callback that will handle changed
//! key/value pairs
//! @param error_callback The callback that will handle errors
//! @param context Pointer to app specific data that will get passed into calls
//! to the callbacks
//! @note Only updates for the keys specified in this initial array will be
//! accepted by AppSync, updates for other keys that might come in will just be
//! ignored.
void app_sync_init(struct AppSync *s, uint8_t *buffer, const uint16_t buffer_size, const Tuplet * const keys_and_initial_values, const uint8_t count,
                   AppSyncTupleChangedCallback tuple_changed_callback, AppSyncErrorCallback error_callback, void *context);

//! Cleans up an AppSync system.
//! It frees the buffer allocated by an \ref app_sync_init() call and
//! deregisters itself from the \ref AppMessage subsystem.
//! @param s The AppSync context to deinit.
void app_sync_deinit(struct AppSync *s);

//! Updates key/value pairs using an array of Tuplets.
//! @note The call will attempt to send the updated keys and values to the
//! application on the other end.
//! Only after the other end has acknowledged the update, the `.value_changed`
//! callback will be called to confirm the update has completed and your
//! application code can update its user interface.
//! @param s The AppSync context
//! @param keys_and_values_to_update An array of Tuplets with the keys and
//! values to update. The data in the Tuplets are copied during the call, so the
//! array can be stack-allocated.
//! @param count The number of Tuplets in the `keys_and_values_to_update` array.
//! @return The result code from the \ref AppMessage subsystem.
//! Can be \ref APP_MSG_OK, \ref APP_MSG_BUSY or \ref APP_MSG_INVALID_ARGS
AppMessageResult app_sync_set(struct AppSync *s, const Tuplet * const keys_and_values_to_update, const uint8_t count);

//! Finds and gets a tuple in the "current" dictionary.
//! @param s The AppSync context
//! @param key The key for which to find a Tuple
//! @return Pointer to a found Tuple, or NULL if there was no Tuple with the
//! specified key.
const Tuple * app_sync_get(const struct AppSync *s, const uint32_t key);

//! @} // group AppSync

//! @addtogroup Resources
//! \brief Managing application resources
//!
//! Resources are data files that are bundled with your application binary and can be
//! loaded at runtime. You use resources to embed images or custom fonts in your app,
//! but also to embed any data file. Resources are always read-only.
//!
//! Resources are stored on Pebble’s flash memory and only loaded in RAM when you load
//! them. This means that you can have a large number of resources embedded inside your app,
//! even though Pebble’s RAM memory is very limited.
//!
//! See \htmlinclude UsingResources.html for information on how to embed
//! resources into your app's bundle.
//!
//! @{

//! Opaque reference to a resource.
//! @see \ref resource_get_handle()
typedef const void* ResHandle;

//! Gets the resource handle for a file identifier.
//! @param file_id The resource ID
//! The resource IDs are auto-generated by the Pebble build process, based
//! on the `applib.json`. For example, given the following fragment of a
//! `applib.json`:
//! \code{.json}
//!   ...
//!   "resources" : {
//!     "media": [
//!        {
//!           "name": "MY_ICON",
//!           "file": "img/icon.png",
//!           "type": "png",
//!        },
//!    ...
//!
//! \endcode
//! The "name" value gets prefixed by `RESOURCE_ID_` by the Pebble build
//! process. So, in the example `RESOURCE_ID_MY_ICON` is the file identifier
//! for that resource. To get a resource handle for that resource write:
//! \code{.c}
//! ResHandle rh = resource_get_handle(RESOURCE_ID_MY_ICON);
//! \endcode
ResHandle resource_get_handle(uint32_t resource_id);

//! Gets the size of the resource given a resource handle.
//! @param h The handle to the resource
//! @return The size of the resource in bytes
size_t resource_size(ResHandle h);

//! Copies the bytes for the resource with a given handle from flash storage into a given buffer.
//! @param h The handle to the resource
//! @param buffer The buffer to load the resource data into
//! @param max_length The maximum number of bytes to copy
//! @return The number of bytes actually copied
size_t resource_load(ResHandle h, uint8_t *buffer, size_t max_length);

//! Copies a range of bytes from a resource with a given handle into a given buffer.
//! @param h The handle to the resource
//! @param start_offset The offset in bytes at which to start reading from the resource
//! @param data The buffer to load the resource data into
//! @param num_bytes The maximum number of bytes to copy
//! @return The number of bytes actually copied
size_t resource_load_byte_range(
    ResHandle h, uint32_t start_offset, uint8_t *buffer, size_t num_bytes);

//! @} // group Resources

//! @addtogroup App
//! @{

//! The event loop for apps, to be used in app's main(). Will block until the app is ready to exit.
//! @see \ref App
void app_event_loop(void);

//! @} // group App

//! @addtogroup AppComm App Communication
//!   \brief API for interacting with the Pebble communication subsystem.
//!
//! @note To send messages to a remote device, see the \ref AppMessage or
//! \ref AppSync modules.
//! @{

//! Intervals during which the Bluetooth module may enter a low power mode.
//! The sniff interval defines the period during which the Bluetooth module may
//! not exchange (ACL) packets. The longer the sniff interval, the more time the
//! Bluetooth module may spend in a low power mode.
//! It may be necessary to reduce the sniff interval if an app requires reduced
//! latency when sending messages.
//! @note These settings have a dramatic effect on the Pebble's energy
//! consumption. Use the normal sniff interval whenever possible.  The Bluetooth
//! module is a major consumer of the Pebble's energy.
typedef enum {
  //! Set the sniff interval to normal (power-saving) mode
  SNIFF_INTERVAL_NORMAL = 0,
  //! Reduce the sniff interval to increase the responsiveness of the radio at
  //! the expense of increasing Bluetooth energy consumption by a multiple of 2-5
  //! (very significant)
  SNIFF_INTERVAL_REDUCED = 1,
} SniffInterval;

//! Set the Bluetooth module's sniff interval.
//! The sniff interval will be restored to normal by the OS after the app's
//! de-init handler is called. Set the sniff interval to normal whenever
//! possible.
void app_comm_set_sniff_interval(const SniffInterval interval);

//! Get the Bluetooth module's sniff interval
//! @return The SniffInterval value corresponding to the current interval
SniffInterval app_comm_get_sniff_interval(void);

//! @} // group AppComm

//! @addtogroup Timer Timer
//! \brief Register timers for callbacks
//!
//! The Timer API provides support for calls that let you register a timer for a callback function
//! that is called at a specified time in the future, as well as cancel an already registered timer
//! or reschedule an already running timer at a specified time in the future.
//! @{

//! Waits for a certain amount of milliseconds
//! @param millis The number of milliseconds to wait for
void psleep(int millis);

struct AppTimer;
typedef struct AppTimer AppTimer;

//! The type of function which can be called when a timer fires.  The argument will be the @p callback_data passed to
//! @ref app_timer_register().
typedef void (*AppTimerCallback)(void* data);

//! Registers a timer that ends up in callback being called some specified time in the future.
//! @param timeout_ms The expiry time in milliseconds from the current time
//! @param callback The callback that gets called at expiry time
//! @param callback_data The data that will be passed to callback
AppTimer* app_timer_register(uint32_t timeout_ms, AppTimerCallback callback, void* callback_data);

//! Reschedules an already running timer for some point in the future.
//! @param timer_handle The timer to reschedule
//! @param new_timeout_ms The new expiry time in milliseconds from the current time
//! @return true if the timer was rescheduled, false if the timer has already elapsed
bool app_timer_reschedule(AppTimer *timer_handle, uint32_t new_timeout_ms);

//! Cancels an already registered timer. Once cancelled the the handle may longer be used for any purpose.
void app_timer_cancel(AppTimer *timer_handle);

//! @} // group Timer

//! @addtogroup Storage
//! \brief A mechanism to store persistent application data and state
//!
//! The Persistent Storage API provides you with a mechanism for performing a variety of tasks,
//! like saving user settings, caching data from the phone app, or counting high scores for
//! Pebble watchapp games.
//!
//! In Pebble OS, storage is defined by a collection of fields that you can create, modify or delete.
//! In the API, a field is specified as a key with a corresponding value.
//!
//! Using the Storage API, every app is able to get its own persistent storage space. Each value
//! in that space is associated with a uint32_t key.
//!
//! Storage supports saving integers, strings and byte arrays. The maximum size of byte arrays and
//! strings is defined by PERSIST_DATA_MAX_LENGTH (currently set to 256 bytes). You call the function
//! persist_exists(key), which returns a boolean indicating if the key exists or not.
//! The Storage API enables your app to save its state, and when compared to using \ref AppMessage to
//! retrieve values from the phone, it provides you with a much faster way to restore state.
//! In addition, it draws less power from the battery.
//!
//! Note that the size of all persisted values cannot exceed 4K.
//! @{

//! The maximum size of a persist value in bytes
#define PERSIST_DATA_MAX_LENGTH 256

//! The maximum size of a persist string in bytes including the NULL terminator
#define PERSIST_STRING_MAX_LENGTH PERSIST_DATA_MAX_LENGTH

//! Checks whether a value has been set for a given key in persistent storage.
//! @param key The key of the field to check.
//! @return true if a value exists, otherwise false.
bool persist_exists(const uint32_t key);

//! Gets the size of a value for a given key in persistent storage.
//! @param key The key of the field to lookup the data size.
//! @return The size of the value in bytes or \ref E_DOES_NOT_EXIST
//! if there is no field matching the given key.
int persist_get_size(const uint32_t key);

//! Reads a bool value for a given key from persistent storage.
//! If the value has not yet been set, this will return false.
//! @param key The key of the field to read from.
//! @return The bool value of the key to read from.
bool persist_read_bool(const uint32_t key);

//! Reads an int value for a given key from persistent storage.
//! @note The int is a signed 32-bit integer.
//! If the value has not yet been set, this will return 0.
//! @param key The key of the field to read from.
//! @return The int value of the key to read from.
int32_t persist_read_int(const uint32_t key);

//! Reads a blob of data for a given key from persistent storage into a given buffer.
//! If the value has not yet been set, the given buffer is left unchanged.
//! @param key The key of the field to read from.
//! @param buffer The pointer to a buffer to be written to.
//! @param buffer_size The maximum size of the given buffer.
//! @return The number of bytes written into the buffer or \ref E_DOES_NOT_EXIST
//! if there is no field matching the given key.
int persist_read_data(const uint32_t key, void *buffer, const size_t buffer_size);

//! Reads a string for a given key from persistent storage into a given buffer.
//! The string will be null terminated.
//! If the value has not yet been set, the given buffer is left unchanged.
//! @param key The key of the field to read from.
//! @param buffer The pointer to a buffer to be written to.
//! @param buffer_size The maximum size of the given buffer. This includes the null character.
//! @return The number of bytes written into the buffer or \ref E_DOES_NOT_EXIST
//! if there is no field matching the given key.
int persist_read_string(const uint32_t key, char *buffer, const size_t buffer_size);

//! Writes a bool value flag for a given key into persistent storage.
//! @param key The key of the field to write to.
//! @param value The boolean value to write.
status_t persist_write_bool(const uint32_t key, const bool value);

//! Writes an int value for a given key into persistent storage.
//! @note The int is a signed 32-bit integer.
//! @param key The key of the field to write to.
//! @param value The int value to write.
status_t persist_write_int(const uint32_t key, const int32_t value);

//! Writes a blob of data of a specified size in bytes for a given key into persistent storage.
//! The maximum size is \ref PERSIST_DATA_MAX_LENGTH
//! @param key The key of the field to write to.
//! @param data The pointer to the blob of data.
//! @param size The size in bytes.
//! @return The number of bytes written.
int persist_write_data(const uint32_t key, const void *data, const size_t size);

//! Writes a string a given key into persistent storage.
//! The maximum size is \ref PERSIST_STRING_MAX_LENGTH including the null terminator.
//! @param key The key of the field to write to.
//! @param cstring The pointer to null terminated string.
//! @return The number of bytes written.
int persist_write_string(const uint32_t key, const char *cstring);

//! Deletes the value of a key from persistent storage.
//! @param key The key of the field to delete from.
status_t persist_delete(const uint32_t key);

//! @} // group Storage

//! @} // group Foundation

//! @addtogroup Graphics
//! @{

//! @addtogroup GraphicsTypes Graphics Types
//! \brief Basic graphics types (point, rect, size, color, bitmaps, etc.) and utility functions.
//!
//! @{

//! Color values.
typedef enum GColor {
  //! Represents "clear" or transparent.
  GColorClear = ~0,
  //! Represents black.
  GColorBlack = 0,
  //! Represents white.
  GColorWhite = 1,
} GColor;

//! Represents a point in a 2-dimensional coordinate system.
//! @note Conventionally, the origin of Pebble's 2D coordinate system is in the upper, lefthand corner
//! its x-axis extends to the right and its y-axis extends to the bottom of the screen.
typedef struct GPoint {
  //! The x-coordinate.
  int16_t x;
  //! The y-coordinate.
  int16_t y;
} GPoint;

#define GPoint(x, y) ((GPoint){(x), (y)})

//! Convenience macro to make a GPoint at (0, 0).
#define GPointZero GPoint(0, 0)

//! Tests whether 2 points are equal.
//! @param point_a Pointer to the first point
//! @param point_b Pointer to the second point
//! @return `true` if both points are equal, `false` if not.
bool gpoint_equal(const GPoint * const point_a, const GPoint * const point_b);

//! Represents a 2-dimensional size.
typedef struct GSize {
  //! The width
  int16_t w;
  //! The height
  int16_t h;
} GSize;

#define GSize(w, h) ((GSize){(w), (h)})

//! Convenience macro to make a GSize of (0, 0).
#define GSizeZero GSize(0, 0)

//! Tests whether 2 sizes are equal.
//! @param size_a Pointer to the first size
//! @param size_b Pointer to the second size
//! @return `true` if both sizes are equal, `false` if not.
bool gsize_equal(const GSize *size_a, const GSize *size_b);

//! Represents a rectangle and defining it using the origin of
//! the upper-lefthand corner and its size.
typedef struct GRect {
  //! The coordinate of the upper-lefthand corner point of the rectangle.
  GPoint origin;
  //! The size of the rectangle.
  GSize size;
} GRect;

#define GRect(x, y, w, h) ((GRect){{(x), (y)}, {(w), (h)}})

//! Convenience macro to make a GRect of ((0, 0), (0, 0)).
#define GRectZero GRect(0, 0, 0, 0)

//! Tests whether 2 rectangles are equal.
//! @param rect_a Pointer to the first rectangle
//! @param rect_b Pointer to the second rectangle
//! @return `true` if both rectangles are equal, `false` if not.
bool grect_equal(const GRect* const rect_a, const GRect* const rect_b);

//! Tests whether the size of the rectangle is (0, 0).
//! @param rect Pointer to the rectangle
//! @return `true` if the rectangle its size is (0, 0), or `false` if not.
//! @note If the width and/or height of a rectangle is negative, this
//! function will return `true`!
bool grect_is_empty(const GRect* const rect);

//! Converts a rectangle's values so that the components of its size
//! (width and/or height) are both positive. In the width and/or height are negative,
//! the origin will offset, so that the final rectangle overlaps with the original.
//! For example, a GRect with size (-10, -5) and origin (20, 20), will be standardized
//! to size (10, 5) and origin (10, 15).
//! @param[in] rect The rectangle to convert.
//! @param[out] rect The standardized rectangle.
void grect_standardize(GRect *rect);

//! Trim one rectangle using the edges of a second rectangle.
//! @param[in] rect_to_clip The rectangle that needs to be clipped (in place).
//! @param[out] rect_to_clip The clipped rectangle.
//! @param rect_clipper The rectangle of which the edges will serve as "scissors"
//! in order to trim `rect_to_clip`.
void grect_clip(GRect * const rect_to_clip, const GRect * const rect_clipper);

//! Tests whether a rectangle contains a point.
//! @param rect The rectangle
//! @param point The point
//! @return `true` if the rectangle contains the point, or `false` if it does not.
bool grect_contains_point(const GRect *rect, const GPoint *point);

//! Convenience function to compute the center-point of a given rectangle.
//! This is equal to `(rect->x + rect->width / 2, rect->y + rect->height / 2)`.
//! @param rect The rectangle for which to calculate the center point.
//! @return The point at the center of `rect`
GPoint grect_center_point(const GRect *rect);

//! Reduce the width and height of a rectangle by insetting each of the edges with
//! a fixed inset. The returned rectangle will be centered relative to the input rectangle.
//! @note The function will trip an assertion if the crop yields a rectangle with negative width or height.
//! @param rect The rectangle that will be inset
//! @param crop_size_px The inset by which each of the rectangle will be inset.
//! A positive inset value results in a smaller rectangle, while negative inset value results
//! in a larger rectangle.
//! @return The cropped rectangle.
GRect grect_crop(GRect rect, const int32_t crop_size_px);

//! Structure containing the metadata of a bitmap image.
//!
//! Note that this structure does NOT contain any pixel data; it only has a pointer
//! to a buffer containing the pixels (the `addr` field).
//! The metadata describes how long each row of pixels is in the buffer (the stride).
//! Each row must be a multiple of 32 pixels (4 bytes). Using the `bounds` field,
//! the area that is actually relevant can be specified.
//!
//! For example, when the image is 29 by 5 pixels
//! (width by height) and the first bit of image data is the pixel at (0, 0),
//! then the bounds.size would be `GSize(29, 5)` and bounds.origin would be `GPoint(0, 0)`.
//! ![](gbitmap.png)
//! In the illustration each pixel is a representated as a square. The white
//! squares are the bits that are used, the gray squares are the padding bits, because
//! each row of image data has to be a multiple of 4 bytes (32 bits).
//! The numbers in the column in the left are the offsets (in bytes) from the `*addr`
//! field of the GBitmap.
//!
//! Each pixel in a bitmap is represented by 1 bit. If a bit is set (`1` or `true`), it will result in a
//! white pixel, and vice versa, if a bit is cleared (`0` or `false`), it will result in a black
//! pixel.
//! ![](pixel_bit_values.png)
//! @see \ref BitmapLayer
//! @see \ref graphics_draw_bitmap_in_rect
//! @see \ref Resources
typedef struct __attribute__ ((__packed__)) GBitmap {
  //! Pointer to the address where the image data lives
  void *addr;
  //! @note The number of bytes per row should be a multiple of 4.
  //! Also, the following should (naturally) be true: `(row_size_bytes * 8 >= bounds.w)`
  uint16_t row_size_bytes;

  //! This union is here to make it easy to copy in a full uint16_t of flags from the binary format
  union {
    //! Bitfields of metadata flags.
    uint16_t info_flags;

    struct {
      //! Is .addr heap allocated? Do we need to free .addr in gbitmap_deinit?
      bool is_heap_allocated:1;
      uint16_t reserved:11;
       //! Version of bitmap structure and image data.
      uint8_t version:4;
    };
  };

  //! The box of bits that the `addr` field is pointing to, that contains
  //! the actual image data to use. Note that this may be a subsection of the
  //! data with padding on all sides.
  GRect bounds;
} GBitmap;

//! Creates a new \ref GBitmap on the heap using a Pebble image file stored as a resource.
//! The resulting GBitmap must be destroyed using gbitmap_destroy.
//! @param resource_id The ID of the bitmap resource to load
//! @return A pointer to the \ref GBitmap. `NULL` if the GBitmap could not
//! be created
GBitmap* gbitmap_create_with_resource(uint32_t resource_id);

//! Creates a new GBitmap on the heap initialized with a Pebble image file data
//! (.pbi), as output by bitmapgen.py.
//! @param data The Pebble image file data. Must not be NULL. The function
//! assumes the data to be correct; there are no sanity checks performed on the
//! data.
//! @return A pointer to the \ref GBitmap. `NULL` if the \ref GBitmap could not
//! be created
GBitmap* gbitmap_create_with_data(const uint8_t *data);

//! Create a new \ref GBitmap on the heap as a sub-bitmap of a 'base' \ref
//! GBitmap, using a GRect to indicate what portion of the base to use. The
//! sub-bitmap will just reference the image data of the base bitmap.
//! No deep-copying occurs as a result of calling this function, thus the caller
//! is responsible for making sure the base bitmap will remain available when
//! using the sub-bitmap.
//! @param[in] base_bitmap The bitmap that the sub-bitmap of which the image data
//! will be used by the sub-bitmap
//! @param sub_rect The rectangle within the image data of the base bitmap. The
//! bounds of the base bitmap will be used to clip `sub_rect`.
//! @return A pointer to the \ref GBitmap. `NULL` if the GBitmap could not
//! be created
GBitmap* gbitmap_create_as_sub_bitmap(const GBitmap *base_bitmap, GRect sub_rect);

//! Destroy a \ref GBitmap; free the \ref GBitmap's data if it is dynamically
//! allocated.
//! This must be called for every bitmap that's been created with gbitmap_create_*
void gbitmap_destroy(GBitmap* bitmap);

//! Values to specify how two things should be aligned relative to each other.
//! ![](galign.png)
//! @see \ref bitmap_layer_set_alignment()
typedef enum GAlign {
  //! Align by centering
  GAlignCenter,
  //! Align by making the top edges overlap and left edges overlap
  GAlignTopLeft,
  //! Align by making the top edges overlap and left edges overlap
  GAlignTopRight,
  //! Align by making the top edges overlap and centered horizontally
  GAlignTop,
  //! Align by making the left edges overlap and centered vertically
  GAlignLeft,
  //! Align by making the bottom edges overlap and centered horizontally
  GAlignBottom,
  //! Align by making the right edges overlap and centered vertically
  GAlignRight,
  //! Align by making the bottom edges overlap and right edges overlap
  GAlignBottomRight,
  //! Align by making the bottom edges overlap and left edges overlap
  GAlignBottomLeft
} GAlign;

//! Aligns one rectangle within another rectangle, using an alignment parameter.
//! The relative coordinate systems of both rectangles are assumed to be the same.
//! When clip is true, `rect` is also clipped by the constraint.
//! @param[in] rect The rectangle to align (in place)
//! @param[out] rect The aligned and optionally clipped rectangle
//! @param inside_rect The rectangle in which to align `rect`
//! @param alignment Determines the alignment of `rect` within `inside_rect` by
//! specifying what edges of should overlap.
//! @param clip Determines whether `rect` should be trimmed using the edges of `inside_rect`
//! in case `rect` extends outside of the area that `inside_rect` covers after the alignment.
void grect_align(GRect *rect, const GRect *inside_rect, const GAlign alignment, const bool clip);

//! Values to specify how the source image should be composited onto the destination image.
//!
//! There is no notion of "transparency" in the graphics system. However, the effect of transparency
//! can be created by masking and using compositing modes.
//! ![](compops.png)
//! Contrived example of how the different compositing modes affect drawing.
//! Often, the "destination image" is the render buffer and thus contains the image of
//! what has been drawn before or "underneath".
//! @see \ref bitmap_layer_set_compositing_mode()
//! @see \ref graphics_context_set_compositing_mode()
//! @see \ref graphics_draw_bitmap_in_rect()
typedef enum {
  //! Assign the pixel values of the source image to the destination pixels,
  //! effectively replacing the previous values for those pixels.
  GCompOpAssign,
  //! Assign the **inverted** pixel values of the source image to the destination pixels,
  //! effectively replacing the previous values for those pixels.
  GCompOpAssignInverted,
  //! Use the boolean operator `OR` to composite the source and destination pixels.
  //! The visual result of this compositing mode is the source's white pixels
  //! are painted onto the destination and the source's black pixels are treated
  //! as clear.
  GCompOpOr,
  //! Use the boolean operator `AND` to composite the source and destination pixels.
  //! The visual result of this compositing mode is the source's black pixels
  //! are painted onto the destination and the source's white pixels are treated
  //! as clear.
  GCompOpAnd,
  //! Clears the bits in the destination image, using the source image as mask.
  //! The visual result of this compositing mode is that for the parts where the source image is
  //! white, the destination image will be painted black. Other parts will be left untouched.
  GCompOpClear,
  //! Sets the bits in the destination image, using the source image as mask.
  //! The visual result of this compositing mode is that for the parts where the source image is
  //! black, the destination image will be painted white. Other parts will be left untouched.
  GCompOpSet,
} GCompOp;


typedef struct {
  //! The box relative to bitmap's bounds, that graphics functions MUST use to clip what they draw
  GRect clip_box;
  //! The box relative to bitmap's bounds, that graphics functions MUST use as their coordinate space
  GRect drawing_box;
  //! Line drawing functions MUST use this as line color
  GColor stroke_color:2;
  //! Fill drawing functions MUST use this as fill color
  GColor fill_color:2;
  //! Text drawing functions MUST use this as text color
  GColor text_color:2;
  //! Bitmap compositing functions MUST use this as the compositing mode
  GCompOp compositing_mode:3;
} GDrawState;

struct GContext;
typedef struct GContext GContext;

//! @} // group GraphicsTypes

//! @addtogroup GraphicsContext Graphics Context
//! \brief The "canvas" into which an application draws
//!
//! The Pebble OS graphics engine, inspired by several notable graphics systems, including
//! Apple’s Quartz 2D and its predecessor QuickDraw, provides your app with a canvas into
//! which to draw, namely, the graphics context. A graphics context is the target into which
//! graphics functions can paint, using Pebble drawing routines (see \ref \ref Drawing,
//! \ref PathDrawing and \ref TextDrawing).
//!
//! A graphics context holds a reference to the bitmap into which to paint. It also holds the
//! current drawing state, like the current fill color, stroke color, clipping box, drawing box,
//! compositing mode, and so on. The \ref GContext struct is the type representing the graphics context.
//!
//! For drawing in your Pebble watchface or watchapp, you won't need to create a \ref GContext
//! yourself. In most cases, it is provided by Pebble OS as an argument passed into a render
//! callback (the .update_proc of a Layer).
//!
//! Your app can’t call drawing functions at any given point in time: Pebble OS will request your
//! app to render. Typically, your app will be calling out to graphics functions in
//! the .update_proc callback of a Layer.
//! @see \ref Layer
//! @see \ref Drawing
//! @see \ref PathDrawing
//! @see \ref TextDrawing
//! @{

//! Sets the current stroke color of the graphics context.
//! @param ctx The graphics context onto which to set the stroke color
//! @param color The new stroke color
void graphics_context_set_stroke_color(GContext* ctx, GColor color);

//! Sets the current fill color of the graphics context.
//! @param ctx The graphics context onto which to set the fill color
//! @param color The new fill color
void graphics_context_set_fill_color(GContext* ctx, GColor color);

//! Sets the current text color of the graphics context.
//! @param ctx The graphics context onto which to set the text color
//! @param color The new text color
void graphics_context_set_text_color(GContext* ctx, GColor color);

//! Sets the current bitmap compositing mode of the graphics context.
//! @param ctx The graphics context onto which to set the compositing mode
//! @param mode The new compositing mode
//! @see \ref GCompOp
//! @see \ref bitmap_layer_set_compositing_mode()
//! @note At the moment, this only affects the bitmaps drawing operations
//! -- \ref graphics_draw_bitmap_in_rect() and anything that uses that --, but it
//! currently does not affect the filling or stroking operations.
void graphics_context_set_compositing_mode(GContext* ctx, GCompOp mode);

//! @} // group GraphicsContext

//! @addtogroup Drawing Drawing Primitives
//! \brief Functions to draw into a graphics context
//!
//! Use these drawing functions inside a Layer's `.update_proc` drawing
//! callback. A `GContext` is passed into this callback as an argument.
//! This `GContext` can then be used with all of the drawing functions which
//! are documented below.
//! See \ref GraphicsContext for more information about the graphics context.
//!
//! Refer to \htmlinclude UiFramework.html (chapter "Layers" and "Graphics") for a
//! conceptual overview of the drawing system, Layers and relevant code examples.
//!
//! Other drawing functions and related documentation:
//! * \ref TextDrawing
//! * \ref PathDrawing
//! * \ref GraphicsTypes
//! @{

//! Bit mask values to specify the corners of a rectangle.
//! The values can be combines using binary OR (`|`),
//! For example: the mask to indicate top left and bottom right corners can:
//! be created as follows: `(GCornerTopLeft | GCornerBottomRight)`
typedef enum {
  //! No corners
  GCornerNone = 0,
  //! Top-Left corner
  GCornerTopLeft = 1 << 0,
  //! Top-Right corner
  GCornerTopRight = 1 << 1,
  //! Bottom-Left corner
  GCornerBottomLeft = 1 << 2,
  //! Bottom-Right corner
  GCornerBottomRight = 1 << 3,
  //! All corners
  GCornersAll = GCornerTopLeft | GCornerTopRight | GCornerBottomLeft | GCornerBottomRight,
  //! Top corners
  GCornersTop = GCornerTopLeft | GCornerTopRight,
  //! Bottom corners
  GCornersBottom = GCornerBottomLeft | GCornerBottomRight,
  //! Left corners
  GCornersLeft = GCornerTopLeft | GCornerBottomLeft,
  //! Right corners
  GCornersRight = GCornerTopRight | GCornerBottomRight,
} GCornerMask;

//! Draws a pixel at given point in the current stroke color
//! @param ctx The destination graphics context in which to draw
//! @param point The point at which to draw the pixel
void graphics_draw_pixel(GContext* ctx, GPoint point);

//! Draws a 1-pixel wide line in the current stroke color
//! @param ctx The destination graphics context in which to draw
//! @param p0 The starting point of the line
//! @param p1 The ending point of the line
void graphics_draw_line(GContext* ctx, GPoint p0, GPoint p1);

//! Draws a 1-pixel wide rectangle outline in the current stroke color
//! @param ctx The destination graphics context in which to draw
//! @param rect The rectangle for which to draw the outline
void graphics_draw_rect(GContext* ctx, GRect rect);

//! Fills a retangle with the current fill color, optionally rounding all or a selection of its corners
//! @param ctx The destination graphics context in which to draw
//! @param rect The rectangle to fill
//! @param corner_radius The rounding radius of the corners in pixels (maximum is 8 pixels)
//! @param corner_mask Bitmask of the corners that need to be rounded.
//! @see \ref GCornerMask
void graphics_fill_rect(GContext* ctx, GRect rect, uint16_t corner_radius, GCornerMask corner_mask);

//! Draws the outline of a circle in the current stroke color
//! @param ctx The destination graphics context in which to draw
//! @param p The center point of the circle
//! @param radius The radius in pixels
void graphics_draw_circle(GContext* ctx, GPoint p, uint16_t radius);

//! Fills a circle in the current fill color
//! @param ctx The destination graphics context in which to draw
//! @param p The center point of the circle
//! @param radius The radius in pixels
void graphics_fill_circle(GContext* ctx, GPoint p, uint16_t radius);

//! Draws the outline of a rounded rectangle in the current stroke color
//! @param ctx The destination graphics context in which to draw
//! @param rect The rectangle defining the dimensions of the rounded rectangle to draw
//! @param radius The corner radius in pixels
void graphics_draw_round_rect(GContext* ctx, GRect rect, uint16_t radius);

//! Draws a bitmap into the graphics context, inside the specified rectangle
//! @param ctx The destination graphics context in which to draw the bitmap
//! @param bitmap The bitmap to draw
//! @param rect The rectangle in which to draw the bitmap
//! @note If the size of `rect` is smaller than the size of the bitmap,
//! the bitmap will be clipped on right and bottom edges.
//! If the size of `rect` is larger than the size of the bitmap,
//! the bitmap will be tiled automatically in both horizontal and vertical
//! directions, effectively drawing a repeating pattern.
//! @see GBitmap
//! @see GContext
void graphics_draw_bitmap_in_rect(GContext* ctx, const GBitmap *bitmap, GRect rect);

//! @} // group Drawing

//! @addtogroup PathDrawing Drawing Paths
//! \brief Functions to draw polygons into a graphics context
//!
//! Code example:
//! \code{.c}
//! static GPath *s_my_path_ptr = NULL;
//!
//! static const GPathInfo BOLT_PATH_INFO = {
//!   .num_points = 6,
//!   .points = (GPoint []) {{21, 0}, {14, 26}, {28, 26}, {7, 60}, {14, 34}, {0, 34}}
//! };
//!
//! // .update_proc of my_layer:
//! void my_layer_update_proc(Layer *my_layer, GContext* ctx) {
//!   // Fill the path:
//!   graphics_context_set_fill_color(ctx, GColorWhite);
//!   gpath_draw_filled(ctx, s_my_path_ptr);
//!   // Stroke the path:
//!   graphics_context_set_stroke_color(ctx, GColorBlack);
//!   gpath_draw_outline(ctx, s_my_path_ptr);
//! }
//!
//! void setup_my_path(void) {
//!   s_my_path_ptr = gpath_create(&BOLT_PATH_INFO);
//!   // Rotate 15 degrees:
//!   gpath_rotate_to(s_my_path_ptr, TRIG_MAX_ANGLE / 360 * 15);
//!   // Translate by (5, 5):
//!   gpath_move_to(s_my_path_ptr, GPoint(5, 5));
//! }
//!
//! // For brevity, the setup of my_layer is not written out...
//! \endcode
//! @{

//! Data structure describing a naked path
//! @note Note that this data structure only refers to an array of points;
//! the points are not stored inside this data structure itself.
//! In most cases, one cannot use a stack-allocated array of GPoints. Instead
//! one often needs to provide longer-lived (static or "global") storage for the points.
typedef struct GPathInfo{
  //! The number of points in the `points` array
  uint32_t num_points;
  //! Pointer to an array of points.
  GPoint *points;
} GPathInfo;

//! Data structure describing a path, plus its rotation and translation.
//! @note See the remark with \ref GPathInfo
typedef struct GPath {
  //! The number of points in the `points` array
  uint32_t num_points;
  //! Pointer to an array of points.
  GPoint *points;
  //! The rotation that will be used when drawing the path with
  //! \ref gpath_draw_filled() or \ref gpath_draw_outline()
  int32_t rotation;
  //! The translation that will to be used when drawing the path with
  //! \ref gpath_draw_filled() or \ref gpath_draw_outline()
  GPoint offset;
} GPath;

//! Creates a new GPath on the heap based on a series of points described by a GPathInfo.
//!
//! Values after initialization:
//! * `num_points` and `points` pointer: copied from the GPathInfo.
//! * `rotation`: 0
//! * `offset`: (0, 0)
//! @return A pointer to the GPath. `NULL` if the GPath could not
//! be created
GPath* gpath_create(const GPathInfo *init);

//! Free a dynamically allocated gpath created with \ref gpath_create()
void gpath_destroy(GPath* gpath);

//! Draws the fill of a path into a graphics context, using the current fill color,
//! relative to the drawing area as set up by the layering system.
//! @param ctx The graphics context to draw into
//! @param path The path to fill
//! @see \ref graphics_context_set_fill_color()
void gpath_draw_filled(GContext* ctx, GPath *path);

//! Draws the outline of a path into a graphics context, using the current stroke color,
//! relative to the drawing area as set up by the layering system.
//! @param ctx The graphics context to draw into
//! @param path The path to fill
//! @see \ref graphics_context_set_stroke_color()
void gpath_draw_outline(GContext* ctx, GPath *path);

//! Sets the absolute rotation of the path.
//! The current rotation will be replaced by the specified angle.
//! @param path The path onto which to set the rotation
//! @param angle The absolute angle of the rotation. The angle is represented in the same way
//! that is used with \ref sin_lookup(). See \ref TRIG_MAX_ANGLE for more information.
//! @note Setting a rotation does not affect the points in the path directly.
//! The rotation is applied on-the-fly during drawing, either using \ref gpath_draw_filled() or
//! \ref gpath_draw_outline().
void gpath_rotate_to(GPath *path, int32_t angle);

//! Sets the absolute offset of the path.
//! The current translation will be replaced by the specified offset.
//! @param path The path onto which to set the translation
//! @param point The point which is used as the vector for the translation.
//! @note Setting a translation does not affect the points in the path directly.
//! The translation is applied on-the-fly during drawing, either using \ref gpath_draw_filled() or
//! \ref gpath_draw_outline().
void gpath_move_to(GPath *path, GPoint point);

//! @} // group PathDrawing

//! @addtogroup Fonts
//! @see \ref TextLayer
//! @see \ref TextDrawing
//! @see \ref text_layer_set_font
//! @see \ref graphics_draw_text
//! @{

//! Pointer to opaque font data structure.
//! @see \ref fonts_load_custom_font()
//! @see \ref text_layer_set_font()
//! @see \ref graphics_draw_text()
typedef void* GFont;

//! Loads a system font corresponding to the specified font key.
//! @param font_key The string key of the font to load. See `pebble_fonts.h` for a list of system fonts.
//! @return An opaque pointer to the loaded font, or, a pointer to the default
//! (fallback) font if the specified font cannot be loaded.
//! @note This may load a font from the flash peripheral into RAM.
GFont fonts_get_system_font(const char *font_key);

//! Loads a custom font.
//! @param resource The resource handle of the font to load. See resource_ids.auto.h
//! for a list of resource IDs, and use \ref resource_get_handle() to obtain the resource handle.
//! @return An opaque pointer to the loaded font, or a pointer to the default
//! (fallback) font if the specified font cannot be loaded.
//! @see \htmlinclude UsingResources.html on how to embed a font into your app.
//! @note this may load a font from the flash peripheral into RAM.
GFont fonts_load_custom_font(ResHandle resource);

//! Unloads the specified custom font and frees the memory that is occupied by
//! it.
//! @note When an application exits, the system automatically unloads all fonts
//! that have been loaded.
//! @param font The font to unload.
void fonts_unload_custom_font(GFont font);

//! @} // group Fonts

//! @addtogroup TextDrawing Drawing Text
//!   \brief Functions to draw text into a graphics context
//!
//! See \ref GraphicsContext for more information about the graphics context.
//!
//! Other drawing functions and related documentation:
//! * \ref Drawing
//! * \ref PathDrawing
//! * \ref GraphicsTypes
//! @{

//! Text overflow mode controls the way text overflows when the string that is drawn does not fit
//! inside the area constraint.
//! @see graphics_draw_text
//! @see text_layer_set_overflow_mode
typedef enum {
  //! On overflow, wrap words to a new line below the current one.
  GTextOverflowModeWordWrap,
  //! On overflow, truncate as needed to fit a trailing ellipsis (...).
  GTextOverflowModeTrailingEllipsis,
  //! Acts like GTextOverflowModeTrailingEllipsis but isn't limited to a single line. The text will wrap until the vertical space is consumed and then the final word will be truncated with a trailing ellipsis.
  GTextOverflowModeFill
} GTextOverflowMode;

//! Text aligment controls the way the text is aligned inside the box the text is drawn into.
//! @see graphics_draw_text
//! @see text_layer_set_text_alignment
typedef enum {
  //! Aligns the text to the left of the drawing box
  GTextAlignmentLeft,
  //! Aligns the text centered inside the drawing box
  GTextAlignmentCenter,
  //! Aligns the text to the right of the drawing box
  GTextAlignmentRight,
} GTextAlignment;

struct TextLayout;
typedef struct TextLayout TextLayout;

//! Pointer to opaque text layout cache data structure
typedef TextLayout* GTextLayoutCacheRef;

void graphics_draw_text(GContext* ctx, const char* text, GFont const font, const GRect box, const GTextOverflowMode overflow_mode, const GTextAlignment alignment, const GTextLayoutCacheRef layout);

//! Obtain the maximum size that a text with given font, overflow mode and alignment occupies within a given rectangular constraint.
//! @param text The zero terminated UTF-8 string for which to calculate the size
//! @param font The font in which the text should be set while calculating the size
//! @param box The bounding box in which the text should be constrained
//! @param overflow_mode The overflow behavior, in case the text is larger than what fits inside the box.
//! @param alignment The horizontal alignment of the text
//! @return The maximum size occupied by the text
GSize graphics_text_layout_get_content_size(const char* text, GFont const font, const GRect box, const GTextOverflowMode overflow_mode, const GTextAlignment alignment);

//! @} // group TextDrawing

//! @} // group Graphics

//! @addtogroup UI
//! @{

//! @addtogroup Clicks
//! @{

//! Reference to opaque click recognizer
//! When a \ref ClickHandler callback is called, the recognizer that fired the handler is passed in.
//! @see \ref ClickHandler
typedef void *ClickRecognizerRef;

//! Function signature of the callback that handles a recognized click pattern
//! @param recognizer The click recognizer that detected a "click" pattern
//! @param context Pointer to application specified data as configured using \ref window_set_click_config_provider_with_context()
//! or as set manually in the \ref ClickConfigProvider callback on the `.context` field of the \ref ClickConfig.
//! @see \ref ClickConfigProvider
typedef void (*ClickHandler)(ClickRecognizerRef recognizer, void *context);

//! This callback gets called at times that a click recognizer has to be set up, for example
//! when a new window comes into view.
//! Subscribe to click events using
//!   \ref window_single_click_subscribe()
//!   \ref window_single_repeating_click_subscribe()
//!   \ref window_multi_click_subscribe()
//!   \ref window_long_click_subscribe()
//!   \ref window_raw_click_subscribe()
//! These subscriptions will get used by the click recognizers of each of the 4 buttons.
//! @see ButtonId for the mapping buttons to the array indices.
//! @param context Pointer to application specific data, as configured using \ref window_set_click_config_provider_with_context(), or defaults to the window if registered with \ref window_set_click_config_provider().
typedef void (*ClickConfigProvider)(void *context);

//! Gets the click count.
//! You can use this inside a click handler implementation to get the click count for multi_click
//! and (repeated) click events.
//! @param recognizer The click recognizer for which to get the click count
//! @return The number of consecutive clicks, and for auto-repeating the number of repetitions.
uint8_t click_number_of_clicks_counted(ClickRecognizerRef recognizer);

//! Gets the button identifier.
//! You can use this inside a click handler implementation to get the button id for the click event.
//! @param recognizer The click recognizer for which to get the button id that caused the click event
//! @return the ButtonId of the click recognizer
ButtonId click_recognizer_get_button_id(ClickRecognizerRef recognizer);

//! @} // group Clicks

//! @addtogroup Layer Layers
//! \brief User interface layers for displaying graphic components
//!
//! Layers are objects that can be displayed on a Pebble watchapp window, enabling users to see
//! visual objects, like text or images. Each layer stores the information about its state
//! necessary to draw or redraw the object that it represents and uses graphics routines along with
//! this state to draw itself when asked. Layers can be used to display various graphics.
//!
//! Layers are the basic building blocks for your application UI. Layers can be nested inside each other.
//! Every window has a root layer which is always the topmost layer.
//! You provide a function that is called to draw the content of the layer when needed; or
//! you can use standard layers that are provided by the system, such as text layer, image layer,
//! menu layer, action bar layer, and so on.
//!
//! The Pebble layer hierarchy is the list of things that need to be drawn to the screen.
//! Multiple layers can be arranged into a hierarchy. This enables ordering (front to back),
//! layout and hierarchy. Through relative positioning, visual objects that are grouped together by
//! adding them into the same layer can be moved all at once. This means that the child layers
//! will move accordingly. If a parent layer has clipping enabled, all the children will be clipped
//! to the frame of the parent.
//!
//! Pebble OS provides convenience layers with built-in logic for displaying different graphic
//! components, like text and bitmap layers.
//!
//! Refer to the \htmlinclude UiFramework.html (chapter "Layers") for a conceptual overview
//! of Layers and relevant code examples.
//!
//! The Modules listed here contain what can be thought of conceptually as subclasses of Layer. The
//! listed types can be safely type-casted to `Layer` (or `Layer *` in case of a pointer).
//! The `layer_...` functions can then be used with the data structures of these subclasses.
//! <br/>For example, the following is legal:
//! \code{.c}
//! TextLayer *text_layer;
//! ...
//! layer_set_hidden((Layer *)text_layer, true);
//! \endcode
//! @{

struct Layer;
typedef struct Layer Layer;

//! Function signature for a Layer's render callback (the name of the type
//! is derived from the words 'update procedure').
//! The system will call the `.update_proc` callback whenever the Layer needs
//! to be rendered.
//! @param layer The layer that needs to be rendered
//! @param ctx The destination graphics context to draw into
//! @see \ref Graphics
//! @see \ref layer_set_update_proc()
typedef void (*LayerUpdateProc)(struct Layer *layer, GContext* ctx);

//! Creates a layer on the heap and sets its frame and bounds.
//! Default values:
//! * `bounds` : origin (0, 0) and a size equal to the frame that is passed in.
//! * `clips` : `true`
//! * `hidden` : `false`
//! * `update_proc` : `NULL` (draws nothing)
//! @param frame The frame at which the layer should be initialized.
//! @see \ref layer_set_frame()
//! @see \ref layer_set_bounds()
//! @return A pointer to the layer. `NULL` if the layer could not
//! be created
Layer* layer_create(GRect frame);

//! Creates a layer on the heap with extra space for callback data, and set its frame andbounds.
//! Default values:
//! * `bounds` : origin (0, 0) and a size equal to the frame that is passed in.
//! * `clips` : `true`
//! * `hidden` : `false`
//! * `update_proc` : `NULL` (draws nothing)
//! @param frame The frame at which the layer should be initialized.
//! @param data_size The size (in bytes) of memory to allocate for callback data.
//! @see \ref layer_create()
//! @see \ref layer_set_frame()
//! @see \ref layer_set_bounds()
//! @return A pointer to the layer. `NULL` if the layer could not be created
Layer* layer_create_with_data(GRect frame, size_t data_size);

//! Destroys a layer previously created by layer_create
void layer_destroy(Layer* layer);

//! Marks the complete layer as "dirty", awaiting to be asked by the system to redraw itself.
//! Typically, this function is called whenever state has changed that affects what the layer
//! is displaying.
//! * The layer's `.update_proc` will not be called before this function returns,
//! but will be called asynchronously, shortly.
//! * Internally, a call to this function will schedule a re-render of the window that the
//! layer belongs to. In effect, all layers in that window's layer hierarchy will be asked to redraw.
//! * If an earlier re-render request is still pending, this function is a no-op.
//! @param layer The layer to mark dirty
void layer_mark_dirty(Layer *layer);

//! Sets the layer's render function.
//! The system will call the `update_proc` automatically when the layer needs to redraw itself, see
//! also \ref layer_mark_dirty().
//! @param layer Pointer to the layer structure.
//! @param update_proc Pointer to the function that will be called when the layer needs to be rendered.
//! Typically, one performs a series of drawing commands in the implementation of the `update_proc`,
//! see \ref Drawing, \ref PathDrawing and \ref TextDrawing.
void layer_set_update_proc(Layer *layer, LayerUpdateProc update_proc);

//! Sets the frame of the layer, which is it's bounding box relative to the coordinate
//! system of its parent layer.
//! The size of the layer's bounds will be extended automatically, so that the bounds
//! cover the new frame.
//! @param layer The layer for which to set the frame
//! @param frame The new frame
//! @see \ref layer_set_bounds()
void layer_set_frame(Layer *layer, GRect frame);

//! Gets the frame of the layer, which is it's bounding box relative to the coordinate
//! system of its parent layer.
//! If the frame has changed, \ref layer_mark_dirty() will be called automatically.
//! @param layer The layer for which to get the frame
//! @return The frame of the layer
//! @see layer_set_frame
GRect layer_get_frame(const Layer *layer);

//! Sets the bounds of the layer, which is it's bounding box relative to its frame.
//! If the bounds has changed, \ref layer_mark_dirty() will be called automatically.
//! @param layer The layer for which to set the bounds
//! @param bounds The new bounds
//! @see \ref layer_set_frame()
void layer_set_bounds(Layer *layer, GRect bounds);

//! Gets the bounds of the layer
//! @param layer The layer for which to get the bounds
//! @return The bounds of the layer
//! @see layer_set_bounds
GRect layer_get_bounds(const Layer *layer);

//! Gets the window that the layer is currently attached to.
//! @param layer The layer for which to get the window
//! @return The window that this layer is currently attached to, or `NULL` if it has
//! not been added to a window's layer hierarchy.
//! @see \ref window_get_root_layer()
//! @see \ref layer_add_child()
struct Window *layer_get_window(const Layer *layer);

//! Removes the layer from its current parent layer
//! If removed successfully, the child's parent layer will be marked dirty
//! automatically.
//! @param child The layer to remove
void layer_remove_from_parent(Layer *child);

//! Removes child layers from given layer
//! If removed successfully, the child's parent layer will be marked dirty
//! automatically.
//! @param parent The layer from which to remove all child layers
void layer_remove_child_layers(Layer *parent);

//! Adds the child layer to a given parent layer, making it appear
//! in front of its parent and in front of any existing child layers
//! of the parent.
//! If the child layer was already part of a layer hierarchy, it will
//! be removed from its old parent first.
//! If added successfully, the parent (and children) will be marked dirty
//! automatically.
//! @param parent The layer to which to add the child layer
//! @param child The layer to add to the parent layer
void layer_add_child(Layer *parent, Layer *child);

//! Inserts the layer as a sibling behind another layer.
//! The below_layer has to be a child of a parent layer,
//! otherwise this function will be a noop.
//! If inserted successfully, the parent (and children) will be marked dirty
//! automatically.
//! @param layer_to_insert The layer to insert into the hierarchy
//! @param below_sibling_layer The layer that will be used as the sibling layer
//! above which the insertion will take place
void layer_insert_below_sibling(Layer *layer_to_insert, Layer *below_sibling_layer);

//! Inserts the layer as a sibling in front of another layer.
//! The above_layer has to be a child of a parent layer,
//! otherwise this function will be a noop.
//! If inserted successfully, the parent (and children) will be marked dirty
//! automatically.
//! @param layer_to_insert The layer to insert into the hierarchy
//! @param above_sibling_layer The layer that will be used as the sibling layer
//! below which the insertion will take place
void layer_insert_above_sibling(Layer *layer_to_insert, Layer *above_sibling_layer);

//! Sets the visibility of the layer.
//! If the visibility has changed, \ref layer_mark_dirty() will be called automatically
//! on the parent layer.
//! @param layer The layer for which to set the visibility
//! @param hidden Supply `true` to make the layer hidden, or `false` to make it
//! non-hidden.
void layer_set_hidden(Layer *layer, bool hidden);

//! Gets the visibility of the layer.
//! @param layer The layer for which to get the visibility
//! @return True if the layer is hidden, false if it is not hidden.
bool layer_get_hidden(const Layer *layer);

//! Sets whether clipping is enabled for the layer. If enabled, whatever the layer _and
//! its children_ will draw using their `.update_proc` callbacks, will be clipped by the
//! this layer's frame.
//! If the clipping has changed, \ref layer_mark_dirty() will be called automatically.
//! @param layer The layer for which to set the clipping property
//! @param clips Supply `true` to make the layer clip to its frame, or `false`
//! to make it non-clipping.
void layer_set_clips(Layer *layer, bool clips);

//! Gets whether clipping is enabled for the layer.  If enabled, whatever the layer _and
//! its children_ will draw using their `.update_proc` callbacks, will be clipped by the
//! this layer's frame.
//! @param layer The layer for which to get the clipping property
//! @return True if clipping is enabled for the layer, false if clipping is not enabled for
//! the layer.
bool layer_get_clips(const Layer *layer);

//! Gets the data from a layer that has been created with an extra data region.
//! @param layer The layer to get the data region from.
//! @return A void pointer to the data region.
void* layer_get_data(const Layer *layer);

//! @} // group Layer

//! @addtogroup Window
//! \brief The basic building block of the user interface
//!
//! Windows are the top-level elements in the UI hierarchy and the basic building blocks for a Pebble
//! UI. A single window is always displayed at a time on Pebble, with the exception of when animating
//! from one window to the other, which, in that case, is managed by the window stack. You can stack
//! windows on top of each other, but only the topmost window will be visible.
//!
//! Users wearing a Pebble typically interact with the content and media displayed in a window, clicking
//! and pressing buttons on the watch, depending on what they see and wish to respond to in a window.
//!
//! Windows serve to display a hierarchy of layers on the screen and handle user input. When a window is
//! visible, its root Layer (and all its child layers) are drawn onto the screen automatically.
//!
//! You need a window, which always fills the entire screen, to display images, text, and graphics in
//! your Pebble app. A layer by itself doesn’t display on Pebble; it must be in the current window’s
//! layer hierarchy to be visible.
//!
//! The Window Stack serves as the global manager of what window is presented and makes sure that input
//! events are forwarded to the topmost window.
//!
//! Refer to the \htmlinclude UiFramework.html (chapter "Window") for a conceptual
//! overview of Window, the Window Stack and relevant code examples.
//! @{

struct Window;
typedef struct Window Window;

//! Function signature for a handler that deals with transition events of a window.
//! @see WindowHandlers
//! @see \ref window_set_window_handlers()
typedef void (*WindowHandler)(struct Window *window);

//! WindowHandlers
//! These handlers are called by the \ref WindowStack as windows get pushed on / popped:
//! * `load`:     called when the window is pushed to the screen when it's not loaded.
//!              This is a good moment to do the layout of the window.
//! * `appear`:   called when the window comes on the screen (again). E.g. when
//!               second-top-most window gets revealed (again) after popping the top-most
//!               window, but also when the window is pushed for the first time. This is a
//!               good moment to start timers related to the window, or reset the UI, etc.
//! * `disappear`:called when the window leaves the screen, e.g. when another window
//!               is pushed, or this window is popped. Good moment to stop timers related
//!               to the window.
//! * `unload`:   called when the window is deinited, but could be used in the future to
//!               free resources bound to windows that are not on screen.
//!
//! All these handlers use \ref WindowHandler as their function signature.
//! @see \ref window_set_window_handlers()
//! @see \ref WindowStack
typedef struct WindowHandlers {
  WindowHandler load;
  WindowHandler appear;
  WindowHandler disappear;
  WindowHandler unload;
} WindowHandlers;

//! Creates a new Window on the heap and initalizes it with the default values.
//!
//! * Background color : `GColorWhite`
//! * Root layer's `update_proc` : function that fills the window's background using `background_color`.
//! * Full screen : no
//! * `click_config_provider` : `NULL`
//! * `window_handlers` : all `NULL`
//! * `status_bar_icon` : `NULL` (none)
//! @return A pointer to the window. `NULL` if the window could not
//! be created
Window* window_create(void);

//! Destroys a Window previously created by window_create.
void window_destroy(Window* window);

//! Sets the click configuration provider callback function on the window.
//! This will automatically setup the input handlers of the window as well to use
//! the click recognizer subsystem.
//! @param window The window for which to set the click config provider
//! @param click_config_provider The callback that will be called to configure the click recognizers with the window
//! @see Clicks
//! @see ClickConfigProvider
void window_set_click_config_provider(Window *window, ClickConfigProvider click_config_provider);

//! Same as window_set_click_config_provider(), but will assign a custom context pointer
//! (instead of the window pointer) that will be passed into the ClickHandler click event handlers.
//! @param window The window for which to set the click config provider
//! @param click_config_provider The callback that will be called to configure the click recognizers with the window
//! @param context Pointer to application specific data that will be passed to the click configuration provider callback.
//! @see Clicks
//! @see window_set_click_config_provider
void window_set_click_config_provider_with_context(Window *window, ClickConfigProvider click_config_provider, void *context);

//! Gets the current click configuration provider of the window.
//! @param window The window for which to get the click config provider
ClickConfigProvider window_get_click_config_provider(const Window *window);

//! Sets the window handlers of the window.
//! These handlers get called e.g. when the user enters or leaves the window.
//! @param window The window for which to set the window handlers
//! @param handlers The handlers for the specified window
//! @see \ref WindowHandlers
void window_set_window_handlers(Window *window, WindowHandlers handlers);

//! Gets the root Layer of the window.
//! The root layer is the layer at the bottom of the layer hierarchy for this window.
//! It is the window's "canvas" if you will. By default, the root layer only draws
//! a solid fill with the window's background color.
//! @param window The window for which to get the root layer
//! @return The window's root layer
struct Layer* window_get_root_layer(const Window *window);

//! Sets the background color of the window, which is drawn automatically by the
//! root layer of the window.
//! @param window The window for which to set the background color
//! @param background_color The new background color
//! @see \ref window_get_root_layer()
void window_set_background_color(Window *window, GColor background_color);

//! Sets whether or not the window is fullscreen, consequently hiding the sytem status bar.
//! @note This needs to be called before pushing a window to the window stack.
//! @param window The window for which to set its full-screen property
//! @param enabled True to make the window full-screen or false to leave space for the system status bar.
//! @see \ref window_get_fullscreen()
void window_set_fullscreen(Window *window, bool enabled);

//! Gets whether the window is full-screen, consequently hiding the sytem status bar.
//! @param window The window for which to get its full-screen property
//! @return True if the window is marked as fullscreen, false if it is not marked as fullscreen.
bool window_get_fullscreen(const Window *window);

//! Assigns an icon (max. 16x16 pixels) that can be displayed in the system status bar.
//! When no icon is assigned, the icon of the previous window on the window stack is used.
//! @note This needs to be called before pushing a window to the window stack.
//! @param window The window for which to set the status bar icon
//! @param icon The new status bar icon
void window_set_status_bar_icon(Window *window, const GBitmap *icon);

//! Gets whether the window has been loaded.
//! If a window is loaded, its `.load` handler has been called (and the `.unload` handler
//! has not been called since).
//! @return true if the window is currently loaded or false if not.
//! @param window The window to query its loaded status
//! @see \ref WindowHandlers
bool window_is_loaded(Window *window);

//! Sets a pointer to developer-supplied data that the window uses, to
//! provide a means to access the data at later times in one of the window event handlers.
//! @see window_get_user_data
//! @param window The window for which to set the user data
void window_set_user_data(Window *window, void *data);

//! Gets the pointer to developer-supplied data that was previously
//! set using window_set_user_data().
//! @see window_set_user_data
//! @param window The window for which to get the user data
void* window_get_user_data(const Window *window);

//! Subscribe to single click events. A single click is detected every time "repeat_interval_ms" has been reached.
//! @note Must be called from within the \ref ClickConfigProvider.
//! @note \ref window_single_click_subscribe() and \ref window_single_repeating_click_subscribe() conflict, and cannot both be used on the same button.
//! @param button_id The button events to subscribe to. @see ButtonId.
//! @param context Optional pointer to application specific data that will be passed into the handler.
//! @see Clicks
//! A value of 0ms means "no repeat timer". The minimum is 30ms, and values below will be disregarded.
//! @param handler The \ref ClickHandler to fire on this event.
//! @note When there is a multi_click and/or long_click setup, there will be a delay pending before the single click
//! handler will get fired. On the other hand, when there is no multi_click nor long_click setup, the single click handler will fire directly on button down.
//! @see window_single_repeating_click_subscribe
void window_single_click_subscribe(ButtonId button_id, ClickHandler handler);

//! Subscribe to single click event, with a repeat interval. A single click is detected every time "repeat_interval_ms" has been reached.
//! @note Must be called from within the \ref ClickConfigProvider.
//! @note \ref window_single_click_subscribe() and \ref window_single_repeating_click_subscribe() conflict, and cannot both be used on the same button.
//! @note The back button cannot be overridden with a repeating click.
//! @param repeat_interval_ms When holding down, how many milliseconds before the handler is fired again.
//! @note If there is a long-click handler subscribed on this button, `repeat_interval_ms` will not be used.
//! @see window_single_click_subscribe
void window_single_repeating_click_subscribe(ButtonId button_id, uint16_t repeat_interval_ms, ClickHandler handler);

//! Subscribe to multi click events.
//! @note Must be called from within the \ref ClickConfigProvider.
//! @param button_id The button events to subscribe to. @see ButtonId.
//! @param min Minimum number of clicks before handler is fired. Defaults to 2.
//! @param max Maximum number of clicks after which the click counter is reset. A value of 0 means use "min" also as "max".
//! @param timeout The delay after which a sequence of clicks is considered finished, and the click counter is reset. A value of 0 means to use the system default 300ms.
//! @param last_click_only Defaults to false. When true, only the for the last multi-click the handler is called.
//! @param handler The \ref ClickHandler to fire on this event. Fired for multi-clicks, as "filtered" by the `last_click_only`, `min`, and `max` parameters.
void window_multi_click_subscribe(ButtonId button_id, uint8_t min_clicks, uint8_t max_clicks, uint16_t timeout, bool last_click_only, ClickHandler handler);

//! Subscribe to long click events.
//! @note Must be called from within the \ref ClickConfigProvider.
//! @note The back button cannot be overridden with a long click.
//! @param button_id The button events to subscribe to. @see ButtonId.
//! @param delay_ms Milliseconds after which "handler" is fired. A value of 0 means to use the system default 500ms.
//! @param down_handler The \ref ClickHandler to fire as soon as the button has been held for `delay_ms`. This may be NULL to have no down handler.
//! @param up_handler The \ref ClickHandler to fire on the release of a long click. This may be NULL to have no up handler.
void window_long_click_subscribe(ButtonId button_id, uint16_t delay_ms, ClickHandler down_handler, ClickHandler up_handler);

//! Subscribe to raw click events.
//! @note Must be called from within the \ref ClickConfigProvider.
//! @note The back button cannot be overridden with a raw click.
//! @param button_id The button events to subscribe to. @see ButtonId.
//! @param down_handler The \ref ClickHandler to fire as soon as the button has been pressed. This may be NULL to have no down handler.
//! @param up_handler The \ref ClickHandler to fire on the release of the button. This may be NULL to have no up handler.
//! @param context If this context is not NULL, it will override the general context.
void window_raw_click_subscribe(ButtonId button_id, ClickHandler down_handler, ClickHandler up_handler, void *context);

//! Set the context that will be passed to handlers for the given button's events.
//! @note Must be called from within the \ref ClickConfigProvider.
//! @param button_id The button to set the context for.
//! @param context Pointer to data that will be passed as context.
void window_set_click_context(ButtonId button_id, void *context);

//! @} // group Window

//! @addtogroup WindowStack Window Stack
//! \brief The multiple window manager
//!
//! In Pebble OS, the window stack serves as the global manager of what window is presented,
//! ensuring that input events are forwarded to the topmost window.
//! The navigation model of Pebble centers on the concept of a vertical “stack” of windows, similar
//! to mobile app interactions.
//!
//! In working with the Window Stack API, the basic operations include push and pop. When an app wants to
//! display a new window, it pushes a new window onto the stack. This appears like a window sliding in
//! from the right. As an app is closed, the window is popped off the stack and disappears.
//!
//! For more complicated operations, involving multiple windows, you can determine which windows reside
//! on the stack, using window_stack_contains_window() and remove any specific window, using window_stack_remove().
//!
//! Refer to the \htmlinclude UiFramework.html (chapter "Window Stack") for a conceptual overview
//! of the window stack and relevant code examples.
//!
//! Also see the \ref WindowHandlers of a \ref Window for the callbacks that can be added to a window
//! in order to act upon window stack transitions.
//!
//! @{

//! Pushes the given window on the window navigation stack,
//! on top of the current topmost window of the app.
//! @param window The window to push on top
//! @param animated Pass in `true` to animate the push using a sliding animation,
//! or `false` to skip the animation.
void window_stack_push(Window *window, bool animated);

//! Pops the topmost window on the navigation stack
//! @param animated See \ref window_stack_remove()
//! @return The window that is popped, or NULL if there are no windows to pop.
Window* window_stack_pop(bool animated);

//! Pops all windows.
//! See \ref window_stack_remove() for a description of the `animated` parameter and notes.
void window_stack_pop_all(const bool animated);

//! Removes a given window from the window stack
//! that belongs to the app task.
//! @note If there are no windows for the app left on the stack, the app
//! will be killed by the system, shortly. To avoid this, make sure
//! to push another window shortly after or before removing the last window.
//! @param window The window to remove. If the window is NULL or if it
//! is not on the stack, this function is a no-op.
//! @param animated Pass in `true` to animate the removal of the window using
//! a side-to-side sliding animation to reveal the next window.
//! This is only used in case the window happens to be on top of the window
//! stack (thus visible).
//! @return True if window was successfully removed, false otherwise.
bool window_stack_remove(Window *window, bool animated);

//! Gets the topmost window on the stack that belongs to the app.
//! @return The topmost window on the stack that belongs to the app or
//! NULL if no app window could be found.
Window* window_stack_get_top_window(void);

//! Checks if the window is on the window stack
//! @param window The window to look for on the window stack
//! @return true if the window is currently on the window stack.
bool window_stack_contains_window(Window *window);

//! @} // group WindowStack

//! @addtogroup Animation
//!   \brief Abstract framework to create arbitrary animations
//!
//! The Animation framework provides your Pebble app with an base layer to create arbitrary
//! animations. The simplest way to work with animations is to use the layer frame
//! \ref PropertyAnimation, which enables you to move a Layer around on the screen.
//! Using animation_set_implementation(), you can implement a custom animation.
//!
//! Refer to the \htmlinclude UiFramework.html (chapter "Animation") for a conceptual overview
//! of the animation framework and on how to write custom animations.
//! @{

struct Animation;
typedef struct Animation Animation;

#define NUM_ANIMATION_CURVE 4

//! Values that are used to indicate the different animation curves,
//! which determine the speed at which the animated value(s) change(s).
typedef enum {
  //! Linear curve: the velocity is constant.
  AnimationCurveLinear = 0,
  //! Bicubic ease-in: accelerate from zero velocity
  AnimationCurveEaseIn = 1,
  //! Bicubic ease-in: decelerate to zero velocity
  AnimationCurveEaseOut = 2,
  //! Bicubic ease-in-out: accelerate from zero velocity, decelerate to zero velocity
  AnimationCurveEaseInOut = 3,
  //! Number of available AnimationCurve types
  NumAnimationCurve = NUM_ANIMATION_CURVE
} AnimationCurve;

//! Creates a new Animation on the heap and initalizes it with the default values.
//!
//! * Duration: 250ms,
//! * Curve: \ref AnimationCurveEaseInOut (ease-in-out),
//! * Delay: 0ms,
//! * Handlers: `{NULL, NULL}` (none),
//! * Context: `NULL` (none),
//! * Implementation: `NULL` (no implementation),
//! * Scheduled: no
//! @return A pointer to the animation. `NULL` if the animation could not
//! be created
struct Animation * animation_create(void);

//! Destroys an Animation previously created by animation_create.
void animation_destroy(struct Animation *animation);

//! Constant to indicate "infinite" duration.
//! This can be used with \ref animation_set_duration() to indicate that the animation
//! should run indefinitely.
//! This is useful when implementing for example a frame-by-frame simulation that does not
//! have a clear ending (e.g. a game).
//! @note Note that `time_normalized` parameter that is passed
//! into the `.update` implementation is meaningless in when an infinite duration is used.
#define ANIMATION_DURATION_INFINITE ((uint32_t) ~0)

//! The normalized time at the start of the animation.
#define ANIMATION_NORMALIZED_MIN 0

//! The normalized time at the end of the animation.
#define ANIMATION_NORMALIZED_MAX 65535

//! Sets the time in milliseconds that an animation takes from start to finish.
//! @param animation The animation for which to set the duration.
//! @param duration_ms The duration in milliseconds of the animation. This excludes
//! any optional delay as set using \ref animation_set_delay().
void animation_set_duration(struct Animation *animation, uint32_t duration_ms);

//! Sets an optional delay for the animation.
//! @param animation The animation for which to set the delay.
//! @param delay_ms The delay in milliseconds that the animation system should
//! wait from the moment the animation is scheduled to starting the animation.
void animation_set_delay(struct Animation *animation, uint32_t delay_ms);

//! Sets the animation curve for the animation.
//! @param animation The animation for which to set the curve.
//! @param curve The type of curve.
//! @see AnimationCurve
//! @note It is up to the implementation of the animation to actually use the curve.
//! Because a curve type is often used for animations, it is included in the animation base layer.
void animation_set_curve(struct Animation *animation, AnimationCurve curve);

//! The function pointer type of the handler that will be called when an animation is started,
//! just before updating the first frame of the animation.
//! @param animation The animation that was started.
//! @param context The pointer to custom, application specific data, as set using \ref animation_set_handlers()
//! @note This is called after any optional delay as set by \ref animation_set_delay() has expired.
//! @see animation_set_handlers
typedef void (*AnimationStartedHandler)(struct Animation *animation, void *context);

//! The function pointer type of the handler that will be called when the animation is stopped.
//! @param animation The animation that was stopped.
//! @param finished True if the animation was stopped because it was finished normally,
//! or False if the animation was stopped prematurely, because it was unscheduled before finishing.
//! @param context The pointer to custom, application specific data, as set using \ref animation_set_handlers()
//! @see animation_set_handlers
typedef void (*AnimationStoppedHandler)(struct Animation *animation, bool finished, void *context);

//! The handlers that will get called when an animation starts and stops.
//! See documentation with the function pointer types for more information.
//! @see animation_set_handlers
typedef struct AnimationHandlers {
  //! The handler that will be called when an animation is started.
  AnimationStartedHandler started;
  //! The handler that will be called when an animation is stopped.
  AnimationStoppedHandler stopped;
} AnimationHandlers;

//! Sets the callbacks for the animation.
//! Often an application needs to run code at the start or at the end of an animation.
//! Using this function is possible to register callback functions with an animation,
//! that will get called at the start and end of the animation.
//! @param animation The animation for which to set up the callbacks.
//! @param callbacks The callbacks.
//! @param context A pointer to application specific data, that will be passed as an argument by
//! the animation subsystem when a callback is called.
void animation_set_handlers(struct Animation *animation, AnimationHandlers callbacks, void *context);

//! Gets the application-specific callback context of the animation.
//! This `void` pointer is passed as an argument when the animation system calls AnimationHandlers callbacks.
//! The context pointer can be set to point to any application specific data using \ref animation_set_handlers().
//! @param animation The animation.
//! @see animation_set_handlers
void *animation_get_context(struct Animation *animation);

//! Schedules the animation. Call this once after configuring an animation to get it to
//! start running.
//!
//! If the animation's implementation has a `.setup` callback it will get called before
//! this function returns.
//!
//! @note If the animation was already scheduled,
//! it will first unschedule it and then re-schedule it again.
//! Note that in that case, the animation's `.stopped` handler, the implementation's
//! `.teardown` and `.setup` will get called, due to the unscheduling and scheduling.
//! @param animation The animation to schedule.
//! @see \ref animation_unschedule()
void animation_schedule(struct Animation *animation);

//! Unschedules the animation, which in effect stops the animation.
//! @param animation The animation to unschedule.
//! @note If the animation was not yet finished, unscheduling it will
//! cause its `.stopped` handler to get called, with the "finished" argument set to false.
//! @see \ref animation_schedule()
void animation_unschedule(struct Animation *animation);

//! Unschedules all animations of the application.
//! @see animation_unschedule
void animation_unschedule_all(void);

//! @return True if the animation was scheduled, or false if it was not.
//! @note An animation will be scheduled when it is running and not finished yet.
//! An animation that has finished is automatically unscheduled.
//! @param animation The animation for which to get its scheduled state.
//! @see animation_schedule
//! @see animation_unschedule
bool animation_is_scheduled(struct Animation *animation);

///////////////////
typedef struct Animation {
  ListNode list_node;
  const struct AnimationImplementation *implementation;
  AnimationHandlers handlers; // FIXME: make this const AnimationHandlers *, like the implementation field.
  void *context;
  //! Absolute time when the animation got scheduled, in ms since system start.
  uint32_t abs_start_time_ms;
  uint32_t delay_ms;
  uint32_t duration_ms;
  AnimationCurve curve:3;
  bool is_completed:1;
} Animation;

//! Pointer to function that (optionally) prepares the animation for running.
//! This callback is called when the animation is added to the scheduler.
//! @param animation The animation that needs to be set up.
//! @see animation_schedule
//! @see AnimationTeardownImplementation
typedef void (*AnimationSetupImplementation)(struct Animation *animation);

//! Pointer to function that updates the animation according to the given normalized time.
//! This callback will be called repeatedly by the animation scheduler whenever the animation needs to be updated.
//! @param animation The animation that needs to update; gets passed in by the animation framework.
//! @param time_normalized The current normalized time; gets passed in by the animation framework for each animation frame.
//! This is a value between \ref ANIMATION_NORMALIZED_MIN and \ref ANIMATION_NORMALIZED_MAX.
//! At the start of the animation, the value will be \ref ANIMATION_NORMALIZED_MIN.
//! At the end of the animation, the value will be \ref ANIMATION_NORMALIZED_MAX.
//! For each frame during the animation, the value will be the running time, mapped linearly between
//! \ref ANIMATION_NORMALIZED_MIN and \ref ANIMATION_NORMALIZED_MAX.
//! For example, say an animation was scheduled at t = 1.0s, has a delay of 1.0s and a duration of 2.0s.
//! Then the .update callback will get called on t = 2.0s with time_normalized = \ref ANIMATION_NORMALIZED_MIN.
//! For each frame thereafter until t = 4.0s, the update callback will get called where time_normalized is
//! (\ref ANIMATION_NORMALIZED_MIN + (((\ref ANIMATION_NORMALIZED_MAX - \ref ANIMATION_NORMALIZED_MIN) * t) / duration)).
typedef void (*AnimationUpdateImplementation)(struct Animation *animation, const uint32_t time_normalized);

//! Pointer to function that (optionally) cleans up the animation.
//! This callback is called when the animation is removed from the scheduler.
//! In case the `.setup` implementation
//! allocated any memory, this is a good place to release that memory again.
//! @param animation The animation that needs to be teared down.
//! @see animation_unschedule
//! @see AnimationSetupImplementation
typedef void (*AnimationTeardownImplementation)(struct Animation *animation);

//! The 3 callbacks that implement a custom animation.
//! Only the `.update` callback is mandatory, `.setup` and `.teardown` are optional.
//! See the documentation with the function pointer typedefs for more information.
//!
//! @note The `.setup` callback is called immediately after scheduling the animation,
//! regardless if there is a delay set for that animation using \ref animation_set_delay().
//!
//! The diagram below illustrates the order in which callbacks can be expected to get called
//! over the life cycle of an animation. It also illustrates where the implementation of
//! different animation callbacks are intended to be “living”.
//! ![](animations.png)
//!
//! @see AnimationSetupImplementation
//! @see AnimationUpdateImplementation
//! @see AnimationTeardownImplementation
typedef struct AnimationImplementation {
  //! Called by the animation system when an animation is scheduled, to prepare it for running.
  //! This callback is optional and can be left `NULL` when not needed.
  AnimationSetupImplementation setup;
  //! Called by the animation system when the animation needs to calculate the next animation frame.
  //! This callback is mandatory and should not be left `NULL`.
  AnimationUpdateImplementation update;
  //! Called by the animation system when an animation is unscheduled, to clean up after it has run.
  //! This callback is optional and can be left `NULL` when not needed.
  AnimationTeardownImplementation teardown;
} AnimationImplementation;

//! Sets the implementation of the custom animation.
//! When implementing custom animations, use this function to specify what functions need to be called to
//! for the setup, frame update and teardown of the animation.
//! @param animation The animation for which to set the implementation.
//! @param implementation The structure with function pointers to the implementation of the setup, update and teardown functions.
//! @see AnimationImplementation
void animation_set_implementation(struct Animation *animation, const AnimationImplementation *implementation);

//! @addtogroup PropertyAnimation
//! \brief Concrete animations to move a layer around over time
//!
//! Actually, property animations do more than just moving a Layer around over time.
//! PropertyAnimation is a concrete class of animations and is built around the Animation subsystem,
//! which covers anything timing related, but does not move anything around.
//! A ProperyAnimation animates a "property" of a "subject".
//!
//! <h3>Animating a Layer's frame property</h3>
//! Currently there is only one specific type of property animation offered off-the-shelf, namely
//! one to change the frame (property) of a layer (subject), see \ref property_animation_create_layer_frame().
//!
//! <h3>Implementing a custom PropertyAnimation</h3>
//! It is fairly simple to create your own variant of a PropertyAnimation.
//!
//! Please refer to \htmlinclude UiFramework.html (chapter "Property Animations") for a conceptual overview
//! of the animation framework and make sure you understand the underlying \ref Animation, in case you are
//! not familiar with it, before trying to implement a variation on PropertyAnimation.
//!
//! To implement a custom property animation, use \ref property_animation_create() and provide a function
//! pointers to the accessors (getter and setter) and setup, update and teardown callbacks in the implementation argument.
//! Note that the type of property to animate with \ref PropertyAnimation is limited to int16_t, GPoint or GRect.
//!
//! For each of these types, there are implementations provided
//! for the necessary `.update` handler of the animation: see \ref property_animation_update_int16(),
//! \ref property_animation_update_gpoint() and \ref property_animation_update_grect().
//! These update functions expect the `.accessors` to conform to the following interface:
//! Any getter needs to have the following function signature: `__type__ getter(void *subject);`
//! Any setter needs to have to following function signature: `void setter(void *subject, __type__ value);`
//! See \ref Int16Getter, \ref Int16Setter, \ref GPointGetter, \ref GPointSetter, \ref GRectGetter, \ref GRectSetter
//! for the typedefs that accompany the update fuctions.
//!
//! \code{.c}
//! static const PropertyAnimationImplementation my_implementation = {
//!   .base = {
//!     // using the "stock" update callback:
//!     .update = (AnimationUpdateImplementation) property_animation_update_gpoint,
//!   },
//!   .accessors = {
//!     // my accessors that get/set a GPoint from/onto my subject:
//!     .setter = { .gpoint = my_layer_set_corner_point, },
//!     .getter = { .gpoint = (const GPointGetter) my_layer_get_corner_point, },
//!   },
//! };
//! static PropertyAnimation* s_my_animation_ptr = NULL;
//! static GPoint s_to_point = GPointZero;
//! ...
//! // Use NULL as 'from' value, this will make the animation framework call the getter
//! // to get the current value of the property and use that as the 'from' value:
//! s_my_animation_ptr = property_animation_create(&my_implementation, my_layer, NULL, &s_to_point);
//! animation_schedule(s_my_animation_ptr->animation);
//! \endcode
//! @{

struct PropertyAnimationAccessors;
typedef struct PropertyAnimationAccessors PropertyAnimationAccessors;

struct PropertyAnimationImplementation;
typedef struct PropertyAnimationImplementation PropertyAnimationImplementation;

typedef struct PropertyAnimation {
  //! The "inherited" state from the "base class", \ref Animation.
  Animation animation;
  //! The values of the property that the animation should animated from and to.
  struct {
    //! The value of the property that the animation should animate to.
    //! When the animation completes, this value will be the final value that is set.
    union {
      //! Valid when the property being animated is of type GRect
      GRect grect;
      //! Valid when the property being animated is of type GPoint
      GPoint gpoint;
      //! Valid when the property being animated is of type int16_t
      int16_t int16;
    } to;
    //! The value of the property that the animation should animate to.
    //! When the animation starts, this value will be the initial value that is set.
    union {
      //! Valid when the property being animated is of type GRect
      GRect grect;
      //! Valid when the property being animated is of type GPoint
      GPoint gpoint;
      //! Valid when the property being animated is of type int16_t
      int16_t int16;
    } from;
  } values; //!< See detail table
  void *subject; //!< The subject of the animation of which the property should be animated.
} PropertyAnimation;

//! Convenience function to create and initialize a property animation that animates the frame of a Layer.
//! It sets up the PropertyAnimation to use \ref layer_set_frame() and \ref layer_get_frame()
//! as accessors and uses the `layer` parameter as the subject for the animation.
//! The same defaults are used as with \ref animation_create().
//! @param property_animation The property animation to initialize and set up
//! @param layer the layer that will be animated
//! @param to_frame the frame that the layer should animate to
//! @param from_frame the frame that the layer should animate from
//! @note Pass in `NULL` as one of the frame arguments to have it set automatically to the layer's current frame.
//! This will result in a call to \ref layer_get_frame() to get the current frame of the layer.
//! @return A pointer to the property animation. `NULL` if animation could not
//! be created
struct PropertyAnimation* property_animation_create_layer_frame(struct Layer *layer, GRect *from_frame, GRect *to_frame);

//! Creates a new PropertyAnimation on the heap and and initializes it with the specified values.
//! The same defaults are used as with \ref animation_create().
//! If the `from_value` or the `to_value` is `NULL`, the getter accessor will be called to get the current value
//! of the property and be used instead.
//! @param implementation Pointer to the implementation of the animation. In most cases, it makes sense to pass in
//! a `static const` struct pointer.
//! @param subject Pointer to the "subject" being animated. This will be passed in when the getter/setter accessors are called,
//! see \ref PropertyAnimationAccessors, \ref GPointSetter, and friends. The value of this pointer will be copied into
//! the `.subject` field of the PropertyAnimation struct.
//! @param from_value Pointer to the value that the subject should animate from
//! @param to_value Pointer to the value that the subject should animate to
//! @note Pass in `NULL` as one of the value arguments to have it set automatically to the subject's current property value,
//! as returned by the getter function. Also note that passing in `NULL` for both `from_value` and `to_value`, will
//! result in the animation having the same from- and to- values, effectively not doing anything.
//! @return A pointer to the property animation. `NULL` if animation could not
//! be created
struct PropertyAnimation* property_animation_create(const struct PropertyAnimationImplementation *implementation, void *subject, void *from_value, void *to_value);

//! Free a dynamically allocated property animation
//! @param property_animation The property animation to be freed.
void property_animation_destroy(struct PropertyAnimation* property_animation);

//! Default update callback for a property animations to update a property of type int16_t.
//! Assign this function to the `.base.update` callback field of your PropertyAnimationImplementation,
//! in combination with a `.getter` and `.setter` accessors of types \ref Int16Getter and \ref Int16Setter.
//! The implementation of this function will calculate the next value of the animation and call the
//! setter to set the new value upon the subject.
//! @param property_animation The property animation for which the update is requested.
//! @param time_normalized The current normalized time. See \ref AnimationUpdateImplementation
//! @note This function is not supposed to be called "manually", but will be called automatically when the animation
//! is being run.
void property_animation_update_int16(struct PropertyAnimation *property_animation, const uint32_t time_normalized);

//! Default update callback for a property animations to update a property of type GPoint.
//! Assign this function to the `.base.update` callback field of your PropertyAnimationImplementation,
//! in combination with a `.getter` and `.setter` accessors of types \ref GPointGetter and \ref GPointSetter.
//! The implementation of this function will calculate the next point of the animation and call the
//! setter to set the new point upon the subject.
//! @param property_animation The property animation for which the update is requested.
//! @param time_normalized The current normalized time. See \ref AnimationUpdateImplementation
//! @note This function is not supposed to be called "manually", but will be called automatically when the animation
//! is being run.
void property_animation_update_gpoint(struct PropertyAnimation *property_animation, const uint32_t time_normalized);

//! Default update callback for a property animations to update a property of type GRect.
//! Assign this function to the `.base.update` callback field of your PropertyAnimationImplementation,
//! in combination with a `.getter` and `.setter` accessors of types \ref GRectGetter and \ref GRectSetter.
//! The implementation of this function will calculate the next rectangle of the animation and call the
//! setter to set the new rectangle upon the subject.
//! @param property_animation The property animation for which the update is requested.
//! @param time_normalized The current normalized time. See \ref AnimationUpdateImplementation
//! @note This function is not supposed to be called "manually", but will be called automatically when the animation
//! is being run.
void property_animation_update_grect(struct PropertyAnimation *property_animation, const uint32_t time_normalized);

//! Work-around for function pointer return type GPoint avoid
//! tripping the pre-processor to use the equally named GPoint define
typedef GPoint GPointReturn;

//! Work-around for function pointer return type GRect avoid
//! tripping the pre-processor to use the equally named GRect define
typedef GRect GRectReturn;

//! Function signature of a setter function to set a property of type int16_t onto the subject.
//! @see \ref property_animation_update_int16()
//! @see \ref PropertyAnimationAccessors
typedef void (*Int16Setter)(void *subject, int16_t int16);

//! Function signature of a getter function to get the current property of type int16_t of the subject.
//! @see \ref property_animation_create()
//! @see \ref PropertyAnimationAccessors
typedef int16_t (*Int16Getter)(void *subject);

//! Function signature of a setter function to set a property of type GPoint onto the subject.
//! @see \ref property_animation_update_gpoint()
//! @see \ref PropertyAnimationAccessors
typedef void (*GPointSetter)(void *subject, GPoint gpoint);

//! Function signature of a getter function to get the current property of type GPoint of the subject.
//! @see \ref property_animation_create()
//! @see \ref PropertyAnimationAccessors
typedef GPointReturn (*GPointGetter)(void *subject);

//! Function signature of a setter function to set a property of type GRect onto the subject.
//! @see \ref property_animation_update_grect()
//! @see \ref PropertyAnimationAccessors
typedef void (*GRectSetter)(void *subject, GRect grect);

//! Function signature of a getter function to get the current property of type GRect of the subject.
//! @see \ref property_animation_create()
//! @see \ref PropertyAnimationAccessors
typedef GRectReturn (*GRectGetter)(void *subject);

//! Data structure containing the setter and getter function pointers that the property animation should use.
//! The specified setter function will be used by the animation's update callback. <br/> Based on the
//! type of the property (int16_t, GPoint or GRect), the accompanying update callback should be used,
//! see \ref property_animation_update_int16(), \ref property_animation_update_gpoint() and
//! \ref property_animation_update_grect(). <br/>
//! The getter function is used when the animation is initialized, to assign the current value of the
//! subject's property as "from" or "to" value, see \ref
//! property_animation_create().
typedef struct PropertyAnimationAccessors {
  //! Function pointer to the implementation of the function that __sets__ the updated property
  //! value. This function will be called repeatedly for each animation frame.
  //! @see PropertyAnimationAccessors
  union {
    //! Use if the property to animate is of int16_t type
    Int16Setter int16;
    //! Use if the property to animate is of GPoint type
    GPointSetter gpoint;
    //! Use if the property to animate is of GRect type
    GRectSetter grect;
  } setter;
  //! Function pointer to the implementation of the function that __gets__ the current property
  //! value. This function will be called during \ref property_animation_create(), to get the current
  //! property value, in case the `from_value` or `to_value` argument is `NULL`.
  //! @see PropertyAnimationAccessors
  union {
    //! Use if the property to animate is of int16_t type
    Int16Getter int16;
    //! Use if the property to animate is of GPoint type
    GPointGetter gpoint;
    //! Use if the property to animate is of GRect type
    GRectGetter grect;
  } getter;
} PropertyAnimationAccessors;

//! Data structure containing a collection of function pointers that form the implementation of the
//! property animation.
//! See the code example at the top (\ref PropertyAnimation).
typedef struct PropertyAnimationImplementation {
  //! The "inherited" fields from the Animation "base class".
  AnimationImplementation base;
  //! The accessors to set/get the property to be animated.
  PropertyAnimationAccessors accessors;
} PropertyAnimationImplementation;

//! @} // group PropertyAnimation

//! @} // group Animation

//! @addtogroup Layer
//! @{

//! @addtogroup TextLayer
//! \brief Layer that displays and formats a text string.
//!
//! ![](text_layer.png)
//! The geometric information (bounds, frame) of the Layer
//! is used as the "box" in which the text is drawn. The \ref TextLayer also has a number of other properties
//! that influence how the text is drawn. Most important of these properties are: a pointer to the string to
//! draw itself, the font, the text color, the background color of the layer, the overflow mode and
//! alignment of the text inside the layer.
//! @see Layer
//! @see TextDrawing
//! @see Fonts
//! @{

struct TextLayer;
typedef struct TextLayer TextLayer;

//! Creates a new TextLayer on the heap and initializes it with the default values.
//!
//! * Font: Raster Gothic 14-point Boldface (system font)
//! * Text Alignment: \ref GTextAlignmentLeft
//! * Text color: \ref GColorBlack
//! * Background color: \ref GColorWhite
//! * Clips: `true`
//! * Hidden: `false`
//! * Caching: `false`
//!
//! The text layer is automatically marked dirty after this operation.
//! @param frame The frame with which to initialze the TextLayer
//! @return A pointer to the TextLayer. `NULL` if the TextLayer could not
//! be created
TextLayer* text_layer_create(GRect frame);

//! Destroys a TextLayer previously created by text_layer_create.
void text_layer_destroy(TextLayer* text_layer);

//! Gets the "root" Layer of the text layer, which is the parent for the sub-
//! layers used for its implementation.
//! @param text_layer Pointer to the TextLayer for which to get the "root" Layer
//! @return The "root" Layer of the text layer.
Layer* text_layer_get_layer(TextLayer *text_layer);

//! Sets the pointer to the string where the TextLayer is supposed to find the text
//! at a later point in time, when it needs to draw itself.
//! @param text_layer The TextLayer of which to set the text
//! @param text The new text to set onto the TextLayer. This must be a null-terminated and valid UTF-8 string!
//! @note The string is not copied, so its buffer most likely cannot be stack allocated,
//! but is recommended to be a buffer that is long-lived, at least as long as the TextLayer
//! is part of a visible Layer hierarchy.
//! @see text_layer_get_text
void text_layer_set_text(TextLayer *text_layer, const char *text);

//! Gets the pointer to the string that the TextLayer is using.
//! @param text_layer The TextLayer for which to get the text
//! @see text_layer_set_text
const char* text_layer_get_text(TextLayer *text_layer);

//! Sets the background color of bounding box that will be drawn behind the text
//! @param text_layer The TextLayer of which to set the background color
//! @param color The new \ref GColor to set the background to
//! @see text_layer_set_text_color
void text_layer_set_background_color(TextLayer *text_layer, GColor color);

//! Sets the color of text that will be drawn
//! @param text_layer The TextLayer of which to set the text color
//! @param color The new \ref GColor to set the text color to
//! @see text_layer_set_background_color
void text_layer_set_text_color(TextLayer *text_layer, GColor color);

//! Sets the line break mode of the TextLayer
//! @param text_layer The TextLayer of which to set the overflow mode
//! @param line_mode The new \ref GTextOverflowMode to set
void text_layer_set_overflow_mode(TextLayer *text_layer, GTextOverflowMode line_mode);

//! Sets the font of the TextLayer
//! @param text_layer The TextLayer of which to set the font
//! @param font The new \ref GFont for the TextLayer
//! @see fonts_get_system_font
//! @see fonts_load_custom_font
void text_layer_set_font(TextLayer *text_layer, GFont font);

//! Sets the alignment of the TextLayer
//! @param text_layer The TextLayer of which to set the alignment
//! @param text_alignment The new text alignment for the TextLayer
//! @see GTextAlignment
void text_layer_set_text_alignment(TextLayer *text_layer, GTextAlignment text_alignment);

//! Calculates the size occupied by the current text of the TextLayer
//! @param text_layer the TextLayer for which to calculate the text's size
//! @return The size occupied by the current text of the TextLayer
GSize text_layer_get_content_size(TextLayer *text_layer);

//! Update the size of the text layer
//! This is a convenience function to update the frame of the TextLayer.
//! @param text_layer The TextLayer of which to set the size
//! @param max_size The new size for the TextLayer
void text_layer_set_size(TextLayer *text_layer, const GSize max_size);

//! @} // group TextLayer

//! @addtogroup ScrollLayer
//! \brief Layer that scrolls its contents, animated.
//!
//! ![](scroll_layer.png)
//! <h3>Key Points</h3>
//! * Facilitates vertical scrolling of a layer sub-hierarchy zero or more
//! arbitrary layers. The example image shows a scroll layer containing one
//! large TextLayer.
//! * Shadows to indicate that there is more content are automatically drawn
//! on top of the content. When the end of the scroll layer is reached, the
//! shadow will automatically be retracted.
//! * Scrolling from one offset to another is animated implicitly by default.
//! * The scroll layer contains a "content" sub-layer, which is the layer that
//! is actually moved up an down. Any layer that is a child of this "content"
//! sub-layer, will be moved as well. Effectively, an entire layout of layers
//! can be scrolled this way. Use the convenience function
//! \ref scroll_layer_add_child() to add child layers to the "content" sub-layer.
//! * The scroll layer needs to be informed of the total size of the contents,
//! in order to calculate from and to what point it should be able to scroll.
//! Use \ref scroll_layer_set_content_size() to set the size of the contents.
//! * The button behavior is set up, using the convenience function
//! \ref scroll_layer_set_click_config_onto_window(). This will associate the
//! UP and DOWN buttons with scrolling up and down.
//! * The SELECT button can be configured by installing a click configuration
//! provider using \ref scroll_layer_set_callbacks().
//! * To scroll programatically to a certain offset, use
//! \ref scroll_layer_set_content_offset().
//! * It is possible to get called back for each scrolling increment, by
//! installing the `.content_offset_changed_handler` callback using
//! \ref scroll_layer_set_callbacks().
//! * Only vertical scrolling is supported at the moment.
//! @{

struct ScrollLayer;
typedef struct ScrollLayer ScrollLayer;

//! Function signature for the `.content_offset_changed_handler` callback.
typedef void (*ScrollLayerCallback)(struct ScrollLayer *scroll_layer, void *context);

//! All the callbacks that the ScrollLayer exposes for use by applications.
//! @note The context parameter can be set using scroll_layer_set_context() and
//! gets passed in as context with all of these callbacks.
typedef struct ScrollLayerCallbacks {

  //! Provider function to set up the SELECT button handlers. This will be
  //! called after the scroll layer has configured the click configurations for
  //! the up/down buttons, so it can also be used to modify the default up/down
  //! scrolling behavior.
  ClickConfigProvider click_config_provider;

  //! Called every time the the content offset changes. During a scrolling
  //! animation, it will be called for each intermediary offset as well
  ScrollLayerCallback content_offset_changed_handler;

} ScrollLayerCallbacks;

//! Creates a new ScrollLayer on the heap and initalizes it with the default values:
//! * Clips: `true`
//! * Hidden: `false`
//! * Content size: `frame.size`
//! * Content offset: \ref GPointZero
//! * Callbacks: None (`NULL` for each one)
//! * Callback context: `NULL`
//! @return A pointer to the ScrollLayer. `NULL` if the ScrollLayer could not
//! be created
ScrollLayer* scroll_layer_create(GRect frame);

//! Destroys a ScrollLayer previously created by scroll_layer_create.
void scroll_layer_destroy(ScrollLayer *scroll_layer);

//! Gets the "root" Layer of the scroll layer, which is the parent for the sub-
//! layers used for its implementation.
//! @param scroll_layer Pointer to the ScrollLayer for which to get the "root" Layer
//! @return The "root" Layer of the scroll layer.
Layer* scroll_layer_get_layer(const ScrollLayer *scroll_layer);

//! Adds the child layer to the content sub-layer of the ScrollLayer.
//! This will make the child layer part of the scrollable contents.
//! The content sub-layer of the ScrollLayer will become the parent of the
//! child layer.
//! @param scroll_layer The ScrollLayer to which to add the child layer.
//! @param child The Layer to add to the content sub-layer of the ScrollLayer.
//! @note You may need to update the size of the scrollable contents using
//! \ref scroll_layer_set_content_size().
void scroll_layer_add_child(ScrollLayer *scroll_layer, Layer *child);

//! Convenience function to set the \ref ClickConfigProvider callback on the
//! given window to scroll layer's internal click config provider. This internal
//! click configuration provider, will set up the default UP & DOWN
//! scrolling behavior.
//! This function calls \ref window_set_click_config_provider_with_context to
//! accomplish this.
//!
//! If you application has set a `.click_config_provider`
//! callback using \ref scroll_layer_set_callbacks(), this will be called
//! by the internal click config provider, after configuring the UP & DOWN
//! buttons. This allows your application to configure the SELECT button
//! behavior and optionally override the UP & DOWN
//! button behavior. The callback context for the SELECT click recognizer is
//! automatically set to the scroll layer's context (see
//! \ref scroll_layer_set_context() ). This context is passed into
//! \ref ClickHandler callbacks. For the UP and DOWN buttons, the scroll layer
//! itself is passed in by default as the callback context in order to deal with
//! those buttons presses to scroll up and down automatically.
//! @param scroll_layer The ScrollLayer that needs to receive click events.
//! @param window The window for which to set the click configuration.
//! @see \ref Clicks
//! @see window_set_click_config_provider_with_context
void scroll_layer_set_click_config_onto_window(ScrollLayer *scroll_layer, struct Window *window);

//! Sets the callbacks that the scroll layer exposes.
//! The `context` as set by \ref scroll_layer_set_context() is passed into each
//! of the callbacks. See \ref ScrollLayerCallbacks for the different callbacks.
//! @note If the `context` is NULL, a pointer to scroll_layer is used
//! as context parameter instead when calling callbacks.
//! @param scroll_layer The ScrollLayer for which to assign new callbacks.
//! @param callbacks The new callbacks.
void scroll_layer_set_callbacks(ScrollLayer *scroll_layer, ScrollLayerCallbacks callbacks);

//! Sets a new callback context. This context is passed into the scroll layer's
//! callbacks and also the \ref ClickHandler for the SELECT button.
//! If `NULL` or not set, the context defaults to a pointer to the ScrollLayer
//! itself.
//! @param scroll_layer The ScrollLayer for which to assign the new callback
//! context.
//! @param context The new callback context.
//! @see scroll_layer_set_click_config_onto_window
//! @see scroll_layer_set_callbacks
void scroll_layer_set_context(ScrollLayer *scroll_layer, void *context);

//! Scrolls to the given offset, optionally animated.
//! @note When scrolling down, the offset's `.y` decrements. When scrolling up,
//! the offset's `.y` increments. If scrolled completely to the top, the offset
//! is \ref GPointZero.
//! @note The `.x` field must be `0`. Horizontal scrolling is not supported.
//! @param scroll_layer The ScrollLayer for which to set the content offset
//! @param offset The final content offset
//! @param animated Pass in `true` to animate to the new content offset, or
//! `false` to set the new content offset without animating.
//! @see scroll_layer_get_content_offset
void scroll_layer_set_content_offset(ScrollLayer *scroll_layer, GPoint offset, bool animated);

//! Gets the point by which the contents are offset.
//! @param scroll_layer The ScrollLayer for which to get the content offset
//! @see scroll_layer_set_content_offset
GPoint scroll_layer_get_content_offset(ScrollLayer *scroll_layer);

//! Sets the size of the contents layer. This determines the area that is
//! scrollable. At the moment, this needs to be set "manually" and is not
//! derived from the geometry of the contents layers.
//! @param scroll_layer The ScrollLayer for which to set the content size.
//! @param size The new content size.
//! @see scroll_layer_get_content_size
void scroll_layer_set_content_size(ScrollLayer *scroll_layer, GSize size);

//! Gets the size of the contents layer.
//! @param scroll_layer The ScrollLayer for which to get the content size
//! @see scroll_layer_set_content_size
GSize scroll_layer_get_content_size(const ScrollLayer *scroll_layer);

//! Set the frame of the scroll layer and adjusts the internal layers' geometry
//! accordingly. The scroll layer is marked dirty automatically.
//! @param scroll_layer The ScrollLayer for which to set the frame
//! @param frame The new frame
void scroll_layer_set_frame(ScrollLayer *scroll_layer, GRect frame);

//! The click handlers for the UP button that the scroll layer will install as
//! part of \ref scroll_layer_set_click_config_onto_window().
//! @note This handler is exposed, in case one wants to implement an alternative
//! handler for the UP button, as a way to invoke the default behavior.
//! @param recognizer The click recognizer for which the handler is called
//! @param context A void pointer to the ScrollLayer that is the context of the click event
void scroll_layer_scroll_up_click_handler(ClickRecognizerRef recognizer, void *context);

//! The click handlers for the DOWN button that the scroll layer will install as
//! part of \ref scroll_layer_set_click_config_onto_window().
//! @note This handler is exposed, in case one wants to implement an alternative
//! handler for the DOWN button, as a way to invoke the default behavior.
//! @param recognizer The click recognizer for which the handler is called
//! @param context A void pointer to the ScrollLayer that is the context of the click event
void scroll_layer_scroll_down_click_handler(ClickRecognizerRef recognizer, void *context);

//! Sets the visibility of the scroll layer shadow.
//! If the visibility has changed, \ref layer_mark_dirty() will be called automatically
//! on the scroll layer.
//! @param scroll_layer The scroll layer for which to set the shadow visibility
//! @param hidden Supply `true` to make the shadow hidden, or `false` to make it
//! non-hidden.
void scroll_layer_set_shadow_hidden(ScrollLayer *scroll_layer, bool hidden);

//! Gets the visibility of the scroll layer shadow.
//! @param scroll_layer The scroll layer for which to get the visibility
//! @return True if the shadow is hidden, false if it is not hidden.
bool scroll_layer_get_shadow_hidden(const ScrollLayer *scroll_layer);

//! @} // group ScrollLayer

//! @addtogroup InverterLayer
//! \brief Layer that inverts anything "below it".
//!
//! ![](inverter_layer.png)
//! This layer takes what has been drawn into the graphics context by layers
//! that are "behind" it in the layer hierarchy.
//! Then, the inverter layer uses its geometric information (bounds, frame) as
//! the area to invert in the graphics context. Inverting will cause black
//! pixels to become white and vice versa.
//!
//! The InverterLayer is useful, for example, to highlight the selected item
//! in a menu. In fact, the \ref MenuLayer itself uses InverterLayer to
//! accomplish its selection highlighting.
//! @{

struct InverterLayer;
typedef struct InverterLayer InverterLayer;

//! Creates a new InverterLayer on the heap and initializes it with the default values.
//! * Clips: `true`
//! * Hidden: `false`
//! @return A pointer to the InverterLayer. `NULL` if the InverterLayer could not
//! be created
InverterLayer* inverter_layer_create(GRect frame);

//! Destroys an InverterLayer previously created by inverter_layer_create
void inverter_layer_destroy(InverterLayer* inverter_layer);

//! Gets the "root" Layer of the inverter layer, which is the parent for the sub-
//! layers used for its implementation.
//! @param inverter_layer Pointer to the InverterLayer for which to get the "root" Layer
//! @return The "root" Layer of the inverter layer.
Layer* inverter_layer_get_layer(InverterLayer *inverter_layer);

//! @} // group InverterLayer

//! @addtogroup MenuLayer
//! \brief Layer that displays a standard list menu. Data is provided using
//! callbacks.
//!
//! ![](menu_layer.png)
//! <h3>Key Points</h3>
//! * The familiar list-style menu widget, as used throughout the Pebble user
//! interface.
//! * Built on top of \ref ScrollLayer, inheriting all its goodness like
//! animated scrolling, automatic "more content" shadow indicators, etc.
//! * All data needed to render the menu is requested on-demand via callbacks,
//! to avoid the need to keep a lot of data in memory.
//! * Support for "sections". A section is a group of items, visually separated
//! by a header with the name at the top of the section.
//! * Variable heights: each menu item cell and each section header can have
//! its own height. The heights are provided by callbacks.
//! * Deviation from the Layer system for cell drawing: Each menu item does
//! _not_ have its own Layer (to minimize memory usage). Instead, a
//! drawing callback is set onto the MenuLayer that is responsible
//! for drawing each menu item. The menu layer will call this callback for each
//! menu item that is visible and needs to be rendered.
//! * Cell and header drawing can be customized by implementing a custom drawing
//! callback.
//! * A few "canned" menu cell drawing functions are provided for convenience,
//! which support the default menu cell layout with a title, optional subtitle
//! and icon.
//!
//! For short, static list menus, consider using \ref SimpleMenuLayer.
//! @{

//! and icon on the left of the cell. Call this function inside the `.draw_row`
//! callback implementation, see \ref MenuLayerCallbacks.
//! @param ctx The destination graphics context
//! @param cell_layer The layer of the cell to draw
//! @param title If non-null, draws a title in larger text (24 points, bold
//! Raster Gothic system font).
//! @param subtitle If non-null, draws a subtitle in smaller text (18 points,
//! Raster Gothic system font). If `NULL`, the title will be centered vertically
//! inside the menu cell.
//! @param icon If non-null, draws an icon to the left of the text. If `NULL`,
//! the icon will be omitted and the leftover space is used for the title and
//! subtitle.
void menu_cell_basic_draw(GContext* ctx, const Layer *cell_layer, const char *title, const char *subtitle, GBitmap *icon);

//! Cell drawing function to draw a basic menu cell layout with title, subtitle
//! Cell drawing function to draw a menu cell layout with only one big title.
//! Call this function inside the `.draw_row` callback implementation, see
//! \ref MenuLayerCallbacks.
//! @param ctx The destination graphics context
//! @param cell_layer The layer of the cell to draw
//! @param title If non-null, draws a title in larger text (28 points, bold
//! Raster Gothic system font).
void menu_cell_title_draw(GContext* ctx, const Layer *cell_layer, const char *title);

//! Section header drawing function to draw a basic section header cell layout
//! with the title of the section.
//! Call this function inside the `.draw_header` callback implementation, see
//! \ref MenuLayerCallbacks.
//! @param ctx The destination graphics context
//! @param cell_layer The layer of the cell to draw
//! @param title If non-null, draws the title in small text (14 points, bold
//! Raster Gothic system font).
void menu_cell_basic_header_draw(GContext* ctx, const Layer *cell_layer, const char *title);

//! Default section header height in pixels
#define MENU_CELL_BASIC_HEADER_HEIGHT ((const int16_t) 16)


#define MENU_INDEX_NOT_FOUND ((const uint16_t) ~0)

//! Data structure to represent an menu item's position in a menu, by specifying
//! the section index and the row index within that section.
typedef struct MenuIndex {
  //! The index of the section
  uint16_t section;
  //! The index of the row within the section with index `.section`
  uint16_t row;
} MenuIndex;

#define MenuIndex(section, row) ((MenuIndex){ (section), (row) })

//! Comparator function to determine the order of two MenuIndex values.
//! @param a Pointer to the menu index of the first item
//! @param b Pointer to the menu index of the second item
//! @return 0 if A and B are equal, 1 if A has a higher section & row
//! combination than B or else -1
int16_t menu_index_compare(MenuIndex *a, MenuIndex *b);


typedef struct MenuCellSpan {
  int16_t y;
  int16_t h;
  MenuIndex index;
} MenuCellSpan;

struct MenuLayer;
typedef struct MenuLayer MenuLayer;

//! Function signature for the callback to get the number of sections in a menu.
//! @param menu_layer The menu layer for which the data is requested
//! @param callback_context The callback context
//! @return The number of sections in the menu
//! @see \ref menu_layer_set_callbacks()
//! @see \ref MenuLayerCallbacks
typedef uint16_t (*MenuLayerGetNumberOfSectionsCallback)(struct MenuLayer *menu_layer, void *callback_context);

//! Function signature for the callback to get the number of rows in a
//! given section in a menu.
//! @param menu_layer The menu layer for which the data is requested
//! @param section_index The index of the section of the menu for which the
//! number of items it contains is requested
//! @param callback_context The callback context
//! @return The number of rows in the given section in the menu
//! @see \ref menu_layer_set_callbacks()
//! @see \ref MenuLayerCallbacks
typedef uint16_t (*MenuLayerGetNumberOfRowsInSectionsCallback)(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);

//! Function signature for the callback to get the height of the menu cell
//! at a given index.
//! @param menu_layer The menu layer for which the data is requested
//! @param cell_index The MenuIndex for which the cell height is requested
//! @param callback_context The callback context
//! @return The height of the cell at the given MenuIndex
//! @see \ref menu_layer_set_callbacks()
//! @see \ref MenuLayerCallbacks
typedef int16_t (*MenuLayerGetCellHeightCallback)(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

//! Function signature for the callback to get the height of the section header
//! at a given section index.
//! @param menu_layer The menu layer for which the data is requested
//! @param section_index The index of the section for which the header height is
//! requested
//! @param callback_context The callback context
//! @return The height of the section header at the given section index
//! @see \ref menu_layer_set_callbacks()
//! @see \ref MenuLayerCallbacks
typedef int16_t (*MenuLayerGetHeaderHeightCallback)(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);

//! Function signature for the callback to render the menu cell at a given
//! MenuIndex.
//! @param ctx The destination graphics context to draw into
//! @param cell_layer The cell's layer, containing the geometry of the cell
//! @param cell_index The MenuIndex of the cell that needs to be drawn
//! @param callback_context The callback context
//! @note The `cell_layer` argument is provided to make it easy to re-use an
//! `.update_proc` implementation in this callback. Only the bounds and frame
//! of the `cell_layer` are actually valid and other properties should be
//! ignored.
//! @see \ref menu_layer_set_callbacks()
//! @see \ref MenuLayerCallbacks
typedef void (*MenuLayerDrawRowCallback)(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);

//! Function signature for the callback to render the section header at a given
//! section index.
//! @param ctx The destination graphics context to draw into
//! @param cell_layer The header cell's layer, containing the geometry of the
//! header cell
//! @param section_index The section index of the section header that needs to
//! be drawn
//! @param callback_context The callback context
//! @note The `cell_layer` argument is provided to make it easy to re-use an
//! `.update_proc` implementation in this callback. Only the bounds and frame
//! of the `cell_layer` are actually valid and other properties should be
//! ignored.
//! @see \ref menu_layer_set_callbacks()
//! @see \ref MenuLayerCallbacks
typedef void (*MenuLayerDrawHeaderCallback)(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);

//! Function signature for the callback to handle the event that a user hits
//! the SELECT button.
//! @param menu_layer The menu layer for which the selection event occured
//! @param cell_index The MenuIndex of the cell that is selected
//! @param callback_context The callback context
//! @see \ref menu_layer_set_callbacks()
//! @see \ref MenuLayerCallbacks
typedef void (*MenuLayerSelectCallback)(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

//! Function signature for the callback to handle a change in the current
//! selected item in the menu.
//! @param menu_layer The menu layer for which the selection event occured
//! @param new_index The MenuIndex of the new item that is selected now
//! @param old_index The MenuIndex of the old item that was selected before
//! @param callback_context The callback context
//! @see \ref menu_layer_set_callbacks()
//! @see \ref MenuLayerCallbacks
typedef void (*MenuLayerSelectionChangedCallback)(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context);

//! Data structure containing all the callbacks of a MenuLayer.
typedef struct MenuLayerCallbacks {
  //! Callback that gets called to get the number of sections in the menu.
  //! This can get called at various moments throughout the life of a menu.
  //! @note When `NULL`, the number of sections defaults to 1.
  MenuLayerGetNumberOfSectionsCallback get_num_sections;

  //! Callback that gets called to get the number of rows in a section. This
  //! can get called at various moments throughout the life of a menu.
  //! @note Must be set to a valid callback; `NULL` causes undefined behavior.
  MenuLayerGetNumberOfRowsInSectionsCallback get_num_rows;

  //! Callback that gets called to get the height of a cell.
  //! This can get called at various moments throughout the life of a menu.
  //! @note When `NULL`, the default height of 44 pixels is used.
  MenuLayerGetCellHeightCallback get_cell_height;

  //! Callback that gets called to get the height of a section header.
  //! This can get called at various moments throughout the life of a menu.
  //! @note When `NULL`, the defaults height of 0 pixels is used. This disables
  //! section headers.
  MenuLayerGetHeaderHeightCallback get_header_height;

  //! Callback that gets called to render a menu item.
  //! This gets called for each menu item, every time it needs to be
  //! re-rendered.
  //! @note Must be set to a valid callback; `NULL` causes undefined behavior.
  MenuLayerDrawRowCallback draw_row;

  //! Callback that gets called to render a section header.
  //! This gets called for each section header, every time it needs to be
  //! re-rendered.
  //! @note Must be set to a valid callback, unless `.get_header_height` is
  //! `NULL`. Causes undefined behavior otherwise.
  MenuLayerDrawHeaderCallback draw_header;

  //! Callback that gets called when the user triggers a click with the SELECT
  //! button.
  //! @note When `NULL`, click events for the SELECT button are ignored.
  MenuLayerSelectCallback select_click;

  //! Callback that gets called when the user triggers a long click with the
  //! SELECT button.
  //! @note When `NULL`, long click events for the SELECT button are ignored.
  MenuLayerSelectCallback select_long_click;

  //! Callback that gets called whenever the selection changes.
  //! @note When `NULL`, selection change events are ignored.
  MenuLayerSelectionChangedCallback selection_changed;
} MenuLayerCallbacks;

//! Creates a new MenuLayer on the heap and initalizes it with the default values.
//!
//! * Clips: `true`
//! * Hidden: `false`
//! * Content size: `frame.size`
//! * Content offset: \ref GPointZero
//! * Callbacks: None (`NULL` for each one)
//! * Callback context: `NULL`
//! * After the relevant callbacks are called to populate the menu, the item at MenuIndex(0, 0) will be selected
//!   initially.
//! @return A pointer to the MenuLayer. `NULL` if the MenuLayer could not
//! be created
MenuLayer* menu_layer_create(GRect frame);

//! Destroys a MenuLayer previously created by menu_layer_create.
void menu_layer_destroy(MenuLayer* menu_layer);

//! Gets the "root" Layer of the menu layer, which is the parent for the sub-
//! layers used for its implementation.
//! @param menu_layer Pointer to the MenuLayer for which to get the "root" Layer
//! @return The "root" Layer of the menu layer.
Layer* menu_layer_get_layer(const MenuLayer *menu_layer);

//! Gets the ScrollLayer of the menu layer, which is the layer responsible for
//! the scrolling of the menu layer.
//! @param menu_layer Pointer to the MenuLayer for which to get the ScrollLayer
//! @return The ScrollLayer of the menu layer.
ScrollLayer* menu_layer_get_scroll_layer(const MenuLayer *menu_layer);

//! Sets the callbacks for the MenuLayer.
//! @param menu_layer Pointer to the MenuLayer for which to set the callbacks
//! and callback context.
//! @param callback_context The new callback context. This is passed into each
//! of the callbacks and can be set to point to application provided data.
//! @param callbacks The new callbacks for the MenuLayer. The storage for this
//! data structure must be long lived. Therefore, it cannot be stack-allocated.
//! @see MenuLayerCallbacks
void menu_layer_set_callbacks(MenuLayer *menu_layer, void *callback_context, MenuLayerCallbacks callbacks);

//! Convenience function to set the \ref ClickConfigProvider callback on the
//! given window to menu layer's internal click config provider. This internal
//! click configuration provider, will set up the default UP & DOWN
//! scrolling / menu item selection behavior.
//! This function calls \ref scroll_layer_set_click_config_onto_window to
//! accomplish this.
//!
//! Click and long click events for the SELECT button can be handled by
//! installing the appropriate callbacks using \ref menu_layer_set_callbacks().
//! This is a deviation from the usual click configuration provider pattern.
//! @param menu_layer The MenuLayer that needs to receive click events.
//! @param window The window for which to set the click configuration.
//! @see \ref Clicks
//! @see \ref window_set_click_config_provider_with_context()
//! @see \ref scroll_layer_set_click_config_onto_window()
void menu_layer_set_click_config_onto_window(MenuLayer *menu_layer, struct Window *window);

//! Values to specify how a (selected) row should be aligned relative to the
//! visible area of the MenuLayer.
typedef enum {
  //! Don't align or update the scroll offset of the MenuLayer.
  MenuRowAlignNone,

  //! Scroll the contents of the MenuLayer in such way that the selected row
  //! is centered relative to the visible area.
  MenuRowAlignCenter,

  //! Scroll the contents of the MenuLayer in such way that the selected row
  //! is at the top of the visible area.
  MenuRowAlignTop,

  //! Scroll the contents of the MenuLayer in such way that the selected row
  //! is at the bottom of the visible area.
  MenuRowAlignBottom,

} MenuRowAlign;

//! Selects the next or previous item, relative to the current selection.
//! @param menu_layer The MenuLayer for which to select the next item
//! @param up Supply `false` to select the next item in the list (downwards),
//! or `true` to select the previous item in the list (upwards).
//! @param scroll_align The alignment of the new selection
//! @param animated Supply `true` to animate changing the selection, or `false`
//! to change the selection instantly.
//! @note If there is no next/previous item, this function is a no-op.
void menu_layer_set_selected_next(MenuLayer *menu_layer, bool up, MenuRowAlign scroll_align, bool animated);

//! Selects the item with given \ref MenuIndex.
//! @param menu_layer The MenuLayer for which to change the selection
//! @param index The index of the item to select
//! @param scroll_align The alignment of the new selection
//! @param animated Supply `true` to animate changing the selection, or `false`
//! to change the selection instantly.
//! @note If the section and/or row index exceeds the avaible number of sections
//! or resp. rows, the exceeding index/indices will be capped, effectively
//! selecting the last section and/or row, resp.
void menu_layer_set_selected_index(MenuLayer *menu_layer, MenuIndex index, MenuRowAlign scroll_align, bool animated);

//! Gets the MenuIndex of the currently selection menu item.
//! @param menu_layer The MenuLayer for which to get the current selected index.
MenuIndex menu_layer_get_selected_index(const MenuLayer *menu_layer);

//! Reloads the data of the menu. This causes the menu to re-request the menu
//! item data, by calling the relevant callbacks.
//! The current selection and scroll position will not be changed. See the
//! note with \ref menu_layer_set_selected_index() for the behavior if the
//! old selection is no longer valid.
//! @param menu_layer The MenuLayer for which to reload the data.
void menu_layer_reload_data(MenuLayer *menu_layer);

//! @} // group MenuLayer

//! @addtogroup SimpleMenuLayer
//! \brief Wrapper around \ref MenuLayer, that uses static data to display a
//! list menu.
//!
//! ![](simple_menu_layer.png)
//! @{

struct SimpleMenuLayer;
typedef struct SimpleMenuLayer SimpleMenuLayer;

//! Function signature for the callback to handle the event that a user hits
//! the SELECT button.
//! @param index The row index of the item
//! @param context The callback context
typedef void (*SimpleMenuLayerSelectCallback)(int index, void *context);

//! Data structure containing the information of a menu item.
typedef struct {
  //! The title of the menu item. Required.
  const char *title;
  //! The subtitle of the menu item. Optional, leave `NULL` if unused.
  const char *subtitle;
  //! The icon of the menu item. Optional, leave `NULL` if unused.
  GBitmap *icon;
  //! The callback that needs to be called upon a click on the SELECT button.
  //! Optional, leave `NULL` if unused.
  SimpleMenuLayerSelectCallback callback;
} SimpleMenuItem;

//! Data structure containing the information of a menu section.
typedef struct {
  //! Title of the section. Optional, leave `NULL` if unused.
  const char *title;
  //! Array of items in the section.
  const SimpleMenuItem *items;
  //! Number of items in the `.items` array.
  uint32_t num_items;
} SimpleMenuSection;

//! Creates a new SimpleMenuLayer on the heap and initializes it.
//! It also sets the internal click configuration provider onto given window.
//! @param simple_menu Pointer to the SimpleMenuLayer to initialize
//! @param frame The frame at which to initialize the menu
//! @param window The window onto which to set the click configuration provider
//! @param sections Array with sections that need to be displayed in the menu
//! @param num_sections The number of sections in the `sections` array.
//! @param callback_context Pointer to application specific data, that is passed
//! into the callbacks.
//! @note The `sections` array is not deep-copied and can therefore not be stack
//! allocated, but needs to be backed by long-lived storage.
//! @note This function does not add the menu's layer to the window.
//! @return A pointer to the SimpleMenuLayer. `NULL` if the SimpleMenuLayer could not
//! be created
SimpleMenuLayer* simple_menu_layer_create(GRect frame, Window *window,
    const SimpleMenuSection *sections, int32_t num_sections, void *callback_context);

//! Destroys a SimpleMenuLayer previously created by simple_menu_layer_create.
void simple_menu_layer_destroy(SimpleMenuLayer* menu_layer);

//! Gets the "root" Layer of the simple menu layer, which is the parent for the
//! sub-layers used for its implementation.
//! @param simple_menu Pointer to the SimpleMenuLayer for which to get the
//! "root" Layer
//! @return The "root" Layer of the menu layer.
Layer* simple_menu_layer_get_layer(const SimpleMenuLayer *simple_menu);

//! Gets the row index of the currently selection menu item.
//! @param simple_menu The SimpleMenuLayer for which to get the current
//! selected row index.
int simple_menu_layer_get_selected_index(const SimpleMenuLayer *simple_menu);

//! Selects the item in the first section at given row index.
//! @param simple_menu The SimpleMenuLayer for which to change the selection
//! @param index The row index of the item to select
//! @param animated Supply `true` to animate changing the selection, or `false`
//! to change the selection instantly.
void simple_menu_layer_set_selected_index(SimpleMenuLayer *simple_menu, int32_t index, bool animated);

//! @param simple_menu The \ref SimpleMenuLayer to get the \ref MenuLayer from.
//! @return The \ref MenuLayer.
MenuLayer *simple_menu_layer_get_menu_layer(SimpleMenuLayer *simple_menu);

//! @} // group SimpleMenuLayer

//! @addtogroup ActionBarLayer
//! \brief Vertical, bar-shaped control widget on the right edge of the window
//!
//! ![](action_bar_layer.png)
//! ActionBarLayer is a Layer that displays a bar on the right edge of the
//! window. The bar can contain up to 3 icons, each corresponding with one of
//! the buttons on the right side of the watch. The purpose of each icon is
//! to provide a hint (feed-forward) to what action a click on the respective
//! button will cause.
//!
//! The action bar is useful when there are a few (up to 3) main actions that
//! are desirable to be able to take quickly, literally with one press of a
//! button.
//!
//! <h3>More actions</h3>
//! If there are more than 3 actions the user might want to take:
//! * Try assigning the top and bottom icons of the action bar to the two most
//! immediate actions and use the middle icon to push a Window with a MenuLayer
//! with less immediate actions.
//! * Secondary actions that are not vital, can be "hidden" under a long click.
//! Try to group similar actions to one button. For example, in a Music app,
//! a single click on the top button is tied to the action to jump to the
//! previous track. Holding that same button means seek backwards.
//!
//! <h3>Directionality mapping</h3>
//! When the top and bottom buttons are used to control navigating through
//! a (potentially virtual, non-visible) list of items, follow this guideline:
//! * Tie the top button to the action that goes to the _previous_ item in the
//! list, for example "jump to previous track" in a Music app.
//! * Tie the bottom button to the action that goes to the _next_ item in the
//! list, for example "jump to next track" in a Music app.
//!
//! <h3>Geometry</h3>
//! * The action bar is 20 pixels wide. Use the \ref ACTION_BAR_WIDTH define.
//! * The top and bottom spacing is 3 pixels each (the space between the top and
//! bottom of the frame of the action bar and the edges of the window it is
//! contained in).
//! * Icons should not be wider than 18 pixels. It is recommended to use a size
//! of around 14 x 14 pixels for the "visual core" of the icon, and extending
//! or contracting where needed.
//! <h3>Example Code</h3>
//! The code example below shows how to do the initial setup of the action bar
//! in a window's `.load` handler.
//! Configuring the button actions is similar to the process when using
//! \ref window_set_click_config_provider(). See \ref Clicks for more
//! information.
//!
//! \code{.c}
//! ActionBarLayer *action_bar;
//!
//! // The implementation of my_next_click_handler and my_previous_click_handler
//! // is omitted for the sake of brevity. See the Clicks reference docs.
//!
//! void click_config_provider(void *context) {
//!   window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) my_next_click_handler);
//!   window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) my_previous_click_handler);
//! }
//!
//! void window_load(Window *window) {
//!   ...
//!   // Initialize the action bar:
//!   action_bar = action_bar_layer_create();
//!   // Associate the action bar with the window:
//!   action_bar_layer_add_to_window(action_bar, window);
//!   // Set the click config provider:
//!   action_bar_layer_set_click_config_provider(action_bar,
//!                                              click_config_provider);
//!
//!   // Set the icons:
//!   // The loading the icons is omitted for brevity... See HeapBitmap.
//!   action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, &my_icon_previous);
//!   action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, &my_icon_next);
//! }
//! \endcode
//! @{

//! The width of the action bar in pixels.
#define ACTION_BAR_WIDTH 20

//! The maximum number of action bar items.
#define NUM_ACTION_BAR_ITEMS 3

struct ActionBarLayer;
typedef struct ActionBarLayer ActionBarLayer;

//! Creates a new ActionBarLayer on the heap and initalizes it with the default values.
//! * Background color: \ref GColorBlack
//! * No click configuration provider (`NULL`)
//! * No icons
//! * Not added to / associated with any window, thus not catching any button input yet.
//! @return A pointer to the ActionBarLayer. `NULL` if the ActionBarLayer could not
//! be created
ActionBarLayer* action_bar_layer_create(void);

//! Destroys a ActionBarLayer previously created by action_bar_layer_create
void action_bar_layer_destroy(ActionBarLayer *action_bar_layer);

//! Gets the "root" Layer of the action bar layer, which is the parent for the sub-
//! layers used for its implementation.
//! @param action_bar_layer Pointer to the ActionBarLayer for which to get the "root" Layer
//! @return The "root" Layer of the action bar layer.
Layer* action_bar_layer_get_layer(ActionBarLayer *action_bar_layer);

//! Sets the context parameter, which will be passed in to \ref ClickHandler
//! callbacks and the \ref ClickConfigProvider callback of the action bar.
//! @note By default, a pointer to the action bar itself is passed in, if the
//! context has not been set or if it has been set to `NULL`.
//! @param action_bar The action bar for which to assign the new context
//! @param context The new context
//! @see action_bar_layer_set_click_config_provider()
//! @see \ref Clicks
void action_bar_layer_set_context(ActionBarLayer *action_bar, void *context);

//! Sets the click configuration provider callback of the action bar.
//! In this callback your application can associate handlers to the different
//! types of click events for each of the buttons, see \ref Clicks.
//! @note If the action bar had already been added to a window and the window
//! is currently on-screen, the click configuration provider will be called
//! before this function returns. Otherwise, it will be called by the system
//! when the window becomes on-screen.
//! @note The `.raw` handlers cannot be used without breaking the automatic
//! highlighting of the segment of the action bar that for which a button is
//! @see action_bar_layer_set_icon()
//! @param action_bar The action bar for which to assign a new click
//! configuration provider
//! @param click_config_provider The new click configuration provider
void action_bar_layer_set_click_config_provider(ActionBarLayer *action_bar, ClickConfigProvider click_config_provider);

//! Sets an action bar icon onto one of the 3 slots as identified by `button_id`.
//! Only \ref BUTTON_ID_UP, \ref BUTTON_ID_SELECT and \ref BUTTON_ID_DOWN can be
//! used. Whenever an icon is set, the click configuration provider will be
//! called, to give the application the opportunity to reconfigure the button
//! interaction.
//! @param action_bar The action bar for which to set the new icon
//! @param button_id The identifier of the button for which to set the icon
//! @param icon Pointer to the \ref GBitmap icon
//! @see action_bar_layer_set_click_config_provider()
//! @see \ref MediaUtils
void action_bar_layer_set_icon(ActionBarLayer *action_bar, ButtonId button_id, const GBitmap *icon);

//! Convenience function to clear out an existing icon.
//! All it does is call `action_bar_layer_set_icon(action_bar, button_id, NULL)`
//! @param action_bar The action bar for which to clear an icon
//! @param button_id The identifier of the button for which to clear the icon
//! @see action_bar_layer_set_icon()
void action_bar_layer_clear_icon(ActionBarLayer *action_bar, ButtonId button_id);

//! Adds the action bar's layer on top of the window's root layer. It also
//! adjusts the layout of the action bar to match the geometry of the window it
//! gets added to.
//! Lastly, it calls \ref window_set_click_config_provider_with_context() on
//! the window to set it up to work with the internal callback and raw click
//! handlers of the action bar, to enable the highlighting of the section of the
//! action bar when the user presses a button.
//! @note After this call, do not use
//! \ref window_set_click_config_provider_with_context() with the window that
//! the action bar has been added to (this would de-associate the action bar's
//! click config provider and context). Instead use
//! \ref action_bar_layer_set_click_config_provider() and
//! \ref action_bar_layer_set_context() to register the click configuration
//! provider to configure the buttons actions.
//! @note It is advised to call this is in the window's `.load` or `.appear`
//! handler. Make sure to call \ref action_bar_layer_remove_from_window() in the
//! window's `.unload` or `.disappear` handler.
//! @note Adding additional layers to the window's root layer after this calll
//! can occlude the action bar.
//! @param action_bar The action bar to associate with the window
//! @param window The window with which the action bar is to be associated
void action_bar_layer_add_to_window(ActionBarLayer *action_bar, struct Window *window);

//! Removes the action bar from the window and unconfigures the window's
//! click configuration provider. `NULL` is set as the window's new click config
//! provider and also as its callback context. If it has not been added to a
//! window before, this function is a no-op.
//! @param action_bar The action bar to de-associate from its current window
void action_bar_layer_remove_from_window(ActionBarLayer *action_bar);

//! Sets the background color of the action bar. Defaults to \ref GColorBlack.
//! The action bar's layer is automatically marked dirty.
//! @param action_bar The action bar of which to set the background color
//! @param background_color The new background color
void action_bar_layer_set_background_color(ActionBarLayer *action_bar, GColor background_color);

//! @} // group ActionBarLayer

//! @addtogroup BitmapLayer
//! \brief Layer that displays a bitmap image.
//!
//! ![](bitmap_layer.png)
//! BitmapLayer is a Layer subtype that draws a GBitmap within its frame. It uses an alignment property
//! to specify how to position the bitmap image with its frame. Optionally, when the
//! background color is not GColorClear, it draws a solid background color behind the
//! bitmap image, filling areas of the frame that are not covered by the bitmap image.
//! Lastly, using the compositing mode property of the BitmapLayer, determines the way the
//! bitmap image is drawn on top of what is underneath it (either the background color, or
//! the layers beneath it).
//!
//! <h3>Inside the Implementation</h3>
//! The implementation of BitmapLayer is fairly straightforward and relies heavily on the
//! functionality as exposed by the core drawing functions (see \ref Drawing).
//! \ref BitmapLayer's drawing callback uses \ref graphics_draw_bitmap_in_rect()
//! to perform the actual drawing of the \ref GBitmap. It uses \ref grect_align() to perform
//! the layout of the image and it uses \ref graphics_fill_rect() to draw the background plane.
//! @{

struct BitmapLayer;
typedef struct BitmapLayer BitmapLayer;

//! Creates a new bitmap layer on the heap and initalizes it the default values.
//!
//! * Bitmap: `NULL` (none)
//! * Background color: \ref GColorClear
//! * Compositing mode: \ref GCompOpAssign
//! * Clips: `true`
//! @return A pointer to the BitmapLayer. `NULL` if the BitmapLayer could not
//! be created
BitmapLayer* bitmap_layer_create(GRect frame);

//! Destroys a window previously created by bitmap_layer_create
void bitmap_layer_destroy(BitmapLayer* bitmap_layer);

//! Gets the "root" Layer of the bitmap layer, which is the parent for the sub-
//! layers used for its implementation.
//! @param bitmap_layer Pointer to the BitmapLayer for which to get the "root" Layer
//! @return The "root" Layer of the bitmap layer.
Layer* bitmap_layer_get_layer(const BitmapLayer *bitmap_layer);

//! Gets the pointer to the bitmap image that the BitmapLayer is using.
//!
//! @param bitmap_layer The BitmapLayer for which to get the bitmap image
//! @return A pointer to the bitmap image that the BitmapLayer is using
const GBitmap* bitmap_layer_get_bitmap(BitmapLayer *bitmap_layer);

//! Sets the bitmap onto the BitmapLayer. The bitmap is set by reference (no deep
//! copy), thus the caller of this function has to make sure the bitmap is kept
//! in memory.
//!
//! The bitmap layer is automatically marked dirty after this operation.
//! @param bitmap_layer The BitmapLayer for which to set the bitmap image
//! @param bitmap The new \ref GBitmap to set onto the BitmapLayer
//! @see See \ref MediaUtils for utilities to help with resource loading and bitmap handling.
void bitmap_layer_set_bitmap(BitmapLayer *bitmap_layer, const GBitmap *bitmap);

//! Sets the alignment of the image to draw with in frame of the BitmapLayer.
//! The aligment parameter specifies which edges of the bitmap should overlap
//! with the frame of the BitmapLayer.
//! If the bitmap is smaller than the frame of the BitmapLayer, the background
//! is filled with the background color.
//!
//! The bitmap layer is automatically marked dirty after this operation.
//! @param bitmap_layer The BitmapLayer for which to set the aligment
//! @param alignment The new alignment for the image inside the BitmapLayer
void bitmap_layer_set_alignment(BitmapLayer *bitmap_layer, GAlign alignment);

//! Sets the background color of bounding box that will be drawn behind the image
//! of the BitmapLayer.
//!
//! The bitmap layer is automatically marked dirty after this operation.
//! @param bitmap_layer The BitmapLayer for which to set the background color
//! @param color The new \ref GColor to set the background to
void bitmap_layer_set_background_color(BitmapLayer *bitmap_layer, GColor color);

//! Sets the compositing mode of how the bitmap image is composited onto the
//! BitmapLayer's background plane, or how it is composited onto what has been
//! drawn beneath the BitmapLayer in case the background color is set to
//! GColorClear.
//!
//! The compositing mode only affects the drawing of the bitmap and not the
//! drawing of the background color.
//!
//! The bitmap layer is automatically marked dirty after this operation.
//! @param bitmap_layer The BitmapLayer for which to set the compositing mode
//! @param mode The compositing mode to set
//! @see See \ref GCompOp for visual examples of the different compositing modes.
void bitmap_layer_set_compositing_mode(BitmapLayer *bitmap_layer, GCompOp mode);

//! @} // group BitmapLayer

//! @addtogroup RotBitmapLayer
//! @{

struct RotBitmapLayer;
typedef struct RotBitmapLayer RotBitmapLayer;

//! @return A pointer to the RotBitmapLayer. `NULL` if the RotBitmapLayer could not
//! be created
RotBitmapLayer* rot_bitmap_layer_create(GBitmap *bitmap);

void rot_bitmap_layer_destroy(RotBitmapLayer *bitmap);

void rot_bitmap_layer_set_corner_clip_color(RotBitmapLayer *image, GColor color);

//! sets rotation to the given angle
void rot_bitmap_layer_set_angle(RotBitmapLayer *image, int32_t angle);

//! changes the rotation by the given amount
void rot_bitmap_layer_increment_angle(RotBitmapLayer *image, int32_t angle_change);

void rot_bitmap_set_src_ic(RotBitmapLayer *image, GPoint ic);

void rot_bitmap_set_compositing_mode(RotBitmapLayer *image, GCompOp mode);

//! @} // group RotBitmapLayer

//! @} // group Layer

//! @addtogroup Window
//! @{

//! @addtogroup NumberWindow
//! \brief A ready-made Window prompting the user to pick a number
//!
//! ![](number_window.png)
//! @{

struct NumberWindow;
typedef struct NumberWindow NumberWindow;

//! Function signature for NumberWindow callbacks.
typedef void (*NumberWindowCallback)(struct NumberWindow *number_window, void *context);

//! Data structure containing all the callbacks for a NumberWindow.
typedef struct {
  //! Callback that gets called as the value is incremented.
  //! Optional, leave `NULL` if unused.
  NumberWindowCallback incremented;
  //! Callback that gets called as the value is decremented.
  //! Optional, leave `NULL` if unused.
  NumberWindowCallback decremented;
  //! Callback that gets called as the value is confirmed, in other words the
  //! SELECT button is clicked.
  //! Optional, leave `NULL` if unused.
  NumberWindowCallback selected;
} NumberWindowCallbacks;

//! Creates a new NumberWindow on the heap and initalizes it with the default values.
//!
//! @param label The title or prompt to display in the NumberWindow. Must be long-lived and cannot be stack-allocated.
//! @param callbacks The callbacks
//! @param callback_context Pointer to application specific data that is passed
//! @note The number window is not pushed to the window stack. Use \ref window_stack_push() to do this.
//! @return A pointer to the NumberWindow. `NULL` if the NumberWindow could not
//! be created
NumberWindow* number_window_create(const char *label, NumberWindowCallbacks callbacks, void *callback_context);

//! Destroys a NumberWindow previously created by number_window_create.
void number_window_destroy(NumberWindow* number_window);

//! Sets the text of the title or prompt label.
//! @param numberwindow Pointer to the NumberWindow for which to set the label
//! text
//! @param label The new label text. Must be long-lived and cannot be
//! stack-allocated.
void number_window_set_label(NumberWindow *numberwindow, const char *label);

//! Sets the maximum value this field can hold
//! @param numberwindow Pointer to the NumberWindow for which to set the maximum
//! value
//! @param max The maximum value
void number_window_set_max(NumberWindow *numberwindow, int32_t max);

//! Sets the minimum value this field can hold
//! @param numberwindow Pointer to the NumberWindow for which to set the minimum
//! value
//! @param min The minimum value
void number_window_set_min(NumberWindow *numberwindow, int32_t min);

//! Sets the current value of the field
//! @param numberwindow Pointer to the NumberWindow for which to set the current
//! value
//! @param value The new current value
void number_window_set_value(NumberWindow *numberwindow, int32_t value);

//! Sets the amount by which to increment/decrement by on a button click
//! @param numberwindow Pointer to the NumberWindow for which to set the step
//! increment
//! @param step The new step increment
void number_window_set_step_size(NumberWindow *numberwindow, int32_t step);

//! Gets the current value
//! @param numberwindow Pointer to the NumberWindow for which to get the current
//! value
//! @return The current value
int32_t number_window_get_value(const NumberWindow *numberwindow);

//! @} // group NumberWindow

//! @} // group Window

//! @addtogroup Vibes
//! \brief Controlling the vibration motor
//!
//! The Vibes API provides calls that let you control Pebble’s vibration motor.
//!
//! The vibration motor can be used as a visceral mechanism for giving immediate feedback to the user.
//! You can use it to highlight important moments in games, or to draw the attention of the user.
//! However, you should use the vibration feature sparingly, because sustained use will rapidly deplete Pebble’s battery,
//! and vibrating Pebble too much and too often can become annoying for users.
//! @note When using these calls, if there is an ongoing vibration,
//! calling any of the functions to emit (another) vibration will have no effect.
//! @{

/** Data structure describing a vibration pattern.
 A pattern consists of at least 1 vibe-on duration, optionally followed by
 alternating vibe-off + vibe-on durations. Each segment may have a different duration.

 Example code:
 \code{.c}
 // Vibe pattern: ON for 200ms, OFF for 100ms, ON for 400ms:
static const uint32_t const segments[] = { 200, 100, 400 };
VibePattern pat = {
  .durations = segments,
  .num_segments = ARRAY_LENGTH(segments),
};
vibes_enqueue_custom_pattern(pat);
\endcode
 @see vibes_enqueue_custom_pattern
 */
typedef struct {
  /**
   Pointer to an array of segment durations, measured in milli-seconds.
   The maximum allowed duration is 10000ms.
   */
  const uint32_t *durations;
  /**
   The length of the array of durations.
   */
  uint32_t num_segments;
} VibePattern;

//! Cancel any in-flight vibe patterns; this is a no-op if there is no
//! on-going vibe.
void vibes_cancel(void);

//! Makes the watch emit one short vibration.
void vibes_short_pulse(void);

//! Makes the watch emit one long vibration.
void vibes_long_pulse(void);

//! Makes the watch emit two brief vibrations.
//!
void vibes_double_pulse(void);

//! Makes the watch emit a ‘custom’ vibration pattern.
//! @param pattern An arbitrary vibration pattern
//! @see VibePattern
void vibes_enqueue_custom_pattern(VibePattern pattern);

//! @} // group Vibes

//! @addtogroup Light Light
//! \brief Controlling Pebble's backlight
//!
//! The Light API provides you with functions to turn on Pebble’s backlight or
//! put it back into automatic control. You can trigger the backlight and schedule a timer
//! to automatically disable the backlight after a short delay, which is the preferred
//! method of interacting with the backlight.
//! @{

//! Trigger the backlight and schedule a timer to automatically disable the backlight
//! after a short delay. This is the preferred method of interacting with the backlight.
void light_enable_interaction(void);

//! Turn the watch's backlight on or put it back into automatic control.
//! Developers should take care when calling this function, keeping Pebble's backlight on for long periods of time
//! will rapidly deplete the battery.
//! @param enable Turn the backlight on if `true`, otherwise `false` to put it back into automatic control.
void light_enable(bool enable);

//! @} // group Light

//! @} // group UI

//!   @} // group Animation
//! @} // group UI
typedef int32_t (*AnimationTimingFunction)(uint32_t time_normalized);

//! Returns the current time in Unix Timestamp Format with Milliseconds
//!     @param tloc if provided receives current Unix Time seconds portion
//!     @param out_ms if provided receives current Unix Time milliseconds portion
//!     @return Current Unix Time milliseconds portion
uint16_t time_ms(time_t *tloc, uint16_t *out_ms);

