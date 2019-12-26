#ifndef _AU3_VALUE_H
#define _AU3_VALUE_H
#pragma once

#include "common.h"

typedef struct _au3Object *au3Object;

typedef enum {
    AU3_TNULL,
    AU3_TBOOL,
    AU3_TINTEGER,
    AU3_TNUMBER,
    AU3_TOBJECT,

    AU3_TSTRING,

} au3ValueType;

typedef struct {
    au3ValueType type;
    union {
        bool boolean : 1;
        int64_t integer;
        double number;
        au3Object *object;
    };
} au3Value;

typedef struct {
    int count;
    int capacity;
    au3Value *values;
} au3ValueArray;

void au3_initValueArray(au3ValueArray *array);
void au3_writeValueArray(au3ValueArray *array, au3Value value);
void au3_freeValueArray(au3ValueArray *array);

#endif
