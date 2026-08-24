/*
 * utils.c
 * Utility functions implementation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"
#include "common.h"

char* trimWhitespace(char *str)
{
    int start;
    int end;
    int i;
    
    if (str == NULL) {
        return str;
    }
    
    /* Find first non-whitespace character */
    start = 0;
    while (str[start] != '\0' && isspace((unsigned char)str[start])) {
        start++;
    }
    
    /* Find last non-whitespace character */
    end = strlen(str) - 1;
    while (end >= start && isspace((unsigned char)str[end])) {
        end--;
    }
    
    /* Shift characters to the beginning */
    if (start > 0) {
        for (i = 0; i <= end - start; i++) {
            str[i] = str[start + i];
        }
    }
    
    /* Null-terminate the trimmed string */
    if (end >= start) {
        str[end - start + 1] = '\0';
    } else {
        str[0] = '\0';
    }
    
    return str;
}

void removeComments(char *str)
{
    const char *p;

    if (str == NULL) {
        return;
    }

    p = skipWhitespace(str);
    if (*p == ';') {
        str[0] = '\0';
    }
}

Boolean isLineTooLong(const char *str)
{
    size_t len;

    if (str == NULL) {
        return FALSE;
    }

    len = strlen(str);

    /* A captured trailing newline means the line already fit the read
     * buffer, so it's in range by construction. Only the length matters
     * when no newline was captured (truncated, or a final line at EOF). */
    if (len > 0 && str[len - 1] == '\n') {
        return FALSE;
    }

    return len > MAX_LINE_LENGTH;
}

Boolean isReservedWord(const char *word)
{
    if (word == NULL || strlen(word) == 0) {
        return FALSE;
    }

    /* Check keywords */
    if (strcmp(word, "mcro") == 0 || strcmp(word, "mcroend") == 0) {
        return TRUE;
    }

    /* Check if it's a valid operation */
    if (operationFromString(word) != OP_INVALID) {
        return TRUE;
    }

    /* Check if it's a valid directive */
    if (directiveFromString(word) != DIRECTIVE_INVALID) {
        return TRUE;
    }

    return FALSE;
}

/**
 * Shared syntax check behind isValidSymbolName/isValidMacroName: letter,
 * then letters/digits (and '_' if allow_underscore) up to
 * MAX_MACRO_NAME_LENGTH characters.
 * @param name the name to validate
 * @param allow_underscore whether '_' is accepted after the first character
 * @return TRUE if valid, FALSE otherwise
 */
static Boolean isValidNameSyntax(const char *name, Boolean allow_underscore)
{
    int i;
    int len;

    if (name == NULL || name[0] == '\0') {
        return FALSE;
    }

    len = strlen(name);
    if (len > MAX_MACRO_NAME_LENGTH) {
        return FALSE;
    }

    if (!isalpha((unsigned char)name[0])) {
        return FALSE;
    }

    for (i = 1; i < len; i++) {
        if (!isalnum((unsigned char)name[i]) && !(allow_underscore && name[i] == '_')) {
            return FALSE;
        }
    }

    return TRUE;
}

Boolean isValidSymbolName(const char *name)
{
    return isValidNameSyntax(name, FALSE);
}

Boolean isValidMacroName(const char *name)
{
    return isValidNameSyntax(name, TRUE);
}

Boolean canOpenForReading(const char *filename)
{
    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        return FALSE;
    }
    fclose(fp);
    return TRUE;
}

char* duplicateString(const char *str)
{
    char *copy;
    size_t len;

    if (str == NULL) {
        return NULL;
    }

    len = strlen(str);
    copy = (char *)malloc(len + 1);
    if (copy == NULL) {
        return NULL;
    }

    strcpy(copy, str);
    return copy;
}

char* getFirstToken(const char *str, const char *delimiters, const char **rest_out)
{
    int i;
    char *token;

    if (str == NULL || delimiters == NULL) {
        return NULL;
    }

    i = 0;
    while (str[i] != '\0' && strchr(delimiters, str[i]) == NULL) {
        i++;
    }

    token = (char *)malloc(i + 1);
    if (token == NULL) {
        return NULL;
    }
    strncpy(token, str, i);
    token[i] = '\0';

    if (rest_out != NULL) {
        *rest_out = &str[i];
    }

    return token;
}

const char* skipWhitespace(const char *str)
{
    if (str == NULL) {
        return NULL;
    }

    while (*str == ' ' || *str == '\t') {
        str++;
    }

    return str;
}

