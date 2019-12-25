#include <stdio.h>

#include "debug.h"

void au3_disassembleChunk(au3Chunk *chunk, const char* name)
{
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }
}

static int simpleInstruction(const char* name, int offset)
{
    printf("%s\n", name);
    return offset + 1;
}

int au3_disassembleInstruction(au3Chunk *chunk, int offset)
{
    printf("%04d ", offset);

    uint8_t instruction = chunk->code[offset];

    switch (instruction) {
        case OP_RET:
            return simpleInstruction("OP_RET", offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}
