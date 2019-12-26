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

#define AU3_NULL            ((au3Value){ AU3_TNULL })
#define AU3_BOOL(b)         ((au3Value){ AU3_TBOOL, .boolean = (b) })
#define AU3_INTEGER(i)      ((au3Value){ AU3_TINTEGER, .integer = (i) })
#define AU3_NUMBER(n)       ((au3Value){ AU3_TNUMBER, .number = (n) })
#define AU3_OBJECT(o)       ((au3Value){ AU3_TOBJECT, .object = (au3Object *)(o) })

#define AU3_AS_BOOL(v)      ((v).boolean)                       
#define AU3_AS_INTEGER(v)   ((v).integer)
#define AU3_AS_NUMBER(v)    ((v).number)
#define AU3_AS_OBJECT(v)    ((v).object)

#define AU3_IS_NULL(v)      ((v).type == AU3_TNULL)
#define AU3_IS_BOOL(v)      ((v).type == AU3_TBOOL)
#define AU3_IS_INTEGER(v)   ((v).type == AU3_TINTEGER)
#define AU3_IS_NUMBER(v)    ((v).type == AU3_TNUMBER)
#define AU3_IS_OBJECT(v)    ((v).type == AU3_TOBJECT)

typedef struct {
    int count;
    int capacity;
    au3Value *values;
} au3ValueArray;

void au3_initValueArray(au3ValueArray *array);
void au3_writeValueArray(au3ValueArray *array, au3Value value);
void au3_freeValueArray(au3ValueArray *array);

const char *au3_typeofValue(au3Value value);
void au3_printValue(au3Value value);

#endif
