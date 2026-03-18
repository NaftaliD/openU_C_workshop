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

char* StringUtils_trimWhitespace(char *str)
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

void StringUtils_removeComments(char *str)
{
    int i;
    
    if (str == NULL) {
        return;
    }
    
    /* Find first semicolon and truncate at that position */
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] == ';') {
            str[i] = '\0';
            break;
        }
    }
}

int ValidUtils_isLineTooLong(const char *str)
{
    /* MAX_LINE_LENGTH is 80 characters per specification */
    if (str == NULL) {
        return 0;
    }
    
    return strlen(str) > MAX_LINE_LENGTH;
}

int ValidUtils_isReservedWord(const char *word)
{
    if (word == NULL || strlen(word) == 0) {
        return 0;
    }
    
    /* Check keywords */
    if (strcmp(word, "mcro") == 0 || strcmp(word, "mcroend") == 0) {
        return 1;
    }
    
    /* Check if it's a valid operation */
    if (OperationFromString(word) != OP_INVALID) {
        return 1;
    }
    
    /* Check if it's a valid directive (with leading dot) */
    if (DirectiveFromString(word) != DIRECTIVE_INVALID) {
        return 1;
    }
    
    return 0;
}

