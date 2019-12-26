#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "object.h"
#include "memory.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type *)allocateObject(sizeof(type), objectType)

static au3Object *allocateObject(size_t size, au3ObjectType type)
{
    au3Object *object = (au3Object *)au3_reallocate(NULL, 0, size);
    object->type = type;

    return object;
}

static au3String *allocateString(char *chars, int length)
{
    au3String *string = ALLOCATE_OBJ(au3String, AU3_TSTRING);
    string->length = length;
    string->chars = chars;

    return string;
}

au3String *au3_copyString(const char *chars, int length)
{
    char *heapChars = malloc(sizeof(char) * (length + 1));
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length);
}

#define OBJECT_TYPE(o)  ((o)->type)

#define AS_STRING(o)    ((au3String *)(o))
#define AS_CSTRING(o)   (((au3String *)(o))->chars)

void au3_printObject(au3Object *object)
{
    switch (OBJECT_TYPE(object)) {
        case AU3_TSTRING:
            printf("%s", AS_CSTRING(object));
            break;
        default:
            printf("object: %p", object);
            break;
    }
}

const char *au3_typeofObject(au3Object *object)
{
    switch (OBJECT_TYPE(object)) {
        case AU3_TSTRING:
            return "string";
        default:
            return "object";
    }
}
