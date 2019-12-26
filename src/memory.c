#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"

void *au3_reallocate(void *previous, size_t oldSize, size_t newSize)
{
    if (newSize == 0) {
        free(previous);
        return NULL;
    }

    return realloc(previous, newSize);
}

static void freeObject(au3Object *object)
{
    switch (object->type) {
        case AU3_TSTRING: {
            au3String *string = (au3String *)object;
            free(string->chars);
            AU3_FREE(au3String, object);
            break;
        }
        case AU3_TFUNCTION: {
            au3Function *function = (au3Function *)object;
            au3_freeChunk(&function->chunk);
            AU3_FREE(au3Function, object);
            break;
        }
    }
}

void au3_freeObjects(au3VM *vm)
{
    au3Object *object = vm->objects;

    while (object != NULL) {
        au3Object *next = object->next;
        freeObject(object);
        object = next;
    }
}
