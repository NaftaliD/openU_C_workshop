/*
 * operation_line.c
 * Implementation of the operation list.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "operation_line.h"
#include "growable_array.h"

OperationNode* createOperationNode(void)
{
    return (OperationNode *)malloc(sizeof(OperationNode));
}

OperationList* createOperationList(void)
{
    OperationList *list = (OperationList *)malloc(sizeof(OperationList));
    if (list == NULL) {
        return NULL;
    }

    list->items = (OperationNode **)malloc(sizeof(OperationNode *) * INITIAL_LIST_CAPACITY);
    if (list->items == NULL) {
        free(list);
        return NULL;
    }

    list->count = 0;
    list->capacity = INITIAL_LIST_CAPACITY;
    return list;
}

int appendOperation(OperationList *list, OperationNode *node)
{
    if (list == NULL || node == NULL) {
        return -1;
    }

    if (list->count == list->capacity) {
        if (!growArray((void **)&list->items, &list->capacity, sizeof(OperationNode *))) {
            return -1;
        }
    }

    list->items[list->count] = node;
    list->count++;
    return 0;
}

void destroyOperationList(OperationList *list)
{
    int i;

    if (list == NULL) {
        return;
    }

    for (i = 0; i < list->count; i++) {
        free(list->items[i]);
    }

    free(list->items);
    free(list);
}

/**
 * Formats a single operand as it appears in source syntax ("$N", a decimal
 * immediate, or a label name).
 * @param operand the operand to format
 * @param buffer receives the formatted text
 * @param buffer_size size of buffer
 */
static void operandToString(const Operand *operand, char *buffer, size_t buffer_size)
{
    switch (operand->type) {
        case OPERAND_REGISTER:
            snprintf(buffer, buffer_size, "$%d", operand->value.register_number);
            break;
        case OPERAND_IMMEDIATE:
            snprintf(buffer, buffer_size, "%ld", operand->value.immediate_value);
            break;
        case OPERAND_LABEL_REF:
            snprintf(buffer, buffer_size, "%s", operand->value.label_name);
            break;
    }
}

char* operationNodeToString(const OperationNode *node)
{
    char buffer[512];
    char operand_buf[40];
    int pos = 0;
    int i;
    char *result;

    if (node == NULL) {
        result = (char *)malloc(30);
        if (result != NULL) {
            strcpy(result, "[OperationNode: NULL]\n");
        }
        return result;
    }

    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                    "[OperationNode line=%d op=%s operands=(",
                    node->line_number, operationToString(node->op));

    for (i = 0; i < node->operand_count; i++) {
        operandToString(&node->operands[i], operand_buf, sizeof(operand_buf));
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%s%s",
                        i > 0 ? "," : "", operand_buf);
    }

    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                    ") address=%d has_error=%d]\n", node->address, node->has_error);

    result = (char *)malloc(strlen(buffer) + 1);
    if (result != NULL) {
        strcpy(result, buffer);
    }
    return result;
}

char* operationListToString(const OperationList *list)
{
    char buffer[8192];
    int pos = 0;
    int i;
    char *node_str;
    char *result;

    if (list == NULL) {
        result = (char *)malloc(50);
        if (result != NULL) {
            strcpy(result, "[OperationList: NULL]\n");
        }
        return result;
    }

    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                    "[OperationList: %d operations]\n", list->count);

    for (i = 0; i < list->count && pos < (int)sizeof(buffer) - 500; i++) {
        node_str = operationNodeToString(list->items[i]);
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%s", node_str);
        free(node_str);
    }

    result = (char *)malloc(strlen(buffer) + 1);
    if (result != NULL) {
        strcpy(result, buffer);
    }
    return result;
}
