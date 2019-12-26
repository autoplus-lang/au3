#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "object.h"

au3String *au3_copyString(const char *chars, int length)
{
    char *heapChars = malloc(sizeof(char) * (length + 1));
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length);
}