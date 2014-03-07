#ifndef WIN32
#include "globals.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

//////////////////////////////////////////////////
// sending functionality

// the sending socket
static int g_socketSend = -1;

// initiate the connection
// server is expected to be at localhost@8321
static bool init_send_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        return false;

    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8321);
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
        return false;

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        return false;

    g_socketSend = sockfd;

    return true;
}

// send message over socket
static bool send_message(const uint8_t* const buffer, const uint16_t length) {
    if (g_socketSend < 0)
        return false;

    // header containing length is always sent
    if (send(g_socketSend, &length, sizeof(length), 0) < 0)
        return false;

    // send actual message
    if (send(g_socketSend, buffer, length, 0) < 0)
        return false;

    return true;
}

//////////////////////////////////////////////////
// receiving functionality

static int g_socketReceive = -1;
pthread_t g_threadReceive;

// initiate the server
// server will be at localhost@8322
static bool init_receive_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        return false;

    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8322);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        return false;

    if (listen(sockfd, 5) < 0)
        return false;

    g_socketReceive = sockfd;

    return true;
}

// func(buffer, size)
typedef void (*PostMessageFunc)(uint8_t*, uint16_t);

// thread function for handling incoming messages
static void* receive_server(void* arg) {
    pthread_detach(pthread_self());

    PostMessageFunc postMessage = (PostMessageFunc) arg;

    printf("[INFO] Ready to Receive Messages from JS\n");

    while(1) {
        const int connfd = accept(g_socketReceive, (struct sockaddr*)NULL, NULL);

        printf("[DEBUG] Received Message\n");

        uint16_t msg_size = 0;
        if (recv(connfd, &msg_size, 2, 0) != 2) {
            printf("[ERROR] Bad Message - Invalid Size\n");
            close(connfd);
            continue;
        }

        uint8_t* buffer = (uint8_t*) malloc(msg_size);
        int act_size = 0;
        if ((act_size = recv(connfd, buffer, msg_size, 0)) != msg_size) {
            printf("[ERROR] Bad Message - Invalid Size %u\n", act_size);
            close(connfd);
            free(buffer);
            continue;
        }

        postMessage(buffer, msg_size);

        close(connfd);
    }

    return NULL;
}

// initiates receive server
static bool init_receive_listener(PostMessageFunc func) {
    if (g_socketReceive < 0)
        return false;

    if (pthread_create (&g_threadReceive, NULL, &receive_server, (void*)func) != 0)
        return false;

    return true;
}

//////////////////////////////////////////////////
// app interface

// linked list to hold queued up messages
struct message_queue_post {
    uint8_t* buffer;
    uint16_t size;
    struct message_queue_post* next;
};
struct message_queue_post* g_messageQueue = NULL;
pthread_mutex_t g_messageQueueMutex = PTHREAD_MUTEX_INITIALIZER;

// used from receiver thread to post messages
void post_received_message(uint8_t* buffer, uint16_t size) {
    struct message_queue_post* p = (struct message_queue_post*) malloc(sizeof(struct message_queue_post));
    p->buffer = buffer;
    p->size = size;
    p->next = NULL;

    pthread_mutex_lock(&g_messageQueueMutex);

    if (!g_messageQueue)
        g_messageQueue = p;
    else {
        struct message_queue_post* temp = g_messageQueue;
        while (temp->next)
            temp = temp->next;
        temp->next = p;
    }

    pthread_mutex_unlock(&g_messageQueueMutex);
}

// check if we need to spawn JS server by checking for existing of pebble-js-app-local.js
static bool hasJS() {
   return (access("pebble-js-app-local.js", F_OK) != -1);
}

// called from main()
bool setup_js_app() {
    if (!hasJS())
        return true;

    if (!init_receive_socket())
        return false;

    if (!init_send_socket())
        return false;

    if (!init_receive_listener(&post_received_message))
        return false;

    // initiate connection with this handshake
    // will trigger 'ready' event on server
    const char* msg = "READY";
    if (!send_message((const uint8_t*)msg, strlen(msg)))
        return false;

    printf("[INFO] Initialized Connection to JS app\n");

    return true;
}

//////////////////////////////////////////////
// AppMessage API

// handlers
static AppMessageInboxReceived g_fInboxReceived = NULL;
static AppMessageInboxDropped g_fInboxDropped = NULL;
static AppMessageOutboxSent g_fOutboxSent = NULL;
static AppMessageOutboxFailed g_fOutboxFailed = NULL;

// user-set context
static void* g_pContext = NULL;

// inbox
static uint8_t* g_pInboxBuffer = NULL;
static uint32_t g_uiInboxSize = 0;
static DictionaryIterator g_dictIn;

// outbox
static uint8_t* g_pOutboxBuffer = NULL;
static uint32_t g_uiOutboxSize = 0;
static DictionaryIterator g_dictOut;

// create inbox and outbox
AppMessageResult app_message_open(const uint32_t size_inbound, const uint32_t size_outbound) {
    // obey size constraints
    if ((size_inbound > app_message_inbox_size_maximum()) ||
        (size_outbound > app_message_outbox_size_maximum()))
        return APP_MSG_OUT_OF_MEMORY;

    const uint32_t new_inbox_size = (size_inbound > APP_MESSAGE_INBOX_SIZE_MINIMUM) ? size_inbound : APP_MESSAGE_INBOX_SIZE_MINIMUM;
    const uint32_t new_outbox_size = (size_outbound > APP_MESSAGE_OUTBOX_SIZE_MINIMUM) ? size_outbound : APP_MESSAGE_OUTBOX_SIZE_MINIMUM;

    // allocate inbox
    if (g_uiInboxSize != new_inbox_size) {
        if (g_pInboxBuffer)
            free(g_pInboxBuffer);

        g_pInboxBuffer = (uint8_t*) malloc(new_inbox_size);
        g_uiInboxSize = new_inbox_size;
    }

    // allocate outbox
    if (g_uiOutboxSize != new_outbox_size) {
        if (g_pOutboxBuffer)
            free(g_pOutboxBuffer);

        g_pOutboxBuffer = (uint8_t*) malloc(new_outbox_size);
        g_uiOutboxSize = new_outbox_size;
    }

    return APP_MSG_OK;
}

// clears callbacks
void app_message_deregister_callbacks(void) {
    g_fInboxReceived = NULL;
    g_fInboxDropped = NULL;
    g_fOutboxSent = NULL;
    g_fOutboxFailed = NULL;
}

// gets user-set context
void *app_message_get_context(void) {
    return g_pContext;
}

// sets user context
void *app_message_set_context(void *context) {
    void* p = g_pContext;
    g_pContext = context;
    return p;
}

// sets callback for mesage receipt
AppMessageInboxReceived app_message_register_inbox_received(AppMessageInboxReceived received_callback) {
    AppMessageInboxReceived p = g_fInboxReceived;
    g_fInboxReceived = received_callback;
    return p;
}

// sets callback for dropped messages
AppMessageInboxDropped app_message_register_inbox_dropped(AppMessageInboxDropped dropped_callback) {
    AppMessageInboxDropped p = g_fInboxDropped;
    g_fInboxDropped = dropped_callback;
    return p;
}

// sets callback for successful message send
AppMessageOutboxSent app_message_register_outbox_sent(AppMessageOutboxSent sent_callback) {
    AppMessageOutboxSent p = g_fOutboxSent;
    g_fOutboxSent = sent_callback;
    return p;
}

// sets callback for message sending failure
AppMessageOutboxFailed app_message_register_outbox_failed(AppMessageOutboxFailed failed_callback) {
    AppMessageOutboxFailed p = g_fOutboxFailed;
    g_fOutboxFailed = failed_callback;
    return p;
}

// supposed to be defined in firmware
uint32_t app_message_inbox_size_maximum(void) {
    return 0xFFFF; // made this up
}

// supposed to be defined in firmware
uint32_t app_message_outbox_size_maximum(void) {
    return 0xFFFF; // made this up
}

// get writeable DictionaryIterator belonging to the outbox
// if iterator is already in use it is reset to the buffer start
AppMessageResult app_message_outbox_begin(DictionaryIterator **iterator) {
    if (dict_write_begin(&g_dictOut, g_pOutboxBuffer, g_uiOutboxSize) != DICT_OK)
        return APP_MSG_INVALID_ARGS;

    *iterator = &g_dictOut;

    return APP_MSG_OK;
}

// sends written outbox contents
AppMessageResult app_message_outbox_send(void) {
    if (!send_message(g_pOutboxBuffer, (void*)g_dictOut.end - (void*)g_pOutboxBuffer)) {
        if (g_fOutboxFailed)
            g_fOutboxFailed(&g_dictOut, APP_MSG_NOT_CONNECTED, g_pContext);

        return APP_MSG_NOT_CONNECTED;
    }

    if (g_fOutboxSent)
        g_fOutboxSent(&g_dictOut, g_pContext);

    return APP_MSG_OK;
}

// dispatch any queued messages
void service_app_message() {
    // pull out queued messages
    struct message_queue_post* msgQueue = NULL;
    pthread_mutex_lock(&g_messageQueueMutex);
    msgQueue = g_messageQueue;
    g_messageQueue = NULL;
    pthread_mutex_unlock(&g_messageQueueMutex);

    // if nothing queued, bail
    if (!msgQueue)
        return;

    // add messages to inbox
    uint16_t size_used = 0;
    struct message_queue_post* curr = msgQueue;
    while (curr) {
        // detect overflow
        if ((size_used + curr->size) > g_uiInboxSize) {
            if (g_fInboxDropped)
                g_fInboxDropped(APP_MSG_BUFFER_OVERFLOW, g_pContext);
        }
        else {
            memcpy(g_pInboxBuffer + size_used, curr->buffer, curr->size);
            size_used += curr->size;
        }

        // clean up
        free(curr->buffer);
        struct message_queue_post* next = curr->next;
        free(curr);
        curr = next;
    }

    // dispatch
    dict_read_begin_from_buffer(&g_dictIn, g_pInboxBuffer, size_used);
    service_app_sync(&g_dictIn);
    if (g_fInboxReceived)
        g_fInboxReceived(&g_dictIn, g_pContext);

}
#endif
