/*
 * pointer_list.c
 * Implementation of the generic pointer list.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "pointer_list.h"

PointerList* createPointerList(ElementDestructor destroy_element)
{
    PointerList *list = (PointerList *)malloc(sizeof(PointerList));
    if (list == NULL) {
        return NULL;
    }

    list->items = (void **)malloc(sizeof(void *) * INITIAL_LIST_CAPACITY);
    if (list->items == NULL) {
        free(list);
        return NULL;
    }

    list->count = 0;
    list->capacity = INITIAL_LIST_CAPACITY;
    list->destroy_element = destroy_element;
    return list;
}

int appendPointer(PointerList *list, void *item)
{
    void **temp;
    int new_capacity;

    if (list == NULL || item == NULL) {
        return -1;
    }

    if (list->count == list->capacity) {
        new_capacity = list->capacity * 2;
        temp = (void **)realloc(list->items, sizeof(void *) * new_capacity);
        if (temp == NULL) {
            return -1;
        }
        list->items = temp;
        list->capacity = new_capacity;
    }

    list->items[list->count] = item;
    list->count++;
    return 0;
}

void destroyPointerList(PointerList *list)
{
    int i;

    if (list == NULL) {
        return;
    }

    for (i = 0; i < list->count; i++) {
        if (list->destroy_element != NULL) {
            list->destroy_element(list->items[i]);
        }
    }

    free(list->items);
    free(list);
}
