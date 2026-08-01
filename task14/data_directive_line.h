/*
 * data_directive_line.h
 * A single parsed data-directive statement (.db/.dw/.dh/.asciz). .entry and
 * .extern are handled elsewhere (entry_list.h, symbol_table.h) -- neither
 * allocates image bytes, so neither fits this shape. Also a growable list of
 * these (backed by growable_array.h's shared growth primitive).
 */

#ifndef DATA_DIRECTIVE_LINE_H
#define DATA_DIRECTIVE_LINE_H

#include "common.h"

/* One fully-parsed data-directive statement */
typedef struct {
    int       line_number;
    Directive directive;
    long     *values;         /* one entry per value; .asciz = char codes + trailing 0 */
    int       value_count;
    int       value_capacity;
    int       address;        /* DC when encountered; += ICF once Pass One finishes */
    int       has_error;
} DataDirectiveNode;

/* Growable array of pointers, in file order */
typedef struct {
    DataDirectiveNode **items;
    int count;
    int capacity;
} DataDirectiveList;

/**
 * Looks up the per-value byte width for a data directive.
 * @param directive the directive enum value
 * @return 1 for .db/.asciz, 4 for .dw, 2 for .dh, -1 if not a data directive
 */
int getDirectiveByteWidth(Directive directive);

/**
 * Allocates a DataDirectiveNode. Fields are left uninitialized -- the
 * caller (parseLine's directive sub-parser) fills in every field
 * unconditionally.
 * @return the new node, or NULL on allocation failure
 */
DataDirectiveNode* createDataDirectiveNode(void);

/**
 * Allocates and initializes an empty data-directive list.
 * @return the new list, or NULL on allocation failure
 */
DataDirectiveList* createDataDirectiveList(void);

/**
 * Appends node, growing the list if needed. Takes ownership of node.
 * @param list the data-directive list
 * @param node the node to append
 * @return 0 on success, -1 on allocation failure
 */
int appendDataDirective(DataDirectiveList *list, DataDirectiveNode *node);

/**
 * Appends one value to node's own value list, growing it if needed (starts
 * unallocated, as createDataDirectiveNode leaves it -- the first call here
 * allocates it at INITIAL_LIST_CAPACITY).
 * @param node the data-directive node being built
 * @param value the value to append
 * @return 0 on success, -1 on allocation failure
 */
int appendDirectiveValue(DataDirectiveNode *node, long value);

/**
 * Frees each node's values array, each node, and the list itself.
 * @param list the data-directive list to free
 */
void destroyDataDirectiveList(DataDirectiveList *list);

/**
 * Adds icf to every node's address. Called once, after Pass One finishes,
 * to shift data addresses past the code image -- mirrors
 * relocateDataSymbols in symbol_table.h, which does the same for the
 * symbol table's DATA-type entries.
 * @param list the data-directive list
 * @param icf the final instruction counter value
 */
void relocateDataDirectiveAddresses(DataDirectiveList *list, int icf);

/**
 * Builds a debug string describing one data-directive node. Caller must
 * free the returned string.
 * @param node the node to describe
 * @return a malloc'd description string
 */
char* dataDirectiveNodeToString(const DataDirectiveNode *node);

/**
 * Builds a debug string describing every node in the list. Caller must free
 * the returned string.
 * @param list the data-directive list to describe
 * @return a malloc'd description string
 */
char* dataDirectiveListToString(const DataDirectiveList *list);

#endif /* DATA_DIRECTIVE_LINE_H */
