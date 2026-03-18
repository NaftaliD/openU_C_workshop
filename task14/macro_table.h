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
 * MacroTable_create
 * Allocates and initializes an empty macro table.
 */
MacroTable* MacroTable_create(void);

/*
 * MacroTable_insert
 * Inserts a new macro into the table.
 * Takes ownership of the body string (caller should not free it).
 * Returns 0 on success, -1 if macro already exists.
 */
int MacroTable_insert(MacroTable *table, const char *name, char *body);

/*
 * MacroTable_lookup
 * Searches for a macro by name.
 * Returns pointer to MacroNode on success, NULL if not found.
 */
MacroNode* MacroTable_lookup(MacroTable *table, const char *name);

/*
 * MacroTable_destroy
 * Frees all memory associated with the macro table.
 */
void MacroTable_destroy(MacroTable *table);

/*
 * MacroNode_toString
 * Create a string representation of a macro node for debugging.
 * Returns a malloc'd string describing the macro and its contents.
 * Caller must free the returned string.
 */
char* MacroNode_toString(const MacroNode *node);

/*
 * MacroTable_toString
 * Create a string representation of the entire macro table for debugging.
 * Returns a malloc'd string describing all macros in the table.
 * Caller must free the returned string.
 */
char* MacroTable_toString(const MacroTable *table);

#endif /* MACRO_TABLE_H */
