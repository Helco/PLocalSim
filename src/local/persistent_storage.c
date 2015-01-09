#include "globals.h"
#include <stdio.h>

#define STORAGE_DATA "persistent.store"

//
// This file implements the Storage API
//
// Values created by persist_* are stored in a global storage which is then
// stored in "persistent.store" in build/local/ alongside the other simdata.
// Persistent keys will survive as long as this file exists.
//

typedef enum {
	KEY_UNUSED = 0,
	KEY_BOOL,
	KEY_INTEGER,
	KEY_STRING,
	KEY_DATA
} PersistentKeyType;

typedef struct _PersistentKey {
	uint32_t id;
	PersistentKeyType  type;
	union {
		bool    boolean;
		int32_t integer;
		char    string[PERSIST_STRING_MAX_LENGTH];
		uint8_t data[PERSIST_DATA_MAX_LENGTH];
	} val;
	size_t byte_size; // required for string / data, used by all others :)
} __attribute__((__packed__)) PersistentKey;

static int key_count = 0;
static PersistentKey* key_store = NULL;

// key lookup to reduce code duplication
static PersistentKey* _lookup_key(const uint32_t key, const bool get_unused) {
	int i;

	if (key_store == NULL || key_count == 0) {
		return NULL;
	}

	for(i = 0; i < key_count; ++i) {
		if (get_unused && key_store[i].type == KEY_UNUSED) {
			// return first free
			return &key_store[i];
		}
		else {
			if (key_store[i].id == key && key_store[i].type != KEY_UNUSED) {
				return &key_store[i];
			}
		}
	}

	if (get_unused) {
		// no free keys, we have to create a new one..
		key_store = (PersistentKey*)realloc(key_store, sizeof(PersistentKey) * (key_count + 1));
		if (key_store == NULL) {
			// oops.. disable the store
			printf("[ERROR] out of memory while reallocating the key_store!\n");
			key_count = 0;
			return NULL;
		}
		else {
			// a new blank key :)
			key_store[key_count].id = 0;
			key_store[key_count].type = KEY_UNUSED;
			return &key_store[key_count++];
		}
	}
	return NULL;
}

// simulator access

bool persistent_storage_load() {
	if(key_store != NULL) {
		printf("[WARNING] multiple calls to persistant_storage_load!\n");
		return false;
	}

	FILE* handle = fopen(STORAGE_DATA, "rb");
	if (handle == NULL) {
		printf("[ERROR] could not read '%s' - persist_* will start with fresh data!\n", STORAGE_DATA);
		key_count = 0;
	}
	else {
		fseek(handle, 0, SEEK_END);
		key_count = ftell(handle) / sizeof(PersistentKey);
		fseek(handle, 0, SEEK_SET);

		if (key_count == 0) {
			fclose(handle);
		}
	}

	if(key_count == 0) {
		// no keys, either it's a first initialization or the old file was corrupt
		key_count = 1;
		key_store = (PersistentKey*)malloc(sizeof(PersistentKey));
		if (key_store == NULL) {
			printf("[ERROR] failed to allocate memory for persistant storage!\n");
			return false;
		}

		key_store[0].id = 0;
		key_store[0].type = KEY_UNUSED;
	}
	else {
		// read stored data
		key_store = (PersistentKey*)malloc(sizeof(PersistentKey) * key_count);
		if (key_store == NULL) {
			printf("[ERROR] failed to allocate memory for persistant storage!\n");
			return false;
		}

		uint8_t* ptr = (uint8_t*)key_store;
		size_t total, read = 0;

		total = key_count * sizeof(PersistentKey);
		while ((read = fread(ptr, 1, total, handle)) != total) {
			ptr   += read;
			total -= read;

			if (total == 0) {
				break;
			}
		}
		fclose(handle);
	}

	printf("[INFO] Got %d keys from '%s'\n", key_count, STORAGE_DATA);
	return true;
}

bool persistent_storage_save() {
	if (key_store == NULL || key_count == 0) {
		printf("[INFO] Persistant storeg is empty.\n");
		return true;
	}

	FILE* handle = fopen(STORAGE_DATA, "wb");
	if (handle == NULL) {
		printf("[ERROR] could not write to '%s'!\n", STORAGE_DATA);
		return false;
	}

	uint8_t* ptr = (uint8_t*)key_store;
	size_t total, written = 0;

	total = key_count * sizeof(PersistentKey);
	while ((written = fwrite(ptr, 1, total, handle)) != total) {
		ptr   += written;
		total -= written;

		if (total == 0) {
			break;
		}
	}
	fclose(handle);

	printf("[INFO] Wrote %d keys to '%s'\n", key_count, STORAGE_DATA);
	return true;
}

void persistent_storage_free() {
	persistent_storage_save();
	if (key_store != NULL) {
		free(key_store);
		key_store = NULL;
		key_count = 0;
	}
}

// pebble API part

status_t persist_delete(const uint32_t key) {
	PersistentKey* ptr = NULL;

	ptr = _lookup_key(key, false);
	if (ptr != NULL) {
		if (ptr->id == key && ptr->type != KEY_UNUSED) {
			ptr->id = 0;
			ptr->type = KEY_UNUSED;

			return S_SUCCESS;
		}
	}
	return E_DOES_NOT_EXIST;
}

bool persist_exists(const uint32_t key) {
	return (_lookup_key(key, false) != NULL);
}

int persist_get_size(const uint32_t key) {
	PersistentKey* ptr = NULL;

	ptr = _lookup_key(key, false);
	if (ptr != NULL) {
		return ptr->byte_size;
	}
	return E_DOES_NOT_EXIST;
}

bool persist_read_bool(const uint32_t key) {
	PersistentKey* ptr = NULL;

	ptr = _lookup_key(key, false);
	if (ptr != NULL && ptr->type == KEY_BOOL) {
		return ptr->val.boolean;
	}
	return false;
}

int persist_read_data(const uint32_t key, void* buffer, const size_t buffer_size) {
	PersistentKey* ptr = NULL;

	ptr = _lookup_key(key, false);
	if (ptr != NULL && ptr->type == KEY_DATA) {
		size_t cnt = (buffer_size < ptr->byte_size) ? buffer_size : ptr->byte_size;

		if (buffer == NULL) {
			return 0;
		}

		memcpy(buffer, ptr->val.data, cnt);
		return cnt;
	}
	return E_DOES_NOT_EXIST;
}

int32_t persist_read_int(const uint32_t key) {
	PersistentKey* ptr = NULL;

	ptr = _lookup_key(key, false);
	if (ptr != NULL && ptr->type == KEY_INTEGER) {
		return ptr->val.integer;
	}
	return 0;
}

int persist_read_string(const uint32_t key, char* buffer, const size_t buffer_size) {
	PersistentKey* ptr = NULL;

	ptr = _lookup_key(key, false);
	if (ptr != NULL && ptr->type == KEY_STRING) {
		size_t cnt = (buffer_size < ptr->byte_size) ? buffer_size : ptr->byte_size;

		if (buffer == NULL) {
			return 0;
		}

		memcpy(buffer, ptr->val.string, cnt);
		return cnt;
	}
	return E_DOES_NOT_EXIST;
}

status_t persist_write_bool(const uint32_t key, const bool value) {
	PersistentKey* ptr = NULL;

	ptr = _lookup_key(key, false);
	if (ptr == NULL) {
		ptr = _lookup_key(key, true);
	}
	if (ptr != NULL) {
		memset(ptr, 0, sizeof(PersistentKey));

		ptr->id = key;
		ptr->type = KEY_BOOL;
		ptr->val.boolean = value;
		ptr->byte_size = sizeof(bool);

		return S_SUCCESS;
	}
	return E_OUT_OF_STORAGE;
}

int persist_write_data(const uint32_t key, const void* value, const size_t size) {
	PersistentKey* ptr = NULL;

	if (value == NULL) {
		return E_INVALID_ARGUMENT;
	}

	ptr = _lookup_key(key, false);
	if (ptr == NULL) {
		ptr = _lookup_key(key, true);
	}
	if (ptr != NULL) {
		size_t cnt = (size < PERSIST_DATA_MAX_LENGTH) ? size : PERSIST_DATA_MAX_LENGTH;
		memset(ptr, 0, sizeof(PersistentKey));

		ptr->id = key;
		ptr->type = KEY_DATA;
		ptr->byte_size = cnt;

		memcpy(ptr->val.data, value, cnt);
		return cnt;
	}
	return E_OUT_OF_STORAGE;
}

status_t persist_write_int(const uint32_t key, const int32_t value) {
	PersistentKey* ptr = NULL;

	ptr = _lookup_key(key, false);
	if (ptr == NULL) {
		ptr = _lookup_key(key, true);
	}
	if (ptr != NULL) {
		memset(ptr, 0, sizeof(PersistentKey));

		ptr->id = key;
		ptr->type = KEY_INTEGER;
		ptr->val.integer = value;
		ptr->byte_size = sizeof(int32_t);

		return S_SUCCESS;
	}
	return E_OUT_OF_STORAGE;
}

int persist_write_string(const uint32_t key, const char* cstring) {
	PersistentKey* ptr = NULL;

	if (cstring == NULL) {
		return E_INVALID_ARGUMENT;
	}

	ptr = _lookup_key(key, false);
	if (ptr == NULL) {
		ptr = _lookup_key(key, true);
	}
	if (ptr != NULL) {
		size_t cnt = strlen(cstring);
		cnt = (cnt < PERSIST_STRING_MAX_LENGTH - 1) ? cnt : PERSIST_STRING_MAX_LENGTH;

		memset(ptr, 0, sizeof(PersistentKey));

		ptr->id = key;
		ptr->type = KEY_STRING;
		ptr->byte_size = cnt;

		memcpy(ptr->val.string, cstring, cnt);
		return cnt;
	}
	return E_OUT_OF_STORAGE;
}
