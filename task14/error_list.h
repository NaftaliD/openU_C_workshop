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

/* Growable array of structs */
typedef struct {
    ErrorNode *items;
    int count;
    int capacity;
    int had_fatal_error; /* set if appendError itself ever failed to grow
                            (allocation failure) while trying to record an
                            error -- so an OOM while logging an error is
                            never silently indistinguishable from "no errors" */
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
 * @return 0 on success, -1 on allocation failure (list->had_fatal_error is
 *         also set in that case)
 */
int appendError(ErrorList *list, int line_number, const char *message);

/**
 * Reports whether the file this list belongs to should be treated as
 * having errors: either a real error was recorded, or appendError itself
 * once failed to grow while trying to record one.
 * @param list the error list
 * @return 1 if the file has errors (or a logging failure occurred), 0 otherwise
 */
int errorListHasErrors(const ErrorList *list);

/**
 * Frees the backing array and the list itself.
 * @param list the error list to free
 */
void destroyErrorList(ErrorList *list);

/**
 * Builds a debug string describing every recorded error. Caller must free
 * the returned string.
 * @param list the error list to describe
 * @return a malloc'd description string
 */
char* errorListToString(const ErrorList *list);

#endif /* ERROR_LIST_H */
