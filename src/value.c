#include <stdio.h>
#include <stdlib.h>

#include "value.h"

const char *au3_typeofValue(au3Value value)
{
    switch (value.type) {
        default:
        case AU3_TNULL:
            return "null";
        case AU3_TBOOL:
            return "bool";
        case AU3_TINTEGER:
            return "integer";
        case AU3_TNUMBER:
            return "number";
    }
}

void au3_printValue(au3Value value)
{
    switch (value.type) {
        case AU3_TNULL:
            printf("null");
            break;
        case AU3_TBOOL:
            printf(AU3_AS_BOOL(value) ? "true" : "false");
            break;
        case AU3_TINTEGER:
            printf("%lld", AU3_AS_INTEGER(value));
            break;
        case AU3_TNUMBER:
            printf("%.14g", AU3_AS_NUMBER(value));
            break;
    }
}

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
