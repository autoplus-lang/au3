#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "value.h"
#include "object.h"

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
        case AU3_TOBJECT:
            return au3_typeofObject(AU3_AS_OBJECT(value));
    }
}

void au3_printValue(au3Value value)
{
    switch (value.type) {
        default:
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
        case AU3_TOBJECT:
            au3_printObject(AU3_AS_OBJECT(value));
            break;
    }
}

bool au3_valuesEqual(au3Value a, au3Value b)
{
    if (a.type != b.type) return false;

    switch (a.type) {
        case AU3_TNULL:     return true;
        case AU3_TBOOL:     return AU3_AS_BOOL(a) == AU3_AS_BOOL(b);
        case AU3_TINTEGER:  return AU3_AS_INTEGER(a) == AU3_AS_INTEGER(b);
        case AU3_TNUMBER:   return AU3_AS_NUMBER(a) == AU3_AS_NUMBER(b);
        case AU3_TOBJECT: {
            au3String *aString = AU3_AS_STRING(a);
            au3String *bString = AU3_AS_STRING(b);
            return aString->length == bString->length &&
                memcmp(aString->chars, bString->chars, aString->length) == 0;
        }
    }

    return false;
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
