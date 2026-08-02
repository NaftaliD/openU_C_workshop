/*
 * error_list.c
 * Implementation of the error list.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "error_list.h"
#include "growable_array.h"

ErrorList* createErrorList(void)
{
    ErrorList *list = (ErrorList *)malloc(sizeof(ErrorList));
    if (list == NULL) {
        return NULL;
    }

    list->items = (ErrorNode *)malloc(sizeof(ErrorNode) * INITIAL_LIST_CAPACITY);
    if (list->items == NULL) {
        free(list);
        return NULL;
    }

    list->count = 0;
    list->capacity = INITIAL_LIST_CAPACITY;
    list->had_fatal_error = 0;
    return list;
}

Boolean appendError(ErrorList *list, int line_number, const char *message)
{
    if (list == NULL || message == NULL) {
        return FALSE;
    }

    if (list->count == list->capacity) {
        if (!growArray((void **)&list->items, &list->capacity, sizeof(ErrorNode))) {
            list->had_fatal_error = 1;
            return FALSE;
        }
    }

    list->items[list->count].line_number = line_number;
    strncpy(list->items[list->count].message, message, MAX_ERROR_MESSAGE_LENGTH - 1);
    list->items[list->count].message[MAX_ERROR_MESSAGE_LENGTH - 1] = '\0';
    list->count++;
    return TRUE;
}

Boolean errorListHasErrors(const ErrorList *list)
{
    if (list == NULL) {
        return FALSE;
    }
    return list->count > 0 || list->had_fatal_error;
}

void destroyErrorList(ErrorList *list)
{
    if (list == NULL) {
        return;
    }

    free(list->items);
    free(list);
}

char* errorListToString(const ErrorList *list)
{
    char buffer[LARGE_BUFF_SIZE];
    int pos = 0;
    int i;
    char *result;

    if (list == NULL) {
        result = (char *)malloc(NULL_PLACEHOLDER_SIZE);
        if (result != NULL) {
            strcpy(result, "[ErrorList: NULL]\n");
        }
        return result;
    }

    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                    "[ErrorList: %d error(s), had_fatal_error=%d]\n",
                    list->count, list->had_fatal_error);

    for (i = 0; i < list->count && pos < LARGE_BUFF_SIZE - ERROR_LIST_MARGIN; i++) {
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, "  line %d: %s\n",
                        list->items[i].line_number, list->items[i].message);
    }

    result = (char *)malloc(strlen(buffer) + 1);
    if (result != NULL) {
        strcpy(result, buffer);
    }
    return result;
}
