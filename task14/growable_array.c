/*
 * growable_array.c
 * Implementation of the shared growth-doubling primitive.
 */

#include <stdlib.h>
#include "growable_array.h"

int growArray(void **items, int *capacity, size_t element_size)
{
    void *temp;
    int new_capacity;

    if (items == NULL || capacity == NULL) {
        return 0;
    }

    new_capacity = *capacity * 2;
    temp = realloc(*items, element_size * new_capacity);
    if (temp == NULL) {
        return 0;
    }

    *items = temp;
    *capacity = new_capacity;
    return 1;
}
