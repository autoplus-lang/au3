#ifndef _AU3_COMPILER_H
#define _AU3_COMPILER_H
#pragma once

#include "common.h"
#include "chunk.h"
#include "vm.h"

typedef enum {
    // Single-character tokens.                         
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,

    // One or two character tokens.                     
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,

    // Literals.                                        
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,
    TOKEN_INTEGER, TOKEN_HEXADECIMAL,

    // Keywords.                                        
    TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
    TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_NULL, TOKEN_OR,
    TOKEN_PUTS, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
    TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

    TOKEN_ERROR,
    TOKEN_EOF
} au3TokenType;

typedef struct {
    au3TokenType type;
    const char *start;
    int length;
    int line;
    int column;
} au3Token;

void au3_initLexer(const char *source);
au3Token au3_scanToken();

au3Function *au3_compile(au3VM *vm, const char *source);

#endif
