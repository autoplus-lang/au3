#pragma once

#include "common.h"
#include "value.h"

#define OPCODES() \
/*        opcodes      args     stack       description */ \
    _CODE(PRINT)   	/* []       [-1, +0]    pop a value from stack */ \
    _CODE(POP)     	/* []       [-1, +0]    pop a value from stack and print it */ \
    _CODE(CALL)    	/* [n]      [-n, +1]    */ \
    _CODE(RET)     	/* []       [-1, +0]    */ \
    _CODE(NIL)     	/* []       [-0, +1]    push nil to stack */ \
    _CODE(TRUE)    	/* []       [-0, +1]    push true to stack */ \
    _CODE(FALSE)   	/* []       [-0, +1]    push false to stack */ \
    _CODE(CONST)   	/* [k]      [-0, +1]    push a constant from (k) to stack */ \
    _CODE(NEG)     	/* []       [-1, +1]    */ \
    _CODE(NOT)     	/* []       [-1, +1]    */ \
    _CODE(LT)      	/* []       [-1, +1]    */ \
    _CODE(LE)      	/* []       [-1, +1]    */ \
    _CODE(EQ)      	/* []       [-1, +1]    */ \
    _CODE(ADD)     	/* []       [-2, +1]    */ \
    _CODE(SUB)     	/* []       [-2, +1]    */ \
    _CODE(MUL)     	/* []       [-2, +1]    */ \
    _CODE(DIV)     	/* []       [-2, +1]    */ \
    _CODE(DEF)     	/* [k]      [-1, +0]    pop a value from stack and define as (k) in global */ \
    _CODE(GLD)     	/* [k]      [-0, +1]    push a from (k) in global to stack */ \
    _CODE(GST)     	/* [k]      [-0, +0]    set a value from stack as (k) in global */ \
    _CODE(JMP)     	/* [s, s]   [-0, +0]    */ \
    _CODE(JMPF)    	/* [s, s]   [-1, +0]    */ \
    _CODE(LD)      	/* [s]      [-0, +1]    */ \
    _CODE(ST)      	/* [s]      [-0, +0]    */ \
    _CODE(MAP)      /* []       [-0, +1]    */ \
    _CODE(GET)      \
    _CODE(SET)      \
    _CODE(GETI)     \
    _CODE(SETI)

typedef enum {
#define _CODE(x)    OP_##x,
    OPCODES()
#undef _CODE
    MAX_OPCODES
} opcode_t;

typedef struct {
    char *buffer;
    char *fname;
    size_t size;
} src_t;

src_t *src_new(const char *fname);
void src_free(src_t *source);

typedef struct {
    int count;
    int capacity;
    uint8_t *code;
    uint16_t *lines;
    uint16_t *columns;
    src_t *source;
    arr_t constants;
} chunk_t;

void chunk_init(chunk_t *chunk, src_t *source);
void chunk_free(chunk_t *chunk);
void chunk_emit(chunk_t *chunk, uint8_t byte, int ln, int col);

static const char *opcode_tostr(opcode_t opcode) {
#define _CODE(x) #x,
    static const char *tab[] = { OPCODES() };
    return tab[opcode];
#undef _CODE
}

typedef enum {
    // Single-character tokens.                         
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_LBRACE,
    TOKEN_RBRACE,

    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_SEMICOLON,
    TOKEN_SLASH,
    TOKEN_STAR,

    // One or two character tokens.                     
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,

    // Literals.                                        
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,

    // Keywords.                                        
    TOKEN_AND,
    TOKEN_CASE,
    TOKEN_CLASS,
    TOKEN_CONST,
    TOKEN_DEFAULT,
    TOKEN_DIM,
    TOKEN_DO,
    TOKEN_ELSE,
    TOKEN_ELSEIF,
    TOKEN_ENDFUNC,
    TOKEN_ENDIF,
    TOKEN_ENDSELECT,
    TOKEN_ENDSWITCH,
    TOKEN_ENDWITH,
    TOKEN_ENUM,
    TOKEN_EXIT,
    TOKEN_EXITLOOP,
    TOKEN_FALSE,
    TOKEN_FOR,
    TOKEN_FUNC,
    TOKEN_GLOBAL,
    TOKEN_IF,
    TOKEN_LOCAL,
    TOKEN_NEXT,
    TOKEN_NULL,
    TOKEN_OR,
    TOKEN_PRINT,
    TOKEN_REDIM,
    TOKEN_RETURN,
    TOKEN_SELECT,
    TOKEN_STATIC,
    TOKEN_STEP,
    TOKEN_SUPER,
    TOKEN_SWITCH,
    TOKEN_THIS,
    TOKEN_THEN,
    TOKEN_TO,
    TOKEN_TRUE,
    TOKEN_UNTIL,
    TOKEN_VAR,
    TOKEN_VOLATILE,
    TOKEN_WEND,
    TOKEN_WHILE,
    TOKEN_WITH,

    TOKEN_MACRO,
    TOKEN_PREPROCESSOR,

    TOKEN_ERROR,
    TOKEN_EOF,

    MAX_TOKENS
} toktype_t;

typedef struct {
    const char *start;
    const char *currentLine;
    toktype_t type;
    int length;
    int line;
    int column;
} tok_t;

typedef struct {
    const char *start;
    const char *current;
    const char *currentLine;
    int line;
    int position;
} lexer_t;

void lexer_init(lexer_t *lexer, const char *source);
tok_t lexer_scan(lexer_t *lexer);

fun_t *compile(vm_t *vm, src_t *source);
