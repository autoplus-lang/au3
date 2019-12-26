#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "compiler.h"

#define GC_HEAP_GROW_FACTOR     2

void *au3_reallocate(au3VM *vm, void *previous, size_t oldSize, size_t newSize)
{
    vm->bytesAllocated += newSize - oldSize;

    if (newSize > oldSize) {
#ifdef DEBUG_STRESS_GC                                            
        collectGarbage();
#endif                      

        if (vm->bytesAllocated > vm->nextGC) {
            au3_collectGarbage(vm);
        }
    }

    if (newSize == 0) {
        free(previous);
        return NULL;
    }

    return realloc(previous, newSize);
}

void au3_markObject(au3VM *vm, au3Object *object)
{
    if (object == NULL) return;
    if (object->isMarked) return;
    object->isMarked = true;

    if (vm->grayCapacity < vm->grayCount + 1) {
        vm->grayCapacity = AU3_GROW_CAPACITY(vm->grayCapacity);
        vm->grayStack = realloc(vm->grayStack,
            sizeof(au3Object *) * vm->grayCapacity);
    }

    vm->grayStack[vm->grayCount++] = object;
}

void au3_markValue(au3VM *vm, au3Value value)
{
    if (!AU3_IS_OBJECT(value)) return;
    au3_markObject(vm, AU3_AS_OBJECT(value));
}

static void markArray(au3VM *vm, au3ValueArray *array)
{
    for (int i = 0; i < array->count; i++) {
        au3_markValue(vm, array->values[i]);
    }
}

static void blackenObject(au3VM *vm, au3Object *object)
{
    switch (object->type) {
        case AU3_TFUNCTION: {
            au3Function *function = (au3Function *)object;
            au3_markObject(vm, (au3Object *)function->name);
            markArray(vm, &function->chunk.constants);
            for (int i = 0; i < function->upvalueCount; i++) {
                au3_markObject(vm, (au3Object *)function->upvalues[i]);
            }
            break;
        }

        case AU3_TUPVALUE:
            au3_markValue(vm, ((au3Upvalue *)object)->closed);
            break;

        case AU3_TNATIVE:
        case AU3_TSTRING:
            break;
    }
}

static void freeObject(au3VM *vm, au3Object *object)
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
            free(function->upvalues);
            AU3_FREE(au3Function, object);
            break;
        }
        case AU3_TNATIVE: {
            au3Native *native = (au3Native *)object;
            if (native->tips) free(native->tips);
            AU3_FREE(au3Native, object);
            break;
        }
        case AU3_TUPVALUE: {
            AU3_FREE(au3Upvalue, object);
            break;
        }
    }
}

static void markRoots(au3VM *vm)
{
    for (au3Value *slot = vm->stack; slot < vm->top; slot++) {
        au3_markValue(vm, *slot);
    }

    for (int i = 0; i < vm->frameCount; i++) {
        au3_markObject(vm, (au3Object *)vm->frames[i].function);
    }

    for (au3Upvalue *upvalue = vm->openUpvalues;
        upvalue != NULL;
        upvalue = upvalue->next) {
        au3_markObject(vm, (au3Object *)upvalue);
    }

    au3_markTable(vm, &vm->globals);
    au3_markCompilerRoots(vm);
}

static void traceReferences(au3VM *vm)
{
    while (vm->grayCount > 0) {
        au3Object *object = vm->grayStack[--vm->grayCount];
        blackenObject(vm, object);
    }
}

static void sweep(au3VM *vm)
{
    au3Object *previous = NULL;
    au3Object *object = vm->objects;

    while (object != NULL) {
        if (object->isMarked) {
            object->isMarked = false;
            previous = object;
            object = object->next;
        }
        else {
            au3Object *unreached = object;

            object = object->next;
            if (previous != NULL) {
                previous->next = object;
            }
            else {
                vm->objects = object;
            }

            freeObject(vm, unreached);
        }
    }
}

void au3_collectGarbage(au3VM *vm)
{
    markRoots(vm);
    traceReferences(vm);
    au3_tableRemoveWhite(&vm->strings);
    sweep(vm);

    vm->nextGC = vm->bytesAllocated * GC_HEAP_GROW_FACTOR;
}

void au3_freeObjects(au3VM *vm)
{
    au3Object *object = vm->objects;

    while (object != NULL) {
        au3Object *next = object->next;
        freeObject(vm, object);
        object = next;
    }

    free(vm->grayStack);
}
