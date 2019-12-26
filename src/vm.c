#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "compiler.h"
#include "object.h"
#include "memory.h"

static au3VM firstVM;
static au3VM *g_pVM = NULL;

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
    au3VM *vm;
    
    if (g_pVM == NULL) {
        vm = &firstVM;
        g_pVM = vm;
    }
    else {
        vm = calloc(sizeof(au3VM), 1);
        if (vm == NULL) return NULL;
    }
    
    vm->objects = NULL;
    au3_initTable(&vm->globals);
    au3_initTable(&vm->strings);

    resetStack(vm);
    return vm;
}

void au3_close(au3VM *vm)
{
    if (vm != NULL && g_pVM != NULL) {
        vm = g_pVM;
    }
    
    au3_freeTable(&vm->globals);
    au3_freeTable(&vm->strings);
    au3_freeObjects(vm);

    if (vm != g_pVM) free(vm);
    else g_pVM = NULL;
}

#define PUSH(vm, v)     *((vm)->top++) = (v)
#define POP(vm)         *(--(vm)->top)
#define PEEK(vm, i)     ((vm)->top[-1 - (i)])

static void concatenate(au3VM *vm)
{
    au3String *b = AU3_AS_STRING(POP(vm));
    au3String *a = AU3_AS_STRING(POP(vm));

    int length = a->length + b->length;
    char *chars = malloc(sizeof(char) * (length + 1));
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    au3String *result = au3_takeString(vm, chars, length);
    PUSH(vm, AU3_OBJECT(result));
}

static au3Status execute(au3VM *vm)
{
#define READ_BYTE()     (*vm->ip++)
#define READ_LAST()     (vm->ip[-1])
#define READ_CONST()    (vm->chunk->constants.values[READ_BYTE()])
#define READ_STRING()   AU3_AS_STRING(READ_CONST())

#define BINARY_OP(valueType, op) \
    do { \
        if (!AU3_IS_NUMBER(PEEK(vm, 0)) || !AU3_IS_NUMBER(PEEK(vm, 1))) { \
            runtimeError(vm, "Operands must be numbers."); \
            return AU3_RUNTIME_ERROR; \
        } \
        double b = AU3_AS_NUMBER(POP(vm)); \
        double a = AU3_AS_NUMBER(POP(vm)); \
        PUSH(vm, valueType(a op b)); \
    } while (false)

#define DISPATCH()      for (;;) switch (READ_BYTE())
#define CASE_CODE(x)    case OP_##x:
#define CASE_ERROR()    default:
#define NEXT            continue

    DISPATCH() {

        CASE_CODE(NOP) NEXT;

        CASE_CODE(PUTS) {
            au3_printValue(POP(vm));
            printf("\n");
            NEXT;
        }
        CASE_CODE(POP) {
            POP(vm);
            NEXT;
        }

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
        CASE_CODE(ADD) {
            if (AU3_IS_STRING(PEEK(vm, 0)) && AU3_IS_STRING(PEEK(vm, 1))) {
                concatenate(vm);
            }
            else if (AU3_IS_NUMBER(PEEK(vm, 0)) && AU3_IS_NUMBER(PEEK(vm, 1))) {
                double b = AU3_AS_NUMBER(POP(vm));
                double a = AU3_AS_NUMBER(POP(vm));
                PUSH(vm, AU3_NUMBER(a + b));
            }
            else {
                runtimeError(vm, "Operands must be two numbers or two strings.");
                return AU3_RUNTIME_ERROR;
            }
            NEXT;
        }
        CASE_CODE(SUB) {
            BINARY_OP(AU3_NUMBER, - );
            NEXT;
        }
        CASE_CODE(MUL) {
            BINARY_OP(AU3_NUMBER, * );
            NEXT;
        }
        CASE_CODE(DIV) {
            BINARY_OP(AU3_NUMBER, / );
            NEXT;
        }

        CASE_CODE(NOT) {
            PUSH(vm, AU3_BOOL(AU3_IS_FALSEY(POP(vm))));
            NEXT;
        }
        CASE_CODE(EQ) {
            au3Value b = POP(vm);
            au3Value a = POP(vm);
            PUSH(vm, AU3_BOOL(au3_valuesEqual(a, b)));
            NEXT;
        }
        CASE_CODE(LT) {
            BINARY_OP(AU3_BOOL, < );
            NEXT;
        }
        CASE_CODE(LE) {
            BINARY_OP(AU3_BOOL, <= );
            NEXT;
        }

        CASE_CODE(NULL) {
            PUSH(vm, AU3_NULL);
            NEXT;
        }
        CASE_CODE(TRUE) {
            PUSH(vm, AU3_TRUE);
            NEXT;
        }
        CASE_CODE(FALSE) {
            PUSH(vm, AU3_FALSE);
            NEXT;
        }
        CASE_CODE(CONST) {
            au3Value value = READ_CONST();
            PUSH(vm, value);
            NEXT;
        }

        CASE_CODE(DEF) {
            au3String *name = READ_STRING();
            au3_tableSet(&vm->globals, name, PEEK(vm, 0));
            POP(vm);
            NEXT;
        }
        CASE_CODE(GLD) {
            au3String *name = READ_STRING();
            au3Value value;
            if (!au3_tableGet(&vm->globals, name, &value)) {
                runtimeError(vm, "Undefined variable '%s'.", name->chars);
                return AU3_RUNTIME_ERROR;
            }
            PUSH(vm, value);
            NEXT;
        }
        CASE_CODE(GST) {
            au3String *name = READ_STRING();
            if (au3_tableSet(&vm->globals, name, PEEK(vm, 0))) {
                au3_tableDelete(&vm->globals, name);
                runtimeError(vm, "Undefined variable '%s'.", name->chars);
                return AU3_RUNTIME_ERROR;
            }
            NEXT;
        }

        CASE_CODE(LD) {
            uint8_t slot = READ_BYTE();
            PUSH(vm, vm->stack[slot]);
            NEXT;
        }
        CASE_CODE(ST) {
            uint8_t slot = READ_BYTE();
            vm->stack[slot] = PEEK(vm, 0);
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

    if (!au3_compile(vm, source, &chunk)) {
        au3_freeChunk(&chunk);
        return AU3_COMPILE_ERROR;
    }

    vm->chunk = &chunk;
    vm->ip = vm->chunk->code;
    au3Status result = execute(vm);

    au3_freeChunk(&chunk);
    return result;
}