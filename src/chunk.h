#ifndef _AU3_CHUNK_H
#define _AU3_CHUNK_H
#pragma once

#include "common.h"

typedef struct {
    int count;
    int capacity;
    uint8_t *code;
    int *lines;
} au3Chunk;

#define AU3_CODE_PAGE   256

void au3_initChunk(au3Chunk *chunk);
void au3_freeChunk(au3Chunk *chunk);
void au3_writeChunk(au3Chunk *chunk, uint8_t code, int line);

#endif
