#include <stdlib.h>

#include "chunk.h"

void au3_initChunk(au3Chunk *chunk)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;

    au3_initValueArray(&chunk->constants);
}

void au3_freeChunk(au3Chunk *chunk)
{
    free(chunk->code);
    free(chunk->lines);

    au3_freeValueArray(&chunk->constants);
    au3_initChunk(chunk);
}

void au3_writeChunk(au3Chunk *chunk, uint8_t byte, int line)
{
    if (chunk->capacity < chunk->count + 1) {
        chunk->capacity += AU3_CODE_PAGE;
        chunk->code = realloc(chunk->code, sizeof(uint8_t) * chunk->capacity);
        chunk->lines = realloc(chunk->lines, sizeof(int) * chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->code[chunk->count] = line;
    chunk->count++;
}

int au3_addConstant(au3Chunk *chunk, au3Value value)
{
    au3_writeValueArray(&chunk->constants, value);

    return chunk->constants.count - 1;
}