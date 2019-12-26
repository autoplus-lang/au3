#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "vm.h"
#include "object.h"
#include "debug.h"

typedef struct {
    au3Token current;
    au3Token previous;
    bool hadError;
    bool panicMode;
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =        
    PREC_OR,          // or       
    PREC_AND,         // and      
    PREC_EQUALITY,    // == !=    
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -      
    PREC_FACTOR,      // * /      
    PREC_UNARY,       // ! -      
    PREC_CALL,        // . ()     
    PREC_PRIMARY
} Precedence;

typedef void (* ParseFn)(bool canAssign);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

static Parser parser;
static au3Chunk *compilingChunk;
static au3VM *runningVM;

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

static bool check(au3TokenType type)
{
    return parser.current.type == type;
}

static bool match(au3TokenType type)
{
    if (!check(type)) return false;
    advance();
    return true;
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

static uint8_t makeConstant(au3Value value)
{
    int constant = au3_addConstant(currentChunk(), value);
    if (constant > AU3_MAX_CONST) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

static void emitConstant(au3Value value)
{
    emitBytes(OP_CONST, makeConstant(value));
}

static void endCompiler()
{
    emitReturn();

    if (!parser.hadError) {
        au3_disassembleChunk(currentChunk(), "code");
        printf("==========\n\n");
    }
}

static void expression();
static void statement();
static void declaration();
static ParseRule *getRule(au3TokenType type);
static void parsePrecedence(Precedence precedence);

static uint8_t identifierConstant(au3Token* name)
{
    return makeConstant(AU3_OBJECT(au3_copyString(runningVM, name->start, name->length)));
}

static uint8_t parseVariable(const char *errorMessage)
{
    consume(TOKEN_IDENTIFIER, errorMessage);
    return identifierConstant(&parser.previous);
}

static void defineVariable(uint8_t global)
{
    emitBytes(OP_DEF, global);
}

static void binary(bool canAssign)
{
    // Remember the operator.                                
    au3TokenType operatorType = parser.previous.type;

    // Compile the right operand.                            
    ParseRule *rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    // Emit the operator instruction.                        
    switch (operatorType) {
        case TOKEN_EQUAL_EQUAL:   emitByte(OP_EQ); break;
        case TOKEN_LESS:          emitByte(OP_LT); break;
        case TOKEN_LESS_EQUAL:    emitByte(OP_LE); break;

        case TOKEN_BANG_EQUAL:    emitBytes(OP_EQ, OP_NOT); break;    
        case TOKEN_GREATER:       emitBytes(OP_LE, OP_NOT); break;
        case TOKEN_GREATER_EQUAL: emitBytes(OP_LT, OP_NOT); break;

        case TOKEN_PLUS:          emitByte(OP_ADD); break;
        case TOKEN_MINUS:         emitByte(OP_SUB); break;
        case TOKEN_STAR:          emitByte(OP_MUL); break;
        case TOKEN_SLASH:         emitByte(OP_DIV); break;
        default:
            return; // Unreachable.                              
    }
}

static void literal(bool canAssign)
{
    switch (parser.previous.type) {
        case TOKEN_FALSE:   emitByte(OP_FALSE); break;
        case TOKEN_NULL:    emitByte(OP_NULL); break;
        case TOKEN_TRUE:    emitByte(OP_TRUE); break;
        default:
            return; // Unreachable.                   
    }
}

static void grouping(bool canAssign)
{
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(bool canAssign)
{
    double value = strtod(parser.previous.start, NULL);
    emitConstant(AU3_NUMBER(value));
}

static void string(bool canAssign)
{
    emitConstant(AU3_OBJECT(au3_copyString(runningVM, parser.previous.start + 1,
        parser.previous.length - 2)));
}

static void namedVariable(au3Token name, bool canAssign)
{
    uint8_t arg = identifierConstant(&name);

    if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(OP_GST, arg);
    }
    else {
        emitBytes(OP_GLD, arg);
    }
}

static void variable(bool canAssign)
{
    namedVariable(parser.previous, canAssign);
}

static void unary(bool canAssign)
{
    au3TokenType operatorType = parser.previous.type;

    // Compile the operand.                        
    parsePrecedence(PREC_UNARY);

    // Emit the operator instruction.              
    switch (operatorType) {
        case TOKEN_BANG:    emitByte(OP_NOT); break;
        case TOKEN_MINUS:   emitByte(OP_NEG); break;
        default:
            return; // Unreachable.                    
    }
}

static ParseRule rules[] = {
    [TOKEN_LEFT_PAREN]      = { grouping, NULL,    PREC_NONE },
    [TOKEN_RIGHT_PAREN]     = { NULL,     NULL,    PREC_NONE },
    [TOKEN_LEFT_BRACE]      = { NULL,     NULL,    PREC_NONE },
    [TOKEN_RIGHT_BRACE]     = { NULL,     NULL,    PREC_NONE },

    [TOKEN_COMMA]           = { NULL,     NULL,    PREC_NONE },
    [TOKEN_DOT]             = { NULL,     NULL,    PREC_NONE },
    [TOKEN_MINUS]           = { unary,    binary,  PREC_TERM },
    [TOKEN_PLUS]            = { NULL,     binary,  PREC_TERM },
    [TOKEN_SEMICOLON]       = { NULL,     NULL,    PREC_NONE },
    [TOKEN_SLASH]           = { NULL,     binary,  PREC_FACTOR },
    [TOKEN_STAR]            = { NULL,     binary,  PREC_FACTOR },

    [TOKEN_BANG]            = { unary,    NULL,    PREC_NONE },
    [TOKEN_BANG_EQUAL]      = { NULL,     binary,  PREC_EQUALITY },
    [TOKEN_EQUAL]           = { NULL,     NULL,    PREC_NONE },
    [TOKEN_EQUAL_EQUAL]     = { NULL,     binary,  PREC_EQUALITY },
    [TOKEN_GREATER]         = { NULL,     binary,  PREC_COMPARISON },
    [TOKEN_GREATER_EQUAL]   = { NULL,     binary,  PREC_COMPARISON },
    [TOKEN_LESS]            = { NULL,     binary,  PREC_COMPARISON },
    [TOKEN_LESS_EQUAL]      = { NULL,     binary,  PREC_COMPARISON },

    [TOKEN_IDENTIFIER]      = { variable, NULL,    PREC_NONE },
    [TOKEN_STRING]          = { string,   NULL,    PREC_NONE },
    [TOKEN_NUMBER]          = { number,   NULL,    PREC_NONE },

    [TOKEN_AND]             = { NULL,     NULL,    PREC_NONE },
    [TOKEN_CLASS]           = { NULL,     NULL,    PREC_NONE },
    [TOKEN_ELSE]            = { NULL,     NULL,    PREC_NONE },
    [TOKEN_FALSE]           = { literal,  NULL,    PREC_NONE },
    [TOKEN_FOR]             = { NULL,     NULL,    PREC_NONE },
    [TOKEN_FUN]             = { NULL,     NULL,    PREC_NONE },
    [TOKEN_IF]              = { NULL,     NULL,    PREC_NONE },
    [TOKEN_NULL]            = { literal,  NULL,    PREC_NONE },
    [TOKEN_OR]              = { NULL,     NULL,    PREC_NONE },
    [TOKEN_PUTS]            = { NULL,     NULL,    PREC_NONE },
    [TOKEN_RETURN]          = { NULL,     NULL,    PREC_NONE },
    [TOKEN_SUPER]           = { NULL,     NULL,    PREC_NONE },
    [TOKEN_THIS]            = { NULL,     NULL,    PREC_NONE },
    [TOKEN_TRUE]            = { literal,  NULL,    PREC_NONE },
    [TOKEN_VAR]             = { NULL,     NULL,    PREC_NONE },
    [TOKEN_WHILE]           = { NULL,     NULL,    PREC_NONE },

    [TOKEN_ERROR]           = { NULL,     NULL,    PREC_NONE },
    [TOKEN_EOF]             = { NULL,     NULL,    PREC_NONE },
};

static void parsePrecedence(Precedence precedence)
{
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL) {
        error("Expect expression.");
        return;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(canAssign);
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        error("Invalid assignment target.");
    }
}

static ParseRule *getRule(au3TokenType type)
{
    return &rules[type];
}

static void expression()
{
    parsePrecedence(PREC_ASSIGNMENT);
}

static void varDeclaration()
{
    uint8_t global = parseVariable("Expect variable name.");

    if (match(TOKEN_EQUAL)) {
        expression();
    }
    else {
        emitByte(OP_NULL);
    }
    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    defineVariable(global);
}

static void expressionStatement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");

    emitByte(OP_POP);
}

static void putsStatement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");

    emitByte(OP_PUTS);
}

static void synchronize()
{
    parser.panicMode = false;

    while (parser.current.type != TOKEN_EOF) {
        if (parser.previous.type == TOKEN_SEMICOLON) return;

        switch (parser.current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PUTS:
            case TOKEN_RETURN:
                return;

            default:
                // Do nothing.                                  
                ;
        }

        advance();
    }
}

static void declaration()
{
    if (match(TOKEN_VAR)) {
        varDeclaration();
    }
    else {
        statement();
    }

    if (parser.panicMode) synchronize();
}

static void statement()
{
    if (match(TOKEN_PUTS)) {
        putsStatement();
    }
    else {
        expressionStatement();
    }
}

bool au3_compile(au3VM *vm, const char *source, au3Chunk *chunk)
{
    au3_initLexer(source);
    compilingChunk = chunk;
    runningVM = vm;

    parser.hadError = false;
    parser.panicMode = false;

    advance();

    while (!match(TOKEN_EOF)) {
        declaration();
    }

    endCompiler();

    return !parser.hadError;
}
