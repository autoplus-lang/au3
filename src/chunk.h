#ifndef _AU3_CHUNK_H
#define _AU3_CHUNK_H
#pragma once

#include "common.h"
#include "value.h"

typedef enum {

    OP_NOP,

    OP_PUTS,

    OP_RET,

    OP_NULL,
    OP_TRUE,
    OP_FALSE,
    OP_CONST,

    OP_NEG,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_POW,

    OP_NOT,
    OP_LT,
    OP_LE,
    OP_EQ,

    OP_BNOT,
    OP_BOR,
    OP_BAND,
    OP_BXOR,

    OP_SHL,
    OP_SHR,

} au3Opcode;

typedef struct {
    int count;
    int capacity;
    uint8_t *code;
    int *lines;
    int *columns;
    au3ValueArray constants;
} au3Chunk;

#define AU3_CODE_PAGE   256

void au3_initChunk(au3Chunk *chunk);
void au3_freeChunk(au3Chunk *chunk);
void au3_writeChunk(au3Chunk *chunk, uint8_t code, int line, int column);
int au3_addConstant(au3Chunk *chunk, au3Value value);

#endif
