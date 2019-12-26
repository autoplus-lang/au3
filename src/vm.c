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
    vm->frameCount = 0;
}

static void runtimeError(au3VM *vm, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = vm->frameCount - 1; i >= 0; i--) {
        au3CallFrame *frame = &vm->frames[i];
        au3Function *function = frame->function;
        // -1 because the IP is sitting on the next instruction to be
        // executed.                                                 
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[%d:%d] in ",
            function->chunk.lines[instruction], function->chunk.columns[instruction]);
        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        }
        else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

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

static bool call(au3VM *vm, au3Function *function, int argCount)
{
    if (argCount != function->arity) {
        runtimeError(vm, "Expected %d arguments but got %d.",
            function->arity, argCount);
        return false;
    }

    if (vm->frameCount == AU3_MAX_FRAMES) {
        runtimeError(vm, "Stack overflow.");
        return false;
    }

    au3CallFrame *frame = &vm->frames[vm->frameCount++];
    frame->function = function;
    frame->ip = function->chunk.code;

    frame->slots = vm->top - argCount - 1;
    return true;
}

static bool callValue(au3VM *vm, au3Value callee, int argCount)
{
    if (AU3_IS_OBJECT(callee)) {
        switch (AU3_OBJECT_TYPE(callee)) {
            case AU3_TFUNCTION:
                return call(vm, AU3_AS_FUNCTION(callee), argCount);

            default:
                // Non-callable object type.                   
                break;
            }
    }

    runtimeError(vm, "Can only call functions and classes.");
    return false;
}

static au3Status execute(au3VM *vm)
{
    au3CallFrame *frame = &vm->frames[vm->frameCount - 1];

#define READ_BYTE()     (*frame->ip++)
#define READ_SHORT()    (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_LAST()     (frame->ip[-1])

#define READ_CONST()    (frame->function->chunk.constants.values[READ_BYTE()])
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
        CASE_CODE(CALL) {
            int argCount = READ_BYTE();
            if (!callValue(vm, PEEK(vm, argCount), argCount)) {
                return AU3_RUNTIME_ERROR;
            }

            frame = &vm->frames[vm->frameCount - 1];
            NEXT;
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
            PUSH(vm, frame->slots[slot]);
            NEXT;
        }
        CASE_CODE(ST) {
            uint8_t slot = READ_BYTE();
            frame->slots[slot] = PEEK(vm, 0);
            NEXT;
        }

        CASE_CODE(JMP) {
            uint16_t offset = READ_SHORT();
            frame->ip += offset;
            NEXT;
        }
        CASE_CODE(JMPF) {
            uint16_t offset = READ_SHORT();
            if (AU3_IS_FALSEY(PEEK(vm, 0))) frame->ip += offset;
            NEXT;
        }
        CASE_CODE(LOOP) {
            uint16_t offset = READ_SHORT();
            frame->ip -= offset;
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
    au3Function *function = au3_compile(vm, source);
    if (function == NULL) return AU3_COMPILE_ERROR;

    PUSH(vm, AU3_OBJECT(function));
    callValue(vm, AU3_OBJECT(function), 0);

   return execute(vm);
}
