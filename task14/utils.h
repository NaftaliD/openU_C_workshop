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
 * Blanks str in place if it's a comment line (first non-blank char is ';').
 * @param str the line to strip
 */
void removeComments(char *str);

/**
 * Checks whether a line exceeds MAX_LINE_LENGTH.
 * @param str the line to check
 * @return TRUE if too long, FALSE otherwise
 */
Boolean isLineTooLong(const char *str);

/**
 * Checks whether a word is a reserved language word (mcro, mcroend, any
 * operation name, or any directive name).
 * @param word the word to check
 * @return TRUE if reserved, FALSE otherwise
 */
Boolean isReservedWord(const char *word);

/**
 * Checks a name against label syntax: must start with a letter, contain
 * only letters/digits after that, and be at most MAX_MACRO_NAME_LENGTH
 * characters long.
 * @param name the name to validate
 * @return TRUE if valid, FALSE otherwise
 */
Boolean isValidSymbolName(const char *name);

/**
 * Checks a name against macro-name syntax: same as isValidSymbolName but
 * also allows '_' after the first character.
 * @param name the name to validate
 * @return TRUE if valid, FALSE otherwise
 */
Boolean isValidMacroName(const char *name);

/**
 * Checks that a file can be opened for reading, without leaving it open.
 * @param filename the path to check
 * @return TRUE if the file could be opened for reading, FALSE otherwise
 */
Boolean canOpenForReading(const char *filename);

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
 * yields an empty string, not NULL. NULL means allocation failure.
 * @param str the string to scan
 * @param delimiters characters that end the token
 * @param rest_out if non-NULL, receives a pointer into str at the delimiter
 *                 that ended the token (or at the terminating '\0')
 * @return a malloc'd copy of the token (caller must free), or NULL on
 *         allocation failure
 */
char* getFirstToken(const char *str, const char *delimiters, const char **rest_out);

/**
 * Returns a pointer into str past any leading spaces/tabs. Does not modify
 * str. Companion to getFirstToken, which leaves its rest_out pointing at
 * the delimiter itself rather than past it.
 * @param str the string to scan
 * @return pointer into str at the first non-space/tab character (or at '\0')
 */
const char* skipWhitespace(const char *str);

#endif /* UTILS_H */

