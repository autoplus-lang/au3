#ifndef _AU3_MEMORY_H
#define _AU3_MEMORY_H
#pragma once

#include "common.h"

#define AU3_GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(previous, type, oldCount, count) \
    (type *)au3_reallocate(previous, sizeof(type) * (oldCount), sizeof(type) * (count))

#define FREE_ARRAY(type, pointer, oldCount) \
    au3_reallocate(pointer, sizeof(type) * (oldCount), 0)

#define AU3_ALLOCATE(type, count) \
    (type *)au3_reallocate(NULL, 0, sizeof(type) * (count))

void *au3_reallocate(void *previous, size_t oldSize, size_t newSize);

#endif