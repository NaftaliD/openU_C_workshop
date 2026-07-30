/*
 * data_directive_line.c
 * Implementation of the data-directive list.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "data_directive_line.h"

int getDirectiveByteWidth(Directive directive)
{
    switch (directive) {
        case DIRECTIVE_DB:
        case DIRECTIVE_ASCIZ:
            return 1;
        case DIRECTIVE_DW:
            return 4;
        case DIRECTIVE_DH:
            return 2;
        default:
            return -1;
    }
}

static void destroyDataDirectiveNode(void *node)
{
    DataDirectiveNode *data_node = (DataDirectiveNode *)node;
    free(data_node->values);
    free(data_node);
}

DataDirectiveList* createDataDirectiveList(void)
{
    return createPointerList(destroyDataDirectiveNode);
}

int appendDataDirective(DataDirectiveList *list, DataDirectiveNode *node)
{
    return appendPointer(list, node);
}

void destroyDataDirectiveList(DataDirectiveList *list)
{
    destroyPointerList(list);
}
