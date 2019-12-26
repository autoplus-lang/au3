#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "object.h"

void au3_initTable(au3Table *table)
{
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void au3_freeTable(au3Table *table)
{
    free(table->entries);
    au3_initTable(table);
}
