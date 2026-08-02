/*
 * growable_array.c
 * Implementation of the shared growth-doubling primitive.
 */

#include <stdlib.h>
#include "growable_array.h"

Boolean growArray(void **items, int *capacity, size_t element_size)
{
    void *temp;
    int new_capacity;

    if (items == NULL || capacity == NULL) {
        return FALSE;
    }

    new_capacity = *capacity * 2;
    temp = realloc(*items, element_size * new_capacity);
    if (temp == NULL) {
        return FALSE;
    }

    *items = temp;
    *capacity = new_capacity;
    return TRUE;
}
