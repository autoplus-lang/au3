#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "compiler.h"

static void resetStack(au3VM *vm)
{
    vm->top = vm->stack;
}

static void runtimeError(au3VM *vm, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm->ip - vm->chunk->code;
    int line = vm->chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);

    resetStack(vm);
}

au3VM *au3_create()
{
    au3VM *vm = calloc(sizeof(au3VM), 1);

    if (vm != NULL) {

        resetStack(vm);
    }

    return vm;
}

void au3_close(au3VM *vm)
{
    if (vm == NULL) return;

    free(vm);
}

#define PUSH(vm, v)     *((vm)->top++) = (v)
#define POP(vm)         *(--(vm)->top)
#define PEEK(vm, i)     ((vm)->top[-1 - (i)])

static au3Status execute(au3VM *vm)
{
#define READ_BYTE()     (*vm->ip++)
#define READ_LAST()     (vm->ip[-1])
#define READ_CONST()    (vm->chunk->constants.values[READ_BYTE()])

#define DISPATCH()      for (;;) switch (READ_BYTE())
#define CASE_CODE(x)    case OP_##x:
#define CASE_ERROR()    default:
#define NEXT            continue

    DISPATCH() {
        CASE_CODE(RET) {

            return AU3_OK;
        }
        CASE_CODE(NEG) {
            if (!AU3_IS_NUMBER(PEEK(vm, 0))) {
                runtimeError(vm, "Operand must be a number.");
                return AU3_RUNTIME_ERROR;
            }

            PUSH(vm, AU3_NUMBER(-AU3_AS_NUMBER(POP(vm))));
            NEXT;
        }
        CASE_CODE(CONST) {
            au3Value value = READ_CONST();
            PUSH(vm, value);
            NEXT;
        }
        CASE_ERROR() {
            runtimeError(vm, "Bad opcode, got %d!", READ_LAST());
            return AU3_RUNTIME_ERROR;
        }
    }

    return AU3_OK;
}

au3Status au3_interpret(au3VM *vm, const char *source)
{
    au3Chunk chunk;
    au3_initChunk(&chunk);

    if (!au3_compile(source, &chunk)) {
        au3_freeChunk(&chunk);
        return AU3_COMPILE_ERROR;
    }

    vm->chunk = &chunk;
    vm->ip = vm->chunk->code;
    au3Status result = execute(vm);

    au3_freeChunk(&chunk);
    return result;
}