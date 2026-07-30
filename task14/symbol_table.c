/*
 * symbol_table.c
 * Implementation of symbol table management.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "symbol_table.h"

SymbolTable* createSymbolTable(void)
{
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
    if (table == NULL) {
        return NULL;
    }
    table->head = NULL;
    return table;
}

SymbolNode* lookupSymbol(SymbolTable *table, const char *name)
{
    SymbolNode *current;

    if (table == NULL || name == NULL) {
        return NULL;
    }

    current = table->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

int insertSymbol(SymbolTable *table, const char *name, int value, SymbolType type)
{
    SymbolNode *node;

    if (table == NULL || name == NULL) {
        return -1;
    }

    if (lookupSymbol(table, name) != NULL) {
        return -1;
    }

    node = (SymbolNode *)malloc(sizeof(SymbolNode));
    if (node == NULL) {
        return -1;
    }

    strcpy(node->name, name);
    node->value = value;
    node->type = type;
    node->is_entry = 0;
    node->next = table->head;
    table->head = node;

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
    SymbolNode *current;

    if (table == NULL) {
        return;
    }

    current = table->head;
    while (current != NULL) {
        if (current->type == SYMBOL_TYPE_DATA) {
            current->value += icf;
        }
        current = current->next;
    }
}

void destroySymbolTable(SymbolTable *table)
{
    SymbolNode *current;
    SymbolNode *temp;

    if (table == NULL) {
        return;
    }

    current = table->head;
    while (current != NULL) {
        temp = current->next;
        free(current);
        current = temp;
    }
    free(table);
}
