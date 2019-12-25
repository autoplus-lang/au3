#ifndef _AU3_VALUE_H
#define _AU3_VALUE_H
#pragma once

#include "common.h"

typedef enum {
    AU3_TNUMBER
} au3ValueType;

typedef double au3Value;

typedef struct {
    int count;
    int capacity;
    au3Value *values;
} au3ValueArray;

void au3_initValueArray(au3ValueArray *array);
void au3_writeValueArray(au3ValueArray *array, au3Value value);
void au3_freeValueArray(au3ValueArray *array);

#endif
