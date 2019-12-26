#ifndef _AU3_TABLE_H
#define _AU3_TABLE_H
#pragma once

#include "common.h"
#include "value.h"

typedef struct {
    au3String *key;
    au3Value value;
} au3Entry;

typedef struct {
    int count;
    int capacity;
    au3Entry *entries;
} au3Table;

void au3_initTable(au3Table *table);
void au3_freeTable(au3Table *table);
bool au3_tableGet(au3Table *table, au3String *key, au3Value *value);
bool au3_tableSet(au3Table *table, au3String *key, au3Value value);
bool au3_tableDelete(au3Table *table, au3String *key);
void au3_tableAddAll(au3Table *from, au3Table *to);
au3String *au3_tableFindString(au3Table *table, const char *chars, int length, uint32_t hash);

void au3_tableRemoveWhite(au3Table* table);
void au3_markTable(au3VM *vm,au3Table *table);

#endif
