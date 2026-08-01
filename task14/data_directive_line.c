/*
 * data_directive_line.c
 * Implementation of the data-directive list.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "data_directive_line.h"
#include "growable_array.h"

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

DataDirectiveNode* createDataDirectiveNode(void)
{
    return (DataDirectiveNode *)malloc(sizeof(DataDirectiveNode));
}

DataDirectiveList* createDataDirectiveList(void)
{
    DataDirectiveList *list = (DataDirectiveList *)malloc(sizeof(DataDirectiveList));
    if (list == NULL) {
        return NULL;
    }

    list->items = (DataDirectiveNode **)malloc(sizeof(DataDirectiveNode *) * INITIAL_LIST_CAPACITY);
    if (list->items == NULL) {
        free(list);
        return NULL;
    }

    list->count = 0;
    list->capacity = INITIAL_LIST_CAPACITY;
    return list;
}

int appendDataDirective(DataDirectiveList *list, DataDirectiveNode *node)
{
    if (list == NULL || node == NULL) {
        return -1;
    }

    if (list->count == list->capacity) {
        if (!growArray((void **)&list->items, &list->capacity, sizeof(DataDirectiveNode *))) {
            return -1;
        }
    }

    list->items[list->count] = node;
    list->count++;
    return 0;
}

int appendDirectiveValue(DataDirectiveNode *node, long value)
{
    if (node->values == NULL) {
        node->values = (long *)malloc(sizeof(long) * INITIAL_LIST_CAPACITY);
        if (node->values == NULL) {
            return -1;
        }
        node->value_capacity = INITIAL_LIST_CAPACITY;
        node->value_count = 0;
    } else if (node->value_count == node->value_capacity) {
        if (!growArray((void **)&node->values, &node->value_capacity, sizeof(long))) {
            return -1;
        }
    }

    node->values[node->value_count] = value;
    node->value_count++;
    return 0;
}

void relocateDataDirectiveAddresses(DataDirectiveList *list, int icf)
{
    int i;

    if (list == NULL) {
        return;
    }

    for (i = 0; i < list->count; i++) {
        list->items[i]->address += icf;
    }
}

void destroyDataDirectiveList(DataDirectiveList *list)
{
    int i;

    if (list == NULL) {
        return;
    }

    for (i = 0; i < list->count; i++) {
        free(list->items[i]->values);
        free(list->items[i]);
    }

    free(list->items);
    free(list);
}

char* dataDirectiveNodeToString(const DataDirectiveNode *node)
{
    char buffer[1024];
    int pos = 0;
    int i;
    char *result;

    if (node == NULL) {
        result = (char *)malloc(30);
        if (result != NULL) {
            strcpy(result, "[DataDirectiveNode: NULL]\n");
        }
        return result;
    }

    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                    "[DataDirectiveNode line=%d directive=%s values=(",
                    node->line_number, directiveToString(node->directive));

    /* Bounded like dataDirectiveListToString's own loop -- value_count is
     * unbounded, and without this, pos could run past buffer's end. */
    for (i = 0; i < node->value_count && pos < (int)sizeof(buffer) - 100; i++) {
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%s%ld",
                        i > 0 ? "," : "", node->values[i]);
    }

    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                    ") address=%d has_error=%d]\n", node->address, node->has_error);

    result = (char *)malloc(strlen(buffer) + 1);
    if (result != NULL) {
        strcpy(result, buffer);
    }
    return result;
}

char* dataDirectiveListToString(const DataDirectiveList *list)
{
    char buffer[8192];
    int pos = 0;
    int i;
    char *node_str;
    char *result;

    if (list == NULL) {
        result = (char *)malloc(50);
        if (result != NULL) {
            strcpy(result, "[DataDirectiveList: NULL]\n");
        }
        return result;
    }

    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                    "[DataDirectiveList: %d directives]\n", list->count);

    for (i = 0; i < list->count && pos < (int)sizeof(buffer) - 500; i++) {
        node_str = dataDirectiveNodeToString(list->items[i]);
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%s", node_str);
        free(node_str);
    }

    result = (char *)malloc(strlen(buffer) + 1);
    if (result != NULL) {
        strcpy(result, buffer);
    }
    return result;
}
