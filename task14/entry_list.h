/*
 * entry_list.h
 * Deferred .entry records: a symbol may be a forward reference, so marking
 * it in the symbol table is deferred to Pass Two, which walks this list once.
 */

#ifndef ENTRY_LIST_H
#define ENTRY_LIST_H

#include "common.h"

typedef struct {
    int  line_number;
    char symbol_name[MAX_LABEL_LENGTH + 1];
} EntryNode;

/* Growable array of structs -- small and self-contained, no need to
 * individually malloc each one */
typedef struct {
    EntryNode *items;
    int count;
    int capacity;
} EntryList;

/**
 * Allocates and initializes an empty entry list.
 * @return the new list, or NULL on allocation failure
 */
EntryList* createEntryList(void);

/**
 * Records a deferred .entry reference.
 * @param list the entry list
 * @param line_number source line of the .entry directive
 * @param symbol_name the symbol it names
 * @return 0 on success, -1 on allocation failure
 */
int appendEntry(EntryList *list, int line_number, const char *symbol_name);

/**
 * Frees the backing array and the list itself.
 * @param list the entry list to free
 */
void destroyEntryList(EntryList *list);

#endif /* ENTRY_LIST_H */
