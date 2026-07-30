/*
 * operation_line.c
 * Implementation of the operation list.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "operation_line.h"

static void destroyOperationNode(void *node)
{
    free(node);
}

OperationList* createOperationList(void)
{
    return createPointerList(destroyOperationNode);
}

int appendOperation(OperationList *list, OperationNode *node)
{
    return appendPointer(list, node);
}

void destroyOperationList(OperationList *list)
{
    destroyPointerList(list);
}
