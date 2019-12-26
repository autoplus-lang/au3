#include <stdio.h>

#include "debug.h"
#include "value.h"

void au3_disassembleChunk(au3Chunk *chunk, const char* name)
{
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->count;) {
        offset = au3_disassembleInstruction(chunk, offset);
    }
}

static int constantInstruction(const char *name, au3Chunk *chunk, int offset)
{
    uint8_t constant = chunk->code[offset + 1];
    printf("%-16s %4d '", name, constant);
    au3_printValue(chunk->constants.values[constant]);
    printf("'\n");

    return offset + 2;
}

static int simpleInstruction(const char *name, int offset)
{
    printf("%s\n", name);
    return offset + 1;
}

static int byteInstruction(const char *name, au3Chunk *chunk, int offset)
{
    uint8_t slot = chunk->code[offset + 1];
    printf("%-16s %4d\n", name, slot);
    return offset + 2;
}

static int jumpInstruction(const char *name, int sign, au3Chunk *chunk, int offset)
{
    uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
    jump |= chunk->code[offset + 2];
    printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
    return offset + 3;
}

int au3_disassembleInstruction(au3Chunk *chunk, int offset)
{
    printf("%04d ", offset);

    uint8_t instruction = chunk->code[offset];

    switch (instruction) {

        case OP_NOP:
            return simpleInstruction("OP_NOP", offset);

        case OP_CONST:
            return constantInstruction("OP_CONST", chunk, offset);

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

        case OP_DEF:
            return constantInstruction("OP_DEF", chunk, offset);
        case OP_GLD:
            return constantInstruction("OP_GLD", chunk, offset);
        case OP_GST:
            return constantInstruction("OP_GST", chunk, offset);

        case OP_LD:
            return byteInstruction("OP_LD", chunk, offset);
        case OP_ST:
            return byteInstruction("OP_ST", chunk, offset);

        case OP_JMP:
            return jumpInstruction("OP_JMP", 1, chunk, offset);
        case OP_JMPF:
            return jumpInstruction("OP_JMPF", 1, chunk, offset);

        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}
