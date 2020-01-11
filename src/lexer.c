#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "code.h"

void lexer_init(lexer_t *L, const char *source)
{
    L->start = source;
    L->current = source;
    L->currentLine = source;

    L->line = 1;
    L->position = 1;
}

bool equal_str(const char *a, const char *b, int len)
{
    for (int i = 0; i < len; i++) {
        if (tolower(a[i]) != tolower(b[i]))
            return false;
    }

    return true;
}

static bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        || (c == '_')
        || (c == '$');
}

static bool isDigit(char c)
{
    return (c >= '0')
        && (c <= '9');
}

static bool isHexa(char c)
{
    return (c >= '0' && c <= '9' )
        || (c >= 'a' && c <= 'f')
        || (c >= 'A' && c <= 'F');
}

static bool isAtEnd(lexer_t *L)
{
    return *L->current == '\0';
}

static char advance(lexer_t *L)
{
    L->current++;
    L->position++;
    return (char)tolower(L->current[-1]);
}

static char peek(lexer_t *L)
{
    return (char)tolower(*L->current);
}

static char peekNext(lexer_t *L)
{
    if (isAtEnd(L)) return '\0';
    return (char)tolower(L->current[1]);
}

static void newLine(lexer_t *L)
{
    L->line++;
    L->position = 0;
    L->currentLine = L->current + 1;
}

static bool match(lexer_t *L, char expected)
{
    if (isAtEnd(L)) return false;
    if (*L->current != expected) return false;

    L->current++;
    L->position++;
    return true;
}

static tok_t makeToken(lexer_t *L, toktype_t type)
{
    tok_t token;
    token.type = type;
    token.start = L->start;
    token.length = (int)(L->current - L->start);
    token.line = L->line;
    token.column = L->position - token.length;
    token.currentLine = L->currentLine;

    return token;
}

static tok_t errorToken(lexer_t *L, const char *message)
{
    tok_t token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = L->line;
    token.column = L->position - 1;

    return token;
}

static toktype_t checkKeyword(lexer_t *L, int start, int length, const char *rest, toktype_t type)
{
    if (L->current - L->start == start + length &&
        equal_str(L->start + start, rest, length)) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static void skipWhitespace(lexer_t *L)
{
    for (;;) {
        char c = peek(L);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(L);
                break;

            case '\n':
                newLine(L);
                advance(L);
                break;

            case ';':
                // A comment goes until the end of the line.
                while (peek(L) != '\n' && !isAtEnd(L)) advance(L);
                break;

            case '#':
                // Multiline comments
                // #cs, #comments-start
                if ((checkKeyword(L, 1, 2, "cs", -1) != TOKEN_IDENTIFIER) ||
                    (checkKeyword(L, 1, 14, "comments-start", -1) != TOKEN_IDENTIFIER)) {
                    // Skip '#'
                    advance(L);
                    while (!isAtEnd(L)) {
                        if (peek(L) == '\n') newLine(L);
                        if (peek(L) == '#' && !isAtEnd(L)) {
                            // #ce, #comments-end
                            if ((checkKeyword(L, 1, 2, "ce", -1) != TOKEN_IDENTIFIER) ||
                                (checkKeyword(L, 1, 12, "comments-end", -1) != TOKEN_IDENTIFIER)) {
                                break;
                            }
                        }
                        advance(L);
                    }
                }
                else {
                    return;
                }
                break;

            default:
                return;
        }
    }
}

static toktype_t identifierType(lexer_t *L)
{
#define START(i)    tolower(L->start[i])
#define LENGTH()    (L->current - L->start)

    switch (START(0)) {
        case 'a':
            return checkKeyword(L, 1, 2, "nd", TOKEN_AND);
        case 'c':
            if (LENGTH() > 1) switch (START(1)) {
                case 'a': return checkKeyword(L, 2, 2, "se", TOKEN_CASE);
                case 'l': return checkKeyword(L, 2, 3, "ass", TOKEN_CLASS);
                case 'o': return checkKeyword(L, 2, 3, "nst", TOKEN_CONST);
            }
            break;
        case 'd':
            if (LENGTH() > 1) switch (START(1)) {
                case 'e': return checkKeyword(L, 2, 5, "fault", TOKEN_DEFAULT);
                case 'i': return checkKeyword(L, 2, 1, "m", TOKEN_DIM);
                case 'o': return checkKeyword(L, 2, 0, "", TOKEN_DO);
            }
            break;
        case 'e':
            switch (LENGTH()) {
                case 3:
                    return checkKeyword(L, 1, 2, "nd", TOKEN_END);
                case 4:
                    if (START(1) == 'l')
                        return checkKeyword(L, 1, 3, "lse", TOKEN_ELSE);
                    else if (START(3) == 'n')
                        return checkKeyword(L, 1, 3, "num", TOKEN_ENUM);
                    break;
                case 5:
                    return checkKeyword(L, 1, 4, "ndif", TOKEN_ENDIF);
                case 7: 
                    if (START(3) == 'f')
                        return checkKeyword(L, 1, 6, "ndfunc", TOKEN_ENDFUNC);
                    else if (START(3) == 'w')
                        return checkKeyword(L, 1, 6, "ndwith", TOKEN_ENDWITH);
                    break;
                case 9:
                    if (START(4) == 'e')
                        return checkKeyword(L, 1, 8, "ndselect", TOKEN_ENDSELECT);
                    else if (START(4) == 'w')
                        return checkKeyword(L, 1, 8, "ndswitch", TOKEN_ENDSWITCH);
                    break;
            }
            break;
        case 'f':
            if (LENGTH() > 1) switch (START(1)) {
                case 'a': return checkKeyword(L, 2, 3, "lse", TOKEN_FALSE);
                case 'o': return checkKeyword(L, 2, 1, "r", TOKEN_FOR);
                case 'u': return checkKeyword(L, 2, 2, "nc", TOKEN_FUNC);
            }
            break;
        case 'g':
            return checkKeyword(L, 1, 5, "lobal", TOKEN_GLOBAL);
        case 'i':
            return checkKeyword(L, 1, 1, "f", TOKEN_IF);
        case 'l':
            return checkKeyword(L, 1, 4, "ocal", TOKEN_LOCAL);
        case 'n':
            if (LENGTH() > 1) switch (START(1)) {
                case 'e': return checkKeyword(L, 2, 2, "xt", TOKEN_NEXT);
                case 'o': return checkKeyword(L, 2, 1, "t", TOKEN_NOT);
                case 'u': return checkKeyword(L, 2, 2, "ll", TOKEN_NULL);
            }
            break;
        case 'o':
            return checkKeyword(L, 1, 1, "r", TOKEN_OR);
        case 'p':
            return checkKeyword(L, 1, 4, "rint", TOKEN_PRINT);
        case 'r':
            if (LENGTH() > 2 && START(1) == 'e') {
                    switch (START(2)) {
                        case 't': return checkKeyword(L, 3, 3, "urn", TOKEN_RETURN);
                        case 'd': return checkKeyword(L, 3, 2, "im", TOKEN_REDIM);
                    }
            }
            break;
        case 's':
            if (LENGTH() > 1) switch (START(1)) {
                case 'e': return checkKeyword(L, 2, 4, "lect", TOKEN_SELECT);
                case 't':
                    if (LENGTH() > 2) switch (START(2)) {
                        case 'a': return checkKeyword(L, 3, 3, "tic", TOKEN_STATIC);
                        case 'e': return checkKeyword(L, 3, 1, "p", TOKEN_STEP);                     
                    }
                    break;
                case 'u': return checkKeyword(L, 2, 3, "per", TOKEN_SUPER);
                case 'w': return checkKeyword(L, 2, 4, "itch", TOKEN_SWITCH);
            }
            break;
        case 't':
            if (LENGTH() > 1) switch (START(1)) {
                case 'h':
                    if (LENGTH() > 2) switch (START(2)) {
                        case 'e': return checkKeyword(L, 3, 1, "n", TOKEN_THEN);
                        case 'i': return checkKeyword(L, 3, 1, "s", TOKEN_THIS);
                    }
                    break;
                case 'r': return checkKeyword(L, 2, 2, "ue", TOKEN_TRUE);
                case 'o': return checkKeyword(L, 2, 2, "", TOKEN_TO);
            }       
            break;
        case 'v':
            if (LENGTH() > 1) switch (START(1)) {
                case 'a': return checkKeyword(L, 2, 1, "r", TOKEN_VAR);
                case 'o': return checkKeyword(L, 2, 6, "latile", TOKEN_VOLATILE);
            }
            break;
        case 'w':
            if (LENGTH() > 1) switch (START(1)) {
                case 'e': return checkKeyword(L, 2, 2, "nd", TOKEN_WEND);
                case 'h': return checkKeyword(L, 2, 3, "ile", TOKEN_WHILE);
                case 'i': return checkKeyword(L, 2, 2, "th", TOKEN_WITH);
            }
            break;
    }

    return TOKEN_IDENTIFIER;

#undef START
#undef LENGTH
}

static tok_t identifier(lexer_t *L)
{
    while (isAlpha(peek(L)) || isDigit(peek(L))) advance(L);

    return makeToken(L, identifierType(L));
}

static tok_t number(lexer_t *L)
{
    while (isDigit(peek(L))) advance(L);

    // Look for a fractional part.             
    if (peek(L) == '.' && isDigit(peekNext(L))) {
        // Consume the ".".                      
        advance(L);

        while (isDigit(peek(L))) advance(L);
    }

    return makeToken(L, TOKEN_NUMBER);
}

static tok_t string(lexer_t *L, char start)
{
    while (peek(L) != start && !isAtEnd(L)) {
        if (peek(L) == '\n') newLine(L);
        advance(L);
    }

    if (isAtEnd(L)) return errorToken(L, "Unterminated string.");

    // The closing quote.                                    
    advance(L);
    return makeToken(L, TOKEN_STRING);
}

tok_t lexer_scan(lexer_t *L)
{
    skipWhitespace(L);

    L->start = L->current;

    if (isAtEnd(L)) return makeToken(L, TOKEN_EOF);

    char c = advance(L);
    if (isAlpha(c)) return identifier(L);
    if (isDigit(c)) return number(L);

    switch (c) {
        case '(': return makeToken(L, TOKEN_LPAREN);
        case ')': return makeToken(L, TOKEN_RPAREN);
        case '[': return makeToken(L, TOKEN_LBRACKET);
        case ']': return makeToken(L, TOKEN_RBRACKET);
        case '{': return makeToken(L, TOKEN_LBRACE);
        case '}': return makeToken(L, TOKEN_RBRACE);

        case ',': return makeToken(L, TOKEN_COMMA);
        case '.': return makeToken(L, TOKEN_DOT);
        case '-': return makeToken(L, TOKEN_MINUS);
        case '+': return makeToken(L, TOKEN_PLUS);
        case '/': return makeToken(L, TOKEN_SLASH);
        case '*': return makeToken(L, TOKEN_STAR);

        case '!':
            return makeToken(L, match(L, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return makeToken(L, match(L, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return makeToken(L, match(L, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return makeToken(L, match(L, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

        case '\'':
        case '\"': return string(L, c);
    }

    return errorToken(L, "Unexpected character.");
}
