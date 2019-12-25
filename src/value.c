#include <stdlib.h>

#include "value.h"

void au3_initValueArray(au3ValueArray *array)
{
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void au3_freeValueArray(au3ValueArray* array)
{
    free(array->values);

    au3_initValueArray(array);
}

void au3_writeValueArray(au3ValueArray *array, au3Value value)
{
    if (array->capacity < array->count + 1) {
        array->capacity = (array->capacity < 8) ? 8 : array->capacity * 2;
        array->values = realloc(array->values, sizeof(au3Value) * array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}
