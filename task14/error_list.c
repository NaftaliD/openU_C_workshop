/*
 * error_list.c
 * Implementation of the error list.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "error_list.h"

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
    return list;
}

int appendError(ErrorList *list, int line_number, const char *message)
{
    ErrorNode *temp;
    int new_capacity;

    if (list == NULL || message == NULL) {
        return -1;
    }

    if (list->count == list->capacity) {
        new_capacity = list->capacity * 2;
        temp = (ErrorNode *)realloc(list->items, sizeof(ErrorNode) * new_capacity);
        if (temp == NULL) {
            return -1;
        }
        list->items = temp;
        list->capacity = new_capacity;
    }

    list->items[list->count].line_number = line_number;
    strncpy(list->items[list->count].message, message, MAX_ERROR_MESSAGE_LENGTH - 1);
    list->items[list->count].message[MAX_ERROR_MESSAGE_LENGTH - 1] = '\0';
    list->count++;
    return 0;
}

void destroyErrorList(ErrorList *list)
{
    if (list == NULL) {
        return;
    }

    free(list->items);
    free(list);
}
