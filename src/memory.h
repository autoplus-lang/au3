#ifndef _AU3_MEMORY_H
#define _AU3_MEMORY_H
#pragma once

#include "common.h"
#include "vm.h"

#define AU3_GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(previous, type, oldCount, count) \
    (type *)au3_reallocate(vm, previous, sizeof(type) * (oldCount), sizeof(type) * (count))

#define FREE_ARRAY(type, pointer, oldCount) \
    au3_reallocate(vm, pointer, sizeof(type) * (oldCount), 0)

#define AU3_ALLOCATE(type, count) \
    (type *)au3_reallocate(vm, NULL, 0, sizeof(type) * (count))

#define AU3_FREE(type, pointer) \
    au3_reallocate(vm, pointer, sizeof(type), 0)

void *au3_reallocate(au3VM *vm, void *previous, size_t oldSize, size_t newSize);
void au3_collectGarbage(au3VM *vm);
void au3_freeObjects(au3VM *vm);

void au3_markValue(au3VM *vm, au3Value value);
void au3_markObject(au3VM *vm, au3Object *object);

#endif