#include <stdlib.h>
#include <string.h>

#include "memory.h"

void *au3_reallocate(void *previous, size_t oldSize, size_t newSize)
{
    if (newSize == 0) {
        free(previous);
        return NULL;
    }

    return realloc(previous, newSize);
}