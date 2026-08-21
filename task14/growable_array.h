/*
 * growable_array.h
 * Shared growth-doubling primitive used by every growable list in this
 * project (and macro_processor.c's macro-body buffer).
 */

#ifndef GROWABLE_ARRAY_H
#define GROWABLE_ARRAY_H

#include <stddef.h>
#include "common.h"

/**
 * Doubles the capacity of a growable array. Reallocates *items to
 * (*capacity * 2) * element_size bytes. On failure, both are left
 * untouched.
 * @param items pointer to the buffer pointer to grow
 * @param capacity pointer to its current capacity (in elements)
 * @param element_size size of one element in bytes
 * @return TRUE on success, FALSE on allocation failure
 */
Boolean growArray(void **items, int *capacity, size_t element_size);

#endif /* GROWABLE_ARRAY_H */
