#ifndef _AU3_VM_H
#define _AU3_VM_H
#pragma once

#include "chunk.h"

typedef struct {
    uint8_t *ip;
    au3Chunk *chunk;
} au3VM;

au3VM *au3_create();
void au3_close(au3VM *vm);

#endif
