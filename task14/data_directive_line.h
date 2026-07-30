/*
 * data_directive_line.h
 * A single parsed data-directive statement (.db/.dw/.dh/.asciz). .entry and
 * .extern are handled elsewhere (entry_list.h, symbol_table.h) -- neither
 * allocates image bytes, so neither fits this shape. The list of these is a
 * thin wrapper around the generic PointerList (see pointer_list.h).
 */

#ifndef DATA_DIRECTIVE_LINE_H
#define DATA_DIRECTIVE_LINE_H

#include "common.h"
#include "pointer_list.h"

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

typedef PointerList DataDirectiveList;

/**
 * Looks up the per-value byte width for a data directive.
 * @param directive the directive enum value
 * @return 1 for .db/.asciz, 4 for .dw, 2 for .dh, -1 if not a data directive
 */
int getDirectiveByteWidth(Directive directive);

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
 * Frees each node's values array, each node, and the list itself.
 * @param list the data-directive list to free
 */
void destroyDataDirectiveList(DataDirectiveList *list);

#endif /* DATA_DIRECTIVE_LINE_H */
