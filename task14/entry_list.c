/*
 * entry_list.c
 * Implementation of the entry list.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "entry_list.h"

EntryList* createEntryList(void)
{
    EntryList *list = (EntryList *)malloc(sizeof(EntryList));
    if (list == NULL) {
        return NULL;
    }

    list->items = (EntryNode *)malloc(sizeof(EntryNode) * INITIAL_LIST_CAPACITY);
    if (list->items == NULL) {
        free(list);
        return NULL;
    }

    list->count = 0;
    list->capacity = INITIAL_LIST_CAPACITY;
    return list;
}

int appendEntry(EntryList *list, int line_number, const char *symbol_name)
{
    EntryNode *temp;
    int new_capacity;

    if (list == NULL || symbol_name == NULL) {
        return -1;
    }

    if (list->count == list->capacity) {
        new_capacity = list->capacity * 2;
        temp = (EntryNode *)realloc(list->items, sizeof(EntryNode) * new_capacity);
        if (temp == NULL) {
            return -1;
        }
        list->items = temp;
        list->capacity = new_capacity;
    }

    list->items[list->count].line_number = line_number;
    strcpy(list->items[list->count].symbol_name, symbol_name);
    list->count++;
    return 0;
}

void destroyEntryList(EntryList *list)
{
    if (list == NULL) {
        return;
    }

    free(list->items);
    free(list);
}
