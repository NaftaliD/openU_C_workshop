/*
 * utils.h
 * Utility functions for string handling and validation.
 */

#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include "common.h"

/**
 * Removes leading and trailing whitespace from a string, in place.
 * @param str the string to trim
 * @return str, for convenient chaining
 */
char* trimWhitespace(char *str);

/**
 * Truncates a line at the first ';' comment marker, in place.
 * @param str the line to strip
 */
void removeComments(char *str);

/**
 * Checks whether a line exceeds MAX_LINE_LENGTH.
 * @param str the line to check
 * @return 1 if too long, 0 otherwise
 */
int isLineTooLong(const char *str);

/**
 * Checks whether a word is a reserved language word (mcro, mcroend, any
 * operation name, or any directive name).
 * @param word the word to check
 * @return 1 if reserved, 0 otherwise
 */
int isReservedWord(const char *word);

/**
 * Checks a name against label/macro-name syntax: must start with a letter,
 * contain only letters/digits after that, and be at most
 * MAX_MACRO_NAME_LENGTH characters long.
 * @param name the name to validate
 * @return 1 if valid, 0 otherwise
 */
int isValidSymbolName(const char *name);

/**
 * Allocates and returns a copy of str (C90 has no strdup). Caller must free
 * the returned string.
 * @param str the string to copy
 * @return a malloc'd copy, or NULL if str is NULL or on allocation failure
 */
char* duplicateString(const char *str);

/**
 * Extracts the first token from str, ending at any character in delimiters
 * or at the end of the string. Does not modify str. An empty leading token
 * (str starts with a delimiter, or is empty) yields an empty string, not
 * NULL -- NULL means allocation failure, not "no token".
 * @param str the string to scan
 * @param delimiters characters that end the token
 * @param rest_out if non-NULL, receives a pointer into str at the delimiter
 *                 that ended the token (or at the terminating '\0')
 * @return a malloc'd copy of the token (caller must free), or NULL on
 *         allocation failure
 */
char* getFirstToken(const char *str, const char *delimiters, const char **rest_out);

#endif /* UTILS_H */

