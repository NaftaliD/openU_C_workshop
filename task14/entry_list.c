/*
 * entry_list.c
 * Implementation of the entry list.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "entry_list.h"
#include "growable_array.h"

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
    if (list == NULL || symbol_name == NULL) {
        return -1;
    }

    if (strlen(symbol_name) > MAX_LABEL_LENGTH) {
        return -1;
    }

    if (list->count == list->capacity) {
        if (!growArray((void **)&list->items, &list->capacity, sizeof(EntryNode))) {
            return -1;
        }
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

char* entryListToString(const EntryList *list)
{
    char buffer[4096];
    int pos = 0;
    int i;
    char *result;

    if (list == NULL) {
        result = (char *)malloc(30);
        if (result != NULL) {
            strcpy(result, "[EntryList: NULL]\n");
        }
        return result;
    }

    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                    "[EntryList: %d entries]\n", list->count);

    for (i = 0; i < list->count && pos < (int)sizeof(buffer) - 100; i++) {
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, "  line %d: %s\n",
                        list->items[i].line_number, list->items[i].symbol_name);
    }

    result = (char *)malloc(strlen(buffer) + 1);
    if (result != NULL) {
        strcpy(result, buffer);
    }
    return result;
}
