#include "globals.h"

uint32_t dict_calc_buffer_size(const uint8_t tuple_count, ...) {
    uint32_t uiSize = 1 + (tuple_count * 7);

    va_list arguments;
    va_start(arguments, tuple_count);
    for (int i = 0; i < tuple_count; ++i) {
        uiSize += va_arg(arguments, uint32_t);
    }
    va_end(arguments);

    return uiSize;
}

// sets up iterator and positions to the start
DictionaryResult dict_write_begin(DictionaryIterator *iter,
                                  uint8_t * const buffer,
                                  const uint16_t size) {

    iter->dictionary = (struct Dictionary*) buffer;

    iter->end = buffer + size;

    iter->cursor = (Tuple*)buffer;

    return DICT_OK;
}

// helper used to write actual data
static DictionaryResult dict_write_data_helper(DictionaryIterator *iter,
                                               const uint32_t key,
                                               const TupleType type,
                                               const void * const data,
                                               const uint16_t size) {

    if (!iter || !iter->cursor)
        return DICT_INVALID_ARGS;

    Tuple tuple;
    tuple.key = key;
    tuple.type = type;
    tuple.length = size;

    if (((7 + size) + ((void*)iter->cursor)) >= iter->end)
        return DICT_NOT_ENOUGH_STORAGE;

    // header
    memcpy(iter->cursor, &tuple, 7);
    iter->cursor = ((void*)iter->cursor) + 7;

    // payload
    memcpy(iter->cursor, data, size);
    iter->cursor = ((void*)iter->cursor) + size;

    return DICT_OK;
}

DictionaryResult dict_write_data(DictionaryIterator *iter,
                                 const uint32_t key,
                                 const uint8_t * const data,
                                 const uint16_t size) {

    return dict_write_data_helper(iter, key, TUPLE_BYTE_ARRAY, data, size);
}

DictionaryResult dict_write_cstring(DictionaryIterator *iter,
                                    const uint32_t key,
                                    const char * const cstring) {

    return dict_write_data_helper(iter, key, TUPLE_CSTRING, cstring, strlen(cstring)+1);
}

DictionaryResult dict_write_int(DictionaryIterator *iter,
                                const uint32_t key,
                                const void *integer,
                                const uint8_t width_bytes,
                                const bool is_signed) {

    return dict_write_data_helper(iter, key, is_signed ? TUPLE_INT : TUPLE_UINT, integer, width_bytes);
}

DictionaryResult dict_write_uint8(DictionaryIterator *iter,
                                  const uint32_t key,
                                  const uint8_t value) {

    return dict_write_int(iter, key, &value, 1, false);
}

DictionaryResult dict_write_uint16(DictionaryIterator *iter,
                                   const uint32_t key,
                                   const uint16_t value) {

    return dict_write_int(iter, key, &value, 2, false);
}

DictionaryResult dict_write_uint32(DictionaryIterator *iter,
                                   const uint32_t key,
                                   const uint32_t value) {

    return dict_write_int(iter, key, &value, 4, false);
}

DictionaryResult dict_write_int8(DictionaryIterator *iter,
                                  const uint32_t key,
                                  const int8_t value) {

    return dict_write_int(iter, key, &value, 1, true);
}

DictionaryResult dict_write_int16(DictionaryIterator *iter,
                                   const uint32_t key,
                                   const int16_t value) {

    return dict_write_int(iter, key, &value, 2, true);
}

DictionaryResult dict_write_int32(DictionaryIterator *iter,
                                   const uint32_t key,
                                   const int32_t value) {

    return dict_write_int(iter, key, &value, 4, true);
}

// close off iterator and return size
uint32_t dict_write_end(DictionaryIterator *iter) {
    const uint32_t size = ((void*)iter->cursor - (void*)iter->dictionary);
    iter->end = (void*)iter->cursor;
    iter->cursor = NULL;
    return size;
}

Tuple * dict_read_begin_from_buffer(DictionaryIterator *iter,
                                    const uint8_t * const buffer,
                                    const uint16_t size) {

    if (dict_write_begin(iter, (uint8_t* const)buffer, size) != DICT_OK)
        return NULL;

    return iter->cursor;
}

Tuple * dict_read_next(DictionaryIterator *iter) {
    // already at the end
    if ((void*)iter->cursor >= iter->end)
        return NULL;

    iter->cursor = (Tuple*) (((void*)iter->cursor) + 7 + iter->cursor->length);

    // now we're at the end
    if ((void*)iter->cursor >= iter->end)
        return NULL;

    return iter->cursor;
}

Tuple * dict_read_first(DictionaryIterator *iter) {
    iter->cursor = (Tuple*) iter->dictionary;
    return iter->cursor;
}

DictionaryResult dict_serialize_tuplets(DictionarySerializeCallback callback,
                                        void *context,
                                        const uint8_t tuplets_count,
                                        const Tuplet * const tuplets) {

    const uint32_t size = dict_calc_buffer_size_from_tuplets(tuplets_count, tuplets);
    uint8_t* buffer = (uint8_t*) malloc(size);

    DictionaryIterator iter;
    DictionaryResult rval = dict_write_begin(&iter, buffer, size);
    if (rval != DICT_OK)
        return rval;

    for (int i = 0; i < tuplets_count; ++i) {
        rval = dict_write_tuplet(&iter, &(tuplets[i]));
        if (rval != DICT_OK)
            return rval;
    }

    callback(buffer, size, context);

    return DICT_OK;
}

DictionaryResult dict_serialize_tuplets_to_buffer(const uint8_t tuplets_count,
                                                  const Tuplet * const tuplets,
                                                  uint8_t *buffer,
                                                  uint32_t *size_in_out) {

    DictionaryIterator iter;
    return dict_serialize_tuplets_to_buffer_with_iter(tuplets_count, tuplets, &iter, buffer, size_in_out);
}

DictionaryResult dict_serialize_tuplets_to_buffer_with_iter(const uint8_t tuplets_count,
                                                            const Tuplet * const tuplets,
                                                            DictionaryIterator *iter,
                                                            uint8_t *buffer,
                                                            uint32_t *size_in_out) {

    const uint32_t size = dict_calc_buffer_size_from_tuplets(tuplets_count, tuplets);
    if (size > *size_in_out)
        return DICT_NOT_ENOUGH_STORAGE;
    *size_in_out = size;

    DictionaryResult rval = dict_write_begin(iter, buffer, size);
    if (rval != DICT_OK)
        return rval;

    for (int i = 0; i < tuplets_count; ++i) {
        rval = dict_write_tuplet(iter, &(tuplets[i]));
        if (rval != DICT_OK)
            return rval;
    }

    return DICT_OK;
}

DictionaryResult dict_write_tuplet(DictionaryIterator *iter, const Tuplet * const tuplet) {
    switch (tuplet->type) {
    case TUPLE_BYTE_ARRAY:
        return dict_write_data(iter, tuplet->key, tuplet->bytes.data, tuplet->bytes.length);
    case TUPLE_CSTRING:
        return dict_write_cstring(iter, tuplet->key, tuplet->cstring.data);
    case TUPLE_INT:
        return dict_write_int(iter, tuplet->key, &(tuplet->integer.storage), tuplet->integer.width, true);
    case TUPLE_UINT:
        return dict_write_int(iter, tuplet->key, &(tuplet->integer.storage), tuplet->integer.width, false);
    }

    return DICT_INVALID_ARGS;
}

uint32_t dict_calc_buffer_size_from_tuplets(const uint8_t tuplets_count, const Tuplet * const tuplets) {
    uint32_t uiSize = 1 + (tuplets_count * 7);

    for (int i = 0; i < tuplets_count; ++i) {
        switch(tuplets[i].type) {
        case TUPLE_BYTE_ARRAY:
            uiSize += tuplets[i].bytes.length;
            break;
        case TUPLE_CSTRING:
            uiSize += tuplets[i].cstring.length;
            break;
        case TUPLE_INT:
        case TUPLE_UINT:
            uiSize += tuplets[i].integer.width;
            break;
        }
    }

    return uiSize;
}

DictionaryResult dict_merge(DictionaryIterator *dest,
                            uint32_t *dest_max_size_in_out,
                            DictionaryIterator *source,
                            const bool update_existing_keys_only,
                            const DictionaryKeyUpdatedCallback key_callback,
                            void *context) {

    // figure out size of new dictionary

    uint32_t new_size = 0;

    Tuple* tuple = dict_read_first(dest);
    while (tuple) {
        Tuple* sourceTuple = dict_find(source, tuple->key);
        if (sourceTuple)
            new_size = 7 + sourceTuple->length;
        else
            new_size = 7 + tuple->length;

        tuple = dict_read_next(dest);
    }

    if (!update_existing_keys_only) {
        tuple = dict_read_first(source);
        while (tuple) {
            Tuple* destTuple = dict_find(dest, tuple->key);
            if (!destTuple)
                new_size = 7 + destTuple->length;

            tuple = dict_read_next(source);
        }
    }

    if (new_size > *dest_max_size_in_out)
        return DICT_NOT_ENOUGH_STORAGE;

    // allocate temporary dictionary to hold previous dest
    const uint16_t old_size = dest->end - (void*)dest->dictionary;
    uint8_t* temp_buffer = (uint8_t*) malloc(old_size);
    memcpy(temp_buffer, dest->dictionary, old_size);
    DictionaryIterator temp_iter;
    tuple = dict_read_begin_from_buffer(&temp_iter, temp_buffer, old_size);
    if (!tuple)
        return DICT_INTERNAL_INCONSISTENCY;

    // re-init dest
    DictionaryResult rval = dict_write_begin(dest, (uint8_t*)dest->dictionary, *dest_max_size_in_out);
    if (rval != DICT_OK)
        return rval;

    // merge existing
    while (tuple) {
        Tuple* sourceTuple = dict_find(source, tuple->key);
        if (sourceTuple) {
            Tuple* newTuple = dest->cursor;
            rval = dict_write_data_helper(dest, sourceTuple->key, sourceTuple->type, sourceTuple->value, sourceTuple->length);
            if (rval != DICT_OK)
                return rval;

            if (key_callback)
                key_callback(tuple->key, newTuple, tuple, context);
        }
        else {
            rval = dict_write_data_helper(dest, tuple->key, tuple->type, tuple->value, tuple->length);
            if (rval != DICT_OK)
                return rval;
        }

        tuple = dict_read_next(&temp_iter);
    }

    // merge new
    if (!update_existing_keys_only) {
        tuple = dict_read_first(source);
        while (tuple) {
            Tuple* destTuple = dict_find(&temp_iter, tuple->key);
            if (!destTuple) {
                rval = dict_write_data_helper(dest, tuple->key, tuple->type, tuple->value, tuple->length);
                if (rval != DICT_OK)
                    return rval;
            }

            tuple = dict_read_next(source);
        }
    }

    free(temp_buffer);
    *dest_max_size_in_out = dict_write_end(dest);

    return DICT_OK;
}

Tuple *dict_find(const DictionaryIterator *iter, const uint32_t key) {
    Tuple* tuple = dict_read_first((DictionaryIterator*)iter);
    while (tuple) {
        if (tuple->key == key)
            return tuple;

        tuple = dict_read_next((DictionaryIterator*)iter);
    }
    return tuple;
}
