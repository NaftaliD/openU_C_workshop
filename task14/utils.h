/*
 * utils.h
 * Utility functions for string handling and validation.
 */

#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include "common.h"

/*
 * trimWhitespace
 * Removes leading and trailing whitespace from a string (in-place).
 */
char* trimWhitespace(char *str);

/*
 * removeComments
 * Removes inline comments (everything after ';') from a line.
 */
void removeComments(char *str);

/*
 * isLineTooLong
 * Checks if a line exceeds MAX_LINE_LENGTH (80 characters).
 * Returns 1 if too long, 0 if valid.
 */
int isLineTooLong(const char *str);

/*
 * isReservedWord
 * Checks if a word is a reserved language word.
 * Reserved: mcro, mcroend, all operations, all directives.
 * Returns 1 if reserved, 0 if valid for macro name.
 */
int isReservedWord(const char *word);

/*
 * isValidSymbolName
 * Checks a name against the label/macro-name syntax rules
 * must start with an alphabetic character, followed only by alphanumeric characters, 
 * and be at most MAX_MACRO_NAME_LENGTH characters long.
 * Returns 1 if valid, 0 if invalid.
 */
int isValidSymbolName(const char *name);

#endif /* UTILS_H */

