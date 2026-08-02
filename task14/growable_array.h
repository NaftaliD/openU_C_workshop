/*
 * growable_array.h
 * Shared growth-doubling primitive, used by every growable list in this
 * project plus macro_processor.c's macro-body buffer -- one place that owns
 * "double the capacity, realloc, only commit on success" instead of each
 * caller reimplementing it.
 */

#ifndef GROWABLE_ARRAY_H
#define GROWABLE_ARRAY_H

#include <stddef.h>
#include "common.h"

/**
 * Doubles the capacity of a growable array. Reallocates *items to
 * (*capacity * 2) * element_size bytes. On failure, *items and *capacity
 * are left completely untouched (the old, smaller allocation is still
 * valid) -- capacity is only updated after realloc is confirmed to succeed.
 * @param items pointer to the buffer pointer to grow
 * @param capacity pointer to its current capacity (in elements)
 * @param element_size size of one element in bytes
 * @return TRUE on success, FALSE on allocation failure
 */
Boolean growArray(void **items, int *capacity, size_t element_size);

#endif /* GROWABLE_ARRAY_H */
