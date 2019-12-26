#include <stdio.h>
#include <string.h>

#include "compiler.h"
#include "vm.h"

typedef struct {
    au3Token current;
    au3Token previous;
    bool hadError;
    bool panicMode;
} Parser;

static Parser parser;
static au3Chunk *compilingChunk;

static au3Chunk *currentChunk()
{
    return compilingChunk;
}

static void errorAt(au3Token *token, const char* message)
{
    if (parser.panicMode) return;
    parser.panicMode = true;

    fprintf(stderr, "[%d:%d] Error", token->line, token->column);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    }
    else if (token->type == TOKEN_ERROR) {
        // Nothing.                                                
    }
    else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void error(const char *message)
{
    errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char *message)
{
    errorAt(&parser.current, message);
}

static void advance()
{
    parser.previous = parser.current;

    for (;;) {
        parser.current = au3_scanToken();
        if (parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

static void consume(au3TokenType type, const char* message)
{
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static void emitByte(uint8_t byte)
{
    au3_writeChunk(currentChunk(),
        byte, parser.previous.line, parser.previous.column);
}

static void emitBytes(uint8_t byte1, uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn()
{
    emitByte(OP_RET);
}

static void endCompiler()
{
    emitReturn();
}

static void expression()
{
    // What goes here?      
}

bool au3_compile(const char *source, au3Chunk *chunk)
{
    au3_initLexer(source);
    compilingChunk = chunk;

    parser.hadError = false;
    parser.panicMode = false;

    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");

    endCompiler();

    return !parser.hadError;
}