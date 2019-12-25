#ifndef _AU3_VM_H
#define _AU3_VM_H
#pragma once

#include "chunk.h"

typedef struct {
    uint8_t *ip;
    au3Chunk *chunk;

    au3Value *top;
    au3Value stack[256];

} au3VM;

typedef enum {
    AU3_OK = 0,
    AU3_COMPILE_ERROR,
    AU3_RUNTIME_ERROR
} au3Status;

au3VM *au3_create();
void au3_close(au3VM *vm);
au3Status au3_interpret(au3VM *vm, const char *source);

#endif
