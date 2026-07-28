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

/*
 * createMacroTable
 * Allocates and initializes an empty macro table.
 */
MacroTable* createMacroTable(void);

/*
 * insertMacro
 * Inserts a new macro into the table.
 * Takes ownership of the body string (caller should not free it).
 * Returns 0 on success, -1 if macro already exists.
 */
int insertMacro(MacroTable *table, const char *name, char *body);

/*
 * lookupMacro
 * Searches for a macro by name.
 * Returns pointer to MacroNode on success, NULL if not found.
 */
MacroNode* lookupMacro(MacroTable *table, const char *name);

/*
 * destroyMacroTable
 * Frees all memory associated with the macro table.
 */
void destroyMacroTable(MacroTable *table);

/*
 * macroNodeToString
 * Create a string representation of a macro node for debugging.
 * Returns a malloc'd string describing the macro and its contents.
 * Caller must free the returned string.
 */
char* macroNodeToString(const MacroNode *node);

/*
 * macroTableToString
 * Create a string representation of the entire macro table for debugging.
 * Returns a malloc'd string describing all macros in the table.
 * Caller must free the returned string.
 */
char* macroTableToString(const MacroTable *table);

#endif /* MACRO_TABLE_H */
