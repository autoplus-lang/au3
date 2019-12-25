#ifndef _AU3_DEBUG_H
#define _AU3_DEBUG_H
#pragma once

#include "chunk.h"

void au3_disassembleChunk(au3Chunk *chunk, const char *name);
int au3_disassembleInstruction(au3Chunk *chunk, int offset);

#endif
