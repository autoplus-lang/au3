#pragma once

#include "value.h"
#include "code.h"
#include "table.h"
#include "hash.h"

struct _obj {
    otype_t type : 8;
    uint8_t isMarked : 1;
    obj_t *next;
};

struct _str {
    obj_t obj;
    int length;
    uint32_t hash;
    char *chars;
};

struct _upv {
    obj_t obj;
    upv_t *next;
    val_t *location;
    val_t closed;
};

struct _fun {
    obj_t obj;
    int arity;
    int upvalueCount;
    upv_t **upvalues;
    str_t *name;
    chunk_t chunk;
};

struct _map {
    obj_t obj;
    hash_t hash;
    tab_t table;
};

#define AS_STR(v)       ((str_t *)AS_OBJ(v))
#define AS_CSTR(v)      (((str_t *)AS_OBJ(v))->chars)
#define AS_FUN(v)       ((fun_t *)AS_OBJ(v))
#define AS_MAP(v)       ((map_t *)AS_OBJ(v))

#define OBJ_TYPE(v)     (AS_OBJ(v)->type)

static inline bool obj_is(val_t value, otype_t type) {
    return IS_OBJ(value) && OBJ_TYPE(value) == type;
}

#define IS_STR(v)       (obj_is(v, OT_STR))
#define IS_FUN(v)       (obj_is(v, OT_FUN))
#define IS_MAP(v)       (obj_is(v, OT_MAP))

str_t *str_take(vm_t *vm, char *chars, int length);
str_t *str_copy(vm_t *vm, const char *chars, int length, bool ignorecase);

fun_t *fun_new(vm_t *vm, src_t *source);

map_t *map_new(vm_t *vm);
void map_set(vm_t *vm, map_t *map, const char *key, val_t value);

const char *obj_typeof(obj_t *object);
void obj_print(obj_t *object);
void obj_free(gc_t *gc, obj_t *object);
