#include <string.h>
#include <ctype.h>
#include "common.h"

void trim_whitespace(char *str) {
    char *start;
    char *end;
    size_t len;

    if (str == NULL) {
        return;
    }

    start = str;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }

    if (start != str) {
        len = strlen(start);
        memmove(str, start, len + 1);
    }

    end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
}

int is_blank_string(const char *str) {
    if (str == NULL) {
        return 1;
    }

    while (*str != '\0') {
        if (!isspace((unsigned char)*str)) {
            return 0;
        }
        str++;
    }

    return 1;
}
