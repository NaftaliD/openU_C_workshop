/*
 * macro_table.h
 * Macro table management - storage and retrieval of macro definitions.
 * 
 * Each macro stores its name and source lines.
 * All macros are stored in a linked list.
 */

#ifndef MACRO_TABLE_H
#define MACRO_TABLE_H

/* Single macro definition */
typedef struct MacroNode {
    char name[32];           /* Macro name */
    char *body;              /* Macro body as single string with embedded newlines */
    struct MacroNode *next;  /* Next macro in linked list */
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
 * @return 0 on success, -1 if a macro with that name already exists, -2 on
 *         allocation failure
 */
int insertMacro(MacroTable *table, const char *name, char *body);

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
