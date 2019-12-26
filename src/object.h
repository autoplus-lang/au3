#ifndef _AU3_OBJECT_H
#define _AU3_OBJECT_H
#pragma once

#include "value.h"
#include "vm.h"

struct _au3Object {
    au3ObjectType type;
    struct _au3Object *next;
};

struct _au3String {
    au3Object object;
    int length;
    char *chars;
};

static inline bool au3_isObjType(au3Value value, au3ObjectType type)
{
    return AU3_IS_OBJECT(value) && AU3_AS_OBJECT(value)->type == type;
}

#define AU3_OBJECT_TYPE(v)      (AU3_AS_OBJECT(v)->type)

#define AU3_IS_STRING(v)        au3_isObjType(v, AU3_TSTRING)

#define AU3_AS_STRING(v)        ((au3String *)AU3_AS_OBJECT(v))         
#define AU3_AS_CSTRING(v)       (((au3String *)AU3_AS_OBJECT(v))->chars)

au3String *au3_takeString(au3VM *vm, char *chars, int length);
au3String *au3_copyString(au3VM *vm, const char *chars, int length);

const char *au3_typeofObject(au3Object *object);
void au3_printObject(au3Object *object);

#endif
