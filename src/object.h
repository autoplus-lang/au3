#ifndef _AU3_OBJECT_H
#define _AU3_OBJECT_H
#pragma once

#include "value.h"
#include "chunk.h"
#include "vm.h"

struct _au3Object {
    au3ObjectType type : 8;
    unsigned isMarked : 1;
    struct _au3Object *next;
};

struct _au3String {
    au3Object object;
    int length;
    char *chars;
    uint32_t hash;
};

struct _au3Function {
    au3Object object;
    int arity;
    int upvalueCount;
    au3Upvalue **upvalues;
    au3Chunk chunk;
    au3String *name;
};

struct _au3Upvalue {
    au3Object object;
    au3Value *location;
    au3Value closed;
    struct _au3Upvalue *next;
};

struct _au3Native {
    au3Object object;
    au3NativeFn function;
    char *tips;
};

static inline bool au3_isObjType(au3Value value, au3ObjectType type)
{
    return AU3_IS_OBJECT(value) && AU3_AS_OBJECT(value)->type == type;
}

#define AU3_OBJECT_TYPE(v)      (AU3_AS_OBJECT(v)->type)

#define AU3_IS_STRING(v)        au3_isObjType(v, AU3_TSTRING)
#define AU3_IS_FUNCTION(v)      au3_isObjType(v, AU3_TFUNCTION)
#define AU3_IS_NATIVE(v)        au3_isObjType(v, AU3_TNATIVE)

#define AU3_AS_STRING(v)        ((au3String *)AU3_AS_OBJECT(v))         
#define AU3_AS_CSTRING(v)       (((au3String *)AU3_AS_OBJECT(v))->chars)
#define AU3_AS_FUNCTION(v)      ((au3Function *)AU3_AS_OBJECT(v))
#define AU3_AS_NATIVE(v)        (((au3Native *)AU3_AS_OBJECT(v))->function)

au3String *au3_takeString(au3VM *vm, char *chars, int length);
au3String *au3_copyString(au3VM *vm, const char *chars, int length);

au3Function *au3_newFunction(au3VM *vm);
void au3_makeClosure(au3Function *function);

au3Upvalue *au3_newUpvalue(au3VM *vm, au3Value *slot);
au3Native *au3_newNative(au3VM *vm, au3NativeFn function, const char *tips);

const char *au3_typeofObject(au3Object *object);
void au3_printObject(au3Object *object);

#endif
