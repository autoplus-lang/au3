#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code.h"
#include "value.h"

#define CODE_PAGE   256

void chunk_init(chunk_t *chunk, src_t *source)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    chunk->columns = NULL;
    chunk->source = source;

    arr_init(&chunk->constants);
}

void chunk_free(chunk_t *chunk)
{
    free(chunk->code);
    free(chunk->lines);

    arr_free(&chunk->constants);
    chunk_init(chunk, NULL);
}

void chunk_emit(chunk_t *chunk, uint8_t byte, int line, int column)
{
    if (chunk->count >= chunk->capacity) {
        chunk->capacity += CODE_PAGE;
        chunk->code = realloc(chunk->code, chunk->capacity * sizeof(uint8_t));
        chunk->lines = realloc(chunk->lines, chunk->capacity * sizeof(uint16_t));
        chunk->columns = realloc(chunk->columns, chunk->capacity * sizeof(uint16_t));
    }

    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->columns[chunk->count] = column;
    chunk->count++;
}

src_t *src_new(const char *fname)
{
    src_t *source = malloc(sizeof(src_t));
    if (source == NULL) return NULL;

    char *buffer = read_file(fname, &source->size);
    if (buffer == NULL) {
        free(source);
        return NULL;
    }

    const char *s;
    if ((s = strrchr(fname, '/')) != NULL) s++;
    if ((s = strrchr(fname, '\\')) != NULL) s++;
    if (s == NULL) s = fname;

    source->fname = strdup(s);
    source->buffer = buffer;
    return source;
}

void src_free(src_t *source)
{
    if (source == NULL) return;
    free(source->fname);
    free(source->buffer);
    free(source);
}
