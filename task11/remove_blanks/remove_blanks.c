#include <stdio.h>
#include <ctype.h>

#define BUFF_SIZE 1024


/**
 * Removes all whitespace characters from a string in-place.
 * Iterates through the input string character by character, copying only
 * non-whitespace characters back to the beginning of the string, effectively
 * removing all spaces, tabs, newlines, and other whitespace.
 * @param char* str a pointer to the string to be processed
 * @note Modifies the input string directly
 * @return void
 */
void remove_blanks(char *str){
    char *src = str, *dst = str;
    printf("The string as received by the function:\n \"%s\"\n", str);
    while (*src) {
        if (!isspace(*src)) {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0'; /*Null-terminate the resulting string*/ 
    printf("The string at the end of the function:\n \"%s\"\n", str);
}

int main(){
    char input[BUFF_SIZE];
    char format_str[20];

    printf("Hello, please enter a string to be cleaned of all spaces!\n");

    sprintf(format_str, "%%%d[^\n]", BUFF_SIZE-1); /* Create a format string like "%1024[^\n]" */
    scanf(format_str, input); /* Read a line of input, up to BUFF_SIZE-1 characters */
     
    printf("You entered:\n \"%s\"\n", input);

    remove_blanks(input);

    return 0;
}