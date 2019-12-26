#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "vm.h"
#include "compiler.h"

static char *readFile(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char *buffer = malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("usage: yaau3 [file]\n");
        return 0;
    }

    char *source = readFile(argv[argc - 1]);
    au3VM *vm = au3_create();

    clock_t clk = clock();

    au3Status result = au3_interpret(vm, source);

    if (result != AU3_OK) {
        printf("\n%s error!\n", result == AU3_COMPILE_ERROR ? "Compile" : "Runtime");
    }
    else {
        printf("\ndone in %3gs!\n", (clock() - clk) / (double)CLOCKS_PER_SEC);
    }

    au3_close(vm);
    free(source);
    return 0;
}
