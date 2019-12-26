#include <stdio.h>

#include "debug.h"

void au3_disassembleChunk(au3Chunk *chunk, const char* name)
{
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->count;) {
        offset = au3_disassembleInstruction(chunk, offset);
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

        case OP_NOP:
            return simpleInstruction("OP_NOP", offset);

        case OP_PUTS:
            return simpleInstruction("OP_PUTS", offset);
        case OP_POP:
            return simpleInstruction("OP_POP", offset);

        case OP_RET:
            return simpleInstruction("OP_RET", offset);

        case OP_NEG:
            return simpleInstruction("OP_NEG", offset);
        case OP_ADD:
            return simpleInstruction("OP_ADD", offset);
        case OP_SUB:
            return simpleInstruction("OP_SUB", offset);
        case OP_MUL:
            return simpleInstruction("OP_MUL", offset);
        case OP_DIV:
            return simpleInstruction("OP_DIV", offset);

        case OP_NOT:
            return simpleInstruction("OP_NOT", offset);
        case OP_EQ:
            return simpleInstruction("OP_EQ", offset);
        case OP_LT:
            return simpleInstruction("OP_LT", offset);
        case OP_LE:
            return simpleInstruction("OP_LE", offset);

        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}
