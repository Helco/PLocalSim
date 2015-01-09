#ifndef WIN32
#include "globals.h"
#include <pthread.h>

struct AppSyncList {
    struct AppSync* s;
    struct AppSyncList* next;
};
static struct AppSyncList* g_pAppSyncList = NULL;
pthread_mutex_t g_mxAppSyncList = PTHREAD_MUTEX_INITIALIZER;

void app_sync_init(struct AppSync *s,
                   uint8_t *buffer,
                   const uint16_t buffer_size,
                   const Tuplet * const keys_and_initial_values,
                   const uint8_t count,
                   AppSyncTupleChangedCallback tuple_changed_callback,
                   AppSyncErrorCallback error_callback,
                   void *context) {

    uint32_t final_buffer_size = buffer_size;
    if (dict_serialize_tuplets_to_buffer(count, keys_and_initial_values, buffer, &final_buffer_size) != DICT_OK)
        return;

    s->buffer = buffer;
    s->buffer_size = buffer_size;

    if (dict_write_begin(&s->current_iter, buffer, final_buffer_size) != DICT_OK)
        return;

    s->callback.context = context;
    s->callback.error = error_callback;
    s->callback.value_changed = tuple_changed_callback;

    // insert into list of regiested AppSync
    struct AppSyncList* pNode = (struct AppSyncList*) malloc(sizeof(struct AppSyncList));
    pthread_mutex_lock(&g_mxAppSyncList);
    pNode->s = s;
    pNode->next = g_pAppSyncList;
    g_pAppSyncList = pNode;
    pthread_mutex_unlock(&g_mxAppSyncList);

    // send to client
    DictionaryIterator* iter;
    app_message_outbox_begin(&iter);
    for (int i = 0; i < count; ++i) {
        dict_write_tuplet(iter, &(keys_and_initial_values[i]));
    }
    dict_write_end(iter);
    app_message_outbox_send();

    // notify
    Tuple* p = dict_read_first(&s->current_iter);
    while (p) {
        tuple_changed_callback(p->key, p, NULL, context);

        p = dict_read_next(&s->current_iter);
    }
}

void app_sync_deinit(struct AppSync *s) {
    pthread_mutex_lock(&g_mxAppSyncList);
    if (!g_pAppSyncList)
        return;

    if (g_pAppSyncList->s == s) {
        struct AppSyncList* next = g_pAppSyncList->next;
        free(g_pAppSyncList);
        g_pAppSyncList = next;
    }
    else {
        struct AppSyncList* prev = g_pAppSyncList;
        struct AppSyncList* curr = g_pAppSyncList->next;
        while (curr) {
            if (curr->s == s) {
                prev->next = curr->next;
                free(curr);
                break;
            }
            curr = curr->next;
        }
    }
    pthread_mutex_unlock(&g_mxAppSyncList);

}

AppMessageResult app_sync_set(struct AppSync *s,
                              const Tuplet * const keys_and_values_to_update,
                              const uint8_t count) {

    // update state on this side
    uint32_t in_size = dict_calc_buffer_size_from_tuplets(count, keys_and_values_to_update);
    uint8_t* buffer = (uint8_t*) malloc(in_size);
    DictionaryIterator iter;
    if (dict_serialize_tuplets_to_buffer_with_iter(count, keys_and_values_to_update, &iter, buffer, &in_size) != DICT_OK)
        return APP_MSG_BUFFER_OVERFLOW;

    uint32_t size = s->buffer_size;
    dict_merge(&(s->current_iter), &size, &iter, true, s->callback.value_changed, s->callback.context);

    free(buffer);

    // prepare data & send
    DictionaryIterator* iter_out;
    app_message_outbox_begin(&iter_out);
    uint32_t out_size = iter_out->end - (void*)iter_out->dictionary;
    if (dict_serialize_tuplets_to_buffer_with_iter(count, keys_and_values_to_update, iter_out, (uint8_t*)iter_out->dictionary, &out_size) != DICT_OK)
        return APP_MSG_BUFFER_OVERFLOW;
    return app_message_outbox_send();
}

const Tuple * app_sync_get(const struct AppSync *s, const uint32_t key) {
    return dict_find(&(s->current_iter), key);
}

void service_app_sync(DictionaryIterator* iter) {
    pthread_mutex_lock(&g_mxAppSyncList);
    struct AppSyncList* pAS = g_pAppSyncList;
    while (pAS) {
        uint32_t size = pAS->s->buffer_size;
        dict_merge(&(pAS->s->current_iter), &size, iter, true, pAS->s->callback.value_changed, pAS->s->callback.context);

        pAS = pAS->next;
    }

    pthread_mutex_unlock(&g_mxAppSyncList);
}
#endif
