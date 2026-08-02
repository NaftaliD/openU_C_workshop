/*
 * macro_table.h
 * Macro definitions, stored as a linked list (name + body).
 */

#ifndef MACRO_TABLE_H
#define MACRO_TABLE_H

#include "common.h"

/* Single macro definition */
typedef struct MacroNode {
    char name[MAX_MACRO_NAME_LENGTH + 1];
    char *body;              /* single string, embedded newlines between lines */
    struct MacroNode *next;
} MacroNode;

/* Macro table (linked list head) */
typedef struct {
    MacroNode *head;
} MacroTable;

/**
 * Allocates and initializes an empty macro table.
 * @return the new table, or NULL on allocation failure
 */
MacroTable* createMacroTable(void);

/**
 * Inserts a new macro into the table. Takes ownership of body (caller
 * should not free it) only on success.
 * @param table the macro table
 * @param name the macro's name
 * @param body the macro's body, as a single string with embedded newlines
 * @return RC_SUCCESS, RC_INVALID for a NULL argument, RC_DUPLICATE if a
 *         macro with that name already exists, or RC_OUT_OF_MEMORY
 */
ResultCode insertMacro(MacroTable *table, const char *name, char *body);

/**
 * Searches for a macro by name.
 * @param table the macro table
 * @param name the macro name to look up
 * @return the matching MacroNode, or NULL if not found
 */
MacroNode* lookupMacro(MacroTable *table, const char *name);

/**
 * Frees every macro node's body, every node, and the table itself.
 * @param table the macro table to free
 */
void destroyMacroTable(MacroTable *table);

/**
 * Builds a debug string describing one macro node. Caller must free the
 * returned string.
 * @param node the macro node to describe
 * @return a malloc'd description string
 */
char* macroNodeToString(const MacroNode *node);

/**
 * Builds a debug string describing every macro in the table. Caller must
 * free the returned string.
 * @param table the macro table to describe
 * @return a malloc'd description string
 */
char* macroTableToString(const MacroTable *table);

#endif /* MACRO_TABLE_H */
