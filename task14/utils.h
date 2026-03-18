/*
 * utils.h
 * Utility functions for string handling and validation.
 */

#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include "common.h"

/*
 * StringUtils_trimWhitespace
 * Removes leading and trailing whitespace from a string (in-place).
 */
char* StringUtils_trimWhitespace(char *str);

/*
 * StringUtils_removeComments
 * Removes inline comments (everything after ';') from a line.
 */
void StringUtils_removeComments(char *str);

/*
 * ValidUtils_isLineTooLong
 * Checks if a line exceeds MAX_LINE_LENGTH (80 characters).
 * Returns 1 if too long, 0 if valid.
 */
int ValidUtils_isLineTooLong(const char *str);

/*
 * ValidUtils_isReservedWord
 * Checks if a word is a reserved language word.
 * Reserved: mcro, mcroend, all operations, all directives.
 * Returns 1 if reserved, 0 if valid for macro name.
 */
int ValidUtils_isReservedWord(const char *word);

#endif /* UTILS_H */

