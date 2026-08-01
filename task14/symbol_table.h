/*
 * symbol_table.h
 * Every label/extern symbol encountered while assembling a file, with its
 * resolved address and classification. A symbol's origin is always exactly
 * one of CODE/DATA/EXTERNAL (SymbolType) -- .entry never creates a symbol,
 * it only marks an existing one for the .ent file, so it's a separate flag
 * rather than a fourth SymbolType value.
 */

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "common.h"

typedef enum {
    SYMBOL_TYPE_CODE,
    SYMBOL_TYPE_DATA,
    SYMBOL_TYPE_EXTERNAL
} SymbolType;

typedef struct {
    char name[MAX_LABEL_LENGTH + 1];
    int  value;    /* address; relocated += ICF later, only when type == DATA */
    SymbolType type;
    int  is_entry; /* set later by markEntry; never valid together with EXTERNAL */
} SymbolNode;

/* Growable array of structs (via growable_array.h), matching EntryList's/
 * ErrorList's pattern. lookupSymbol returns a pointer into this array; that
 * stays valid because every insertSymbol call happens during Pass One's
 * single forward read, strictly before anything (markEntry, Pass Two) reads
 * or holds a pointer -- no insert ever happens after a lookup result is
 * retained. */
typedef struct {
    SymbolNode *items;
    int count;
    int capacity;
} SymbolTable;

/**
 * Allocates and initializes an empty symbol table.
 * @return the new table, or NULL on allocation failure
 */
SymbolTable* createSymbolTable(void);

/**
 * Searches for a symbol by name.
 * @param table the symbol table
 * @param name the symbol name to look up
 * @return the matching node, or NULL if not found
 */
SymbolNode* lookupSymbol(SymbolTable *table, const char *name);

/**
 * Inserts a new symbol (is_entry starts at 0; set later via markEntry).
 * @param table the symbol table
 * @param name the symbol's name
 * @param value the symbol's address (0 for SYMBOL_TYPE_EXTERNAL)
 * @param type the symbol's origin: CODE, DATA, or EXTERNAL
 * @return 0 on success, -1 if the name already exists or is longer than
 *         MAX_LABEL_LENGTH, -2 on allocation failure
 */
int insertSymbol(SymbolTable *table, const char *name, int value, SymbolType type);

/**
 * Marks an existing symbol as entry.
 * @param table the symbol table
 * @param name the symbol to mark
 * @return 0 if newly marked, 1 if it was already marked entry (a redundant
 *         but not erroneous repeat .entry declaration), -1 if not found or
 *         if its type is EXTERNAL
 */
int markEntry(SymbolTable *table, const char *name);

/**
 * Adds icf to every symbol whose type is SYMBOL_TYPE_DATA. Called once,
 * after Pass One finishes, to shift data addresses past the code image.
 * @param table the symbol table
 * @param icf the final instruction counter value
 */
void relocateDataSymbols(SymbolTable *table, int icf);

/**
 * Frees every symbol node and the table itself.
 * @param table the symbol table to free
 */
void destroySymbolTable(SymbolTable *table);

/**
 * Builds a debug string describing every symbol in the table. Caller must
 * free the returned string.
 * @param table the symbol table to describe
 * @return a malloc'd description string
 */
char* symbolTableToString(const SymbolTable *table);

#endif /* SYMBOL_TABLE_H */
