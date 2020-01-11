#ifndef _AU3_GC_H
#define _AU3_GC_H
#pragma once

#include "common.h"
#include "object.h"

struct _gc {
    vm_t *vm;
    size_t allocated;
    size_t nextGC;
    obj_t *objects;
    obj_t **grayStack;
    int grayCount;
    int grayCapacity;
};

void gc_init(gc_t *gc);
void gc_free(gc_t *gc);

void *gc_realloc(gc_t *gc, void *ptr, size_t old, size_t new);
void gc_collect(gc_t *gc);

#endif
