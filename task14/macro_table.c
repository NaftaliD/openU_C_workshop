/*
 * macro_table.c
 * Implementation of macro table management.
 * Handles linked list operations for storing macro definitions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "macro_table.h"
#include "logger.h"

MacroTable* createMacroTable(void)
{
    MacroTable *table = (MacroTable *)malloc(sizeof(MacroTable));
    if (table == NULL) return NULL;
    table->head = NULL;
    return table;
}

ResultCode insertMacro(MacroTable *table, const char *name, char *body)
{
    MacroNode *node;

    if (table == NULL || name == NULL || body == NULL) {
        return RC_INVALID;
    }

    /* Check if macro already exists using lookup */
    if (lookupMacro(table, name) != NULL) {
        return RC_DUPLICATE;
    }

    /* Allocate new macro node */
    node = (MacroNode *)malloc(sizeof(MacroNode));
    if (node == NULL) return RC_OUT_OF_MEMORY;

    strcpy(node->name, name);
    node->body = body;
    node->next = table->head;
    table->head = node;

    return RC_SUCCESS;
}

MacroNode* lookupMacro(MacroTable *table, const char *name)
{
    MacroNode *current;
    
    if (table == NULL || name == NULL) return NULL;
    
    current = table->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void destroyMacroTable(MacroTable *table)
{
    MacroNode *current;
    MacroNode *temp;
    
    if (table == NULL) return;
    
    current = table->head;
    while (current != NULL) {
        temp = current->next;
        if (current->body != NULL) {
            free(current->body);
        }
        free(current);
        current = temp;
    }
    free(table);
}

char* macroNodeToString(const MacroNode *node)
{
    char buffer[LARGE_BUFF_SIZE];
    int pos = 0;
    char *result;

    if (node == NULL) {
        result = (char *)malloc(NULL_PLACEHOLDER_SIZE);
        if (result != NULL) {
            strcpy(result, "[MacroNode: NULL]\n");
        }
        return result;
    }
    
    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                    "[MacroNode name='%s']\nBody:\n%s\n",
                    node->name, node->body);
    
    result = (char *)malloc(strlen(buffer) + 1);
    if (result != NULL) {
        strcpy(result, buffer);
    }
    return result;
}

char* macroTableToString(const MacroTable *table)
{
    char buffer[LARGE_BUFF_SIZE];
    int pos = 0;
    MacroNode *current;
    int count = 0;
    char *node_str;
    char *result;

    if (table == NULL) {
        result = (char *)malloc(NULL_PLACEHOLDER_SIZE);
        if (result != NULL) {
            strcpy(result, "[MacroTable: NULL]\n");
        }
        return result;
    }
    
    /* Count macros */
    current = table->head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    
    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                    "[MacroTable: %d macros]\n", count);
    
    current = table->head;
    while (current != NULL && pos < LARGE_BUFF_SIZE - NODE_DUMP_MARGIN) {
        node_str = macroNodeToString(current);
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%s", node_str);
        free(node_str);
        current = current->next;
    }
    
    result = (char *)malloc(strlen(buffer) + 1);
    if (result != NULL) {
        strcpy(result, buffer);
    }
    return result;
}
