/*
 * error_list.h
 * Accumulates every error found while assembling a file, so scanning can
 * continue past the first error before deciding whether to produce output.
 */

#ifndef ERROR_LIST_H
#define ERROR_LIST_H

#include "common.h"

typedef struct {
    int  line_number;
    char message[MAX_ERROR_MESSAGE_LENGTH];
} ErrorNode;

/* Growable array of structs; count doubles as the "any errors?" gate */
typedef struct {
    ErrorNode *items;
    int count;
    int capacity;
} ErrorList;

/**
 * Allocates and initializes an empty error list.
 * @return the new list, or NULL on allocation failure
 */
ErrorList* createErrorList(void);

/**
 * Records an error at the given line number (message copied, truncated to
 * fit if needed).
 * @param list the error list
 * @param line_number source line the error occurred on
 * @param message description of the error
 * @return 0 on success, -1 on allocation failure
 */
int appendError(ErrorList *list, int line_number, const char *message);

/**
 * Frees the backing array and the list itself.
 * @param list the error list to free
 */
void destroyErrorList(ErrorList *list);

#endif /* ERROR_LIST_H */
