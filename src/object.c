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
    au3Object *object = (au3Object *)au3_reallocate(vm, NULL, 0, size);
    object->type = type;
    object->isMarked = false;

    object->next = vm->objects;
    vm->objects = object;
    return object;
}

static au3String *allocateString(au3VM *vm, char *chars, int length, uint32_t hash)
{
    au3String *string = ALLOCATE_OBJ(au3String, AU3_TSTRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    au3_tableSet(&vm->strings, string, AU3_NULL);

    return string;
}

static uint32_t hashString(const char* key, int length)
{
    static const uint32_t basis = 2166136261u;
    static const uint32_t prime = 16777619;

    uint32_t hash = basis;

    for (int i = 0; i < length; i++) {
        hash ^= key[i];
        hash *= prime;
    }

    return hash;
}

au3String *au3_takeString(au3VM *vm, char *chars, int length)
{
    uint32_t hash = hashString(chars, length);
    au3String *interned = au3_tableFindString(&vm->strings, chars, length, hash);
    if (interned != NULL) {
        free(chars);
        return interned;
    }

    return allocateString(vm, chars, length, hash);
}

au3String *au3_copyString(au3VM *vm, const char *chars, int length)
{
    uint32_t hash = hashString(chars, length);
    au3String *interned = au3_tableFindString(&vm->strings, chars, length, hash);
    if (interned != NULL) return interned;

    char *heapChars = malloc(sizeof(char) * (length + 1));
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(vm, heapChars, length, hash);
}

au3Function *au3_newFunction(au3VM *vm)
{
    au3Function *function = ALLOCATE_OBJ(au3Function, AU3_TFUNCTION);

    function->arity = 0;
    function->upvalueCount = 0;
    function->upvalues = NULL;
    function->name = NULL;
    au3_initChunk(&function->chunk);

    return function;
}

void au3_makeClosure(au3Function *function)
{
    int upvalueCount = function->upvalueCount;
    au3Upvalue **upvalues = malloc(sizeof(au3Upvalue *) * upvalueCount);

    for (int i = 0; i < upvalueCount; i++) {
        upvalues[i] = NULL;
    }

    function->upvalues = upvalues;
}

au3Upvalue *au3_newUpvalue(au3VM *vm, au3Value *slot)
{
    au3Upvalue *upvalue = ALLOCATE_OBJ(au3Upvalue, AU3_TUPVALUE);
    upvalue->location = slot;
    upvalue->next = NULL;

    return upvalue;
}

au3Native *au3_newNative(au3VM *vm, au3NativeFn function, const char *tips)
{
    au3Native *native = ALLOCATE_OBJ(au3Native, AU3_TNATIVE);
    native->function = function;
    if (tips != NULL) {
        size_t length = strlen(tips);
        native->tips = malloc(sizeof(char) * (length + 1));
        memcpy(native->tips, '\0', length);
        native->tips[length] = '\0';
    }

    return native;
}

#define OBJECT_TYPE(o)  ((o)->type)

#define AS_STRING(o)    ((au3String *)(o))
#define AS_CSTRING(o)   (((au3String *)(o))->chars)
#define AS_FUNCTION(o)  ((au3Function *)(o))
#define AS_NATIVE(o)    ((au3Native *)(o))

static void printFunction(au3Function *function)
{
    if (function->name == NULL) {
        printf("<script>");
        return;
    }
    printf("func: <%s>", function->name->chars);
}

static void prinNative(au3Native *native)
{
    printf("func: <native>");
    if (native->tips) printf("(%s)", native->tips);
}

void au3_printObject(au3Object *object)
{
    switch (OBJECT_TYPE(object)) {
        case AU3_TSTRING:
            printf("%s", AS_CSTRING(object));
            break;
        case AU3_TFUNCTION:
            printFunction(AS_FUNCTION(object));
            break;
        case AU3_TNATIVE:
            prinNative(AS_NATIVE(object));
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
