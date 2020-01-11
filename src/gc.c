#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gc.h"
#include "vm.h"
#include "object.h"

void gc_init(gc_t *gc)
{
    gc->allocated = 0;
    gc->nextGC = 512 * 1024;
    gc->objects = NULL;

    gc->grayCount = 0;
    gc->grayCapacity = 0;
    gc->grayStack = NULL;
}

void gc_free(gc_t *gc)
{
    obj_t *object = gc->objects;

    while (object != NULL) {
        obj_t *next = object->next;
        obj_free(gc, object);
        object = next;
    }

    free(gc->grayStack);
}

void *gc_realloc(gc_t *gc, void *ptr, size_t old, size_t new)
{
    gc->allocated += new - old;

    if (new > old && gc->allocated > gc->nextGC) {
        gc_collect(gc);
    }

    if (new == 0) {
        free(ptr);
        return NULL;
    }

    return realloc(ptr, new);
}

static void markObject(gc_t *gc, obj_t *object)
{
    if (object == NULL) return;
    if (object->isMarked) return;
    object->isMarked = true;

    if (gc->grayCapacity < gc->grayCount + 1) {
        gc->grayCapacity = GROW_CAP(gc->grayCapacity);
        gc->grayStack = realloc(gc->grayStack,
            gc->grayCapacity * sizeof(obj_t *));
    }

    gc->grayStack[gc->grayCount++] = object;
}

static inline void markValue(gc_t *gc, val_t value)
{
    if (IS_OBJ(value)) markObject(gc, AS_OBJ(value));
}

static void markTable(gc_t *gc, tab_t *table)
{
    for (int i = 0; i < table->capacity; i++) {
        ent_t *entry = &table->entries[i];
        markObject(gc, (obj_t *)entry->key);
        markValue(gc, entry->value);
    }
}

static void mark_hash(gc_t *gc, hash_t *hash)
{
    for (int i = 0; i < hash->capacity; i++) {
        markValue(gc, hash->indexes[i].value);
    }
}

static void mark_array(gc_t *gc, arr_t *array)
{
    for (int i = 0; i < array->count; i++) {
        markValue(gc, array->values[i]);
    }
}

static void blackenObject(gc_t *gc, obj_t *object)
{
    switch (object->type) {
        case OT_STR:
            break;
        case OT_UPV:
            markValue(gc, ((upv_t *)object)->closed);
            break;
        case OT_FUN: {
            fun_t *function = (fun_t *)object;
            markObject(gc, (obj_t *)function->name);
            mark_array(gc, &function->chunk.constants);
            for (int i = 0; i < function->upvalueCount; i++) {
                markObject(gc, (obj_t *)function->upvalues[i]);
            }
            break;
        }
        case OT_MAP: {
            map_t *map = (map_t *)object;
            markTable(gc, &map->table);
            mark_hash(gc, &map->hash);
            break;
        }
    }
}

static void markRoots(vm_t *vm)
{
    gc_t *gc = vm->gc;

    for (int i = 0; i < vm->numRoots; i++) {
        markObject(gc, vm->tempRoots[i]);
    }

    for (val_t *slot = vm->stack; slot < vm->top; slot++) {
        markValue(gc, *slot);
    }

    for (int i = 0; i < vm->frameCount; i++) {
        markObject(gc, (obj_t *)vm->frames[i].function);
    }

    for (upv_t *upvalue = vm->openUpvalues;
        upvalue != NULL;
        upvalue = upvalue->next) {
        markObject(gc, (obj_t *)upvalue);
    }

    //mark_compiler(vm);
}

static void traceReferences(gc_t *gc)
{
    while (gc->grayCount > 0) {
        obj_t *obj = gc->grayStack[--gc->grayCount];
        blackenObject(gc, obj);
    }
}

static void sweep(gc_t *gc)
{
    obj_t *prev = NULL;
    obj_t *obj = gc->objects;

    while (obj != NULL) {
        if (!obj->isMarked) {
            obj->isMarked = false;
            prev = obj;
            obj = obj->next;
        }
        else {
            obj_t *unreached = obj;
            obj = obj->next;

            if (prev != NULL) {
                prev->next = obj;
            }
            else {
                gc->objects = obj;
            }

            obj_free(gc, unreached);
        }
    }
}

static void removeWhite(tab_t *table)
{
    for (int i = 0; i < table->capacity; i++) {
        ent_t *entry = &table->entries[i];
        if (entry->key != NULL && !entry->key->obj.isMarked) {
            tab_remove(table, entry->key);
        }
    }
}

void gc_collect(gc_t *gc)
{
    vm_t *vm = gc->vm;

    markRoots(vm);
    markTable(gc, vm->globals);
    traceReferences(gc);
    removeWhite(vm->strings);
    sweep(gc);

    gc->nextGC = gc->allocated * 2;
}
