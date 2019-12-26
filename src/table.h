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

#endif
