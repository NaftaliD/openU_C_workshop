/*
 * symbol_table.c
 * Implementation of symbol table management.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "symbol_table.h"
#include "growable_array.h"

SymbolTable* createSymbolTable(void)
{
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
    if (table == NULL) {
        return NULL;
    }

    table->items = (SymbolNode *)malloc(sizeof(SymbolNode) * INITIAL_LIST_CAPACITY);
    if (table->items == NULL) {
        free(table);
        return NULL;
    }

    table->count = 0;
    table->capacity = INITIAL_LIST_CAPACITY;
    return table;
}

SymbolNode* lookupSymbol(SymbolTable *table, const char *name)
{
    int i;

    if (table == NULL || name == NULL) {
        return NULL;
    }

    for (i = 0; i < table->count; i++) {
        if (strcmp(table->items[i].name, name) == 0) {
            return &table->items[i];
        }
    }
    return NULL;
}

int insertSymbol(SymbolTable *table, const char *name, int value, SymbolType type)
{
    if (table == NULL || name == NULL) {
        return -1;
    }

    if (strlen(name) > MAX_LABEL_LENGTH) {
        return -1;
    }

    if (lookupSymbol(table, name) != NULL) {
        return -1;
    }

    if (table->count == table->capacity) {
        if (!growArray((void **)&table->items, &table->capacity, sizeof(SymbolNode))) {
            return -2;
        }
    }

    strcpy(table->items[table->count].name, name);
    table->items[table->count].value = value;
    table->items[table->count].type = type;
    table->items[table->count].is_entry = 0;
    table->count++;

    return 0;
}

int markEntry(SymbolTable *table, const char *name)
{
    SymbolNode *node = lookupSymbol(table, name);

    if (node == NULL) {
        return -1;
    }

    if (node->type == SYMBOL_TYPE_EXTERNAL) {
        return -1;
    }

    if (node->is_entry) {
        return 1;  /* already marked -- redundant, not an error */
    }

    node->is_entry = 1;
    return 0;
}

void relocateDataSymbols(SymbolTable *table, int icf)
{
    int i;

    if (table == NULL) {
        return;
    }

    for (i = 0; i < table->count; i++) {
        if (table->items[i].type == SYMBOL_TYPE_DATA) {
            table->items[i].value += icf;
        }
    }
}

void destroySymbolTable(SymbolTable *table)
{
    if (table == NULL) {
        return;
    }

    free(table->items);
    free(table);
}

/**
 * @param type a symbol's origin
 * @return "code", "data", or "external"
 */
static const char* symbolTypeToString(SymbolType type)
{
    switch (type) {
        case SYMBOL_TYPE_CODE:     return "code";
        case SYMBOL_TYPE_DATA:     return "data";
        case SYMBOL_TYPE_EXTERNAL: return "external";
        default:                   return "?";
    }
}

char* symbolTableToString(const SymbolTable *table)
{
    char buffer[8192];
    int pos = 0;
    int i;
    char *result;

    if (table == NULL) {
        result = (char *)malloc(30);
        if (result != NULL) {
            strcpy(result, "[SymbolTable: NULL]\n");
        }
        return result;
    }

    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                    "[SymbolTable: %d symbol(s)]\n", table->count);

    for (i = 0; i < table->count && pos < (int)sizeof(buffer) - 200; i++) {
        pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                        "  %s = %d (%s%s)\n", table->items[i].name,
                        table->items[i].value, symbolTypeToString(table->items[i].type),
                        table->items[i].is_entry ? ", entry" : "");
    }

    result = (char *)malloc(strlen(buffer) + 1);
    if (result != NULL) {
        strcpy(result, buffer);
    }
    return result;
}
