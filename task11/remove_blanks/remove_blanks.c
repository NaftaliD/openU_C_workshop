#include <stdio.h>
#include <ctype.h>

#define BUFF_SIZE 256


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

    printf("Hello, please enter a string to be cleaned of all spaces!\n");

    scanf("%255[^\n]", input); /* Read a line of input, up to 255 characters */
     
    printf("You entered:\n \"%s\"\n", input);

    remove_blanks(input);

    return 0;
}