#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "object.h"
#include "memory.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type *)allocateObject(vm, sizeof(type), objectType)

static au3Object *allocateObject(au3VM *vm, size_t size, au3ObjectType type)
{
    au3Object *object = (au3Object *)au3_reallocate(NULL, 0, size);
    object->type = type;

    object->next = vm->objects;
    vm->objects = object;
    return object;
}

static au3String *allocateString(au3VM *vm, char *chars, int length)
{
    au3String *string = ALLOCATE_OBJ(au3String, AU3_TSTRING);
    string->length = length;
    string->chars = chars;

    return string;
}

au3String *au3_takeString(au3VM *vm, char *chars, int length)
{
    return allocateString(vm, chars, length);
}

au3String *au3_copyString(au3VM *vm, const char *chars, int length)
{
    char *heapChars = malloc(sizeof(char) * (length + 1));
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(vm, heapChars, length);
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
