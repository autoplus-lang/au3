#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "object.h"
#include "memory.h"

#define TABLE_MAX_LOAD  0.75

void au3_initTable(au3Table *table)
{
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void au3_freeTable(au3Table *table)
{
    free(table->entries);
    au3_initTable(table);
}

static au3Entry* findEntry(au3Entry *entries, int capacity, au3String* key)
{
    uint32_t index = key->hash % capacity;
    au3Entry *tombstone = NULL;

    for (;;) {
        au3Entry* entry = &entries[index];

        if (entry->key == NULL) {
            if (AU3_IS_NULL(entry->value)) {
                // Empty entry.                              
                return tombstone != NULL ? tombstone : entry;
            }
            else {
                // We found a tombstone.                     
                if (tombstone == NULL) tombstone = entry;
            }
        }
        else if (entry->key == key) {
            // We found the key.                           
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

bool au3_tableGet(au3Table *table, au3String *key, au3Value *value)
{
    if (table->count == 0) return false;

    au3Entry *entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    *value = entry->value;
    return true;
}

static void adjustCapacity(au3Table *table, int capacity)
{
    au3Entry *entries = malloc(sizeof(au3Entry) * capacity);
    for (int i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = AU3_NULL;
    }

    table->count = 0;
    for (int i = 0; i < table->capacity; i++) {
        au3Entry *entry = &table->entries[i];
        if (entry->key == NULL) continue;

        au3Entry *dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    free(table->entries);
    table->entries = entries;
    table->capacity = capacity;
}

bool au3_tableSet(au3Table *table, au3String *key, au3Value value)
{
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = AU3_GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    au3Entry *entry = findEntry(table->entries, table->capacity, key);

    bool isNewKey = entry->key == NULL;
    if (isNewKey && AU3_IS_NULL(entry->value)) table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool au3_tableDelete(au3Table *table, au3String *key)
{
    if (table->count == 0) return false;

    // Find the entry.                                             
    au3Entry *entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    // Place a tombstone in the entry.                             
    entry->key = NULL;
    entry->value = AU3_TRUE;

    return true;
}

void au3_tableAddAll(au3Table *from, au3Table *to)
{
    for (int i = 0; i < from->capacity; i++) {
        au3Entry* entry = &from->entries[i];
        if (entry->key != NULL) {
            au3_tableSet(to, entry->key, entry->value);
        }
    }
}
