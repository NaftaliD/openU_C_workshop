#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*MACROS to calc diff and sum*/
#define DIFF(a, b) ((a) > (b) ? (a) - (b) : (b) - (a))
#define SUM(a, b) ((a) + (b))

#define INITIAL_SIZE 2
#define BUFF_SIZE 256

/*Function to calculate sum and difference, without using the MACROS*/
void sum_and_difference(int *sum, int *difference) {
    int a = *sum;
    int b = *difference;
    *sum = a + b;
    *difference = abs(a - b);
}

void saveArray(int a, int b, int* arr, int* size, int* max_size) {
    if (*size + 2 > *max_size) {
        *max_size *= 2;
        arr = realloc(arr, *max_size * sizeof(int));
        if (arr == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
    arr[(*size)++] = a;
    arr[(*size)++] = b;
}


int main() {
    int sum, difference;
    char input[BUFF_SIZE];

    int* arr = malloc(2 * sizeof(int) * INITIAL_SIZE);
    int size = 0;
    int max_size = INITIAL_SIZE;

    printf("Hello! This program calculates the sum and absolute difference of any two integers you provide. to end the program enter 'quit'\n\n");

    printf("Enter two integers to see their sum and difference (in absolute value): \n");
    scanf("%255[^\n]", input); /* Read a line of input, up to 255 characters */

    while (strcmp(input, "quit") != 0)
    {
        /* Parse the input to extract two integers */
        if (sscanf(input, "%d %d", &sum, &difference) != 2) {
            printf("Invalid input. Please enter two integers separated by space.\n");
            input[0] = '\0'; /* Clear the input buffer */
            scanf(" %255[^\n]", input); /* Read a line of input, up to 255 characters */
            continue;
        }

        /*  Save the enterd values in array & calculate sum and difference */
        saveArray(sum, difference, arr, &size, &max_size);
        sum_and_difference(&sum, &difference);

        /* Print the results, Once using the function and once using the Macros*/
        printf("Sum and difference calculated using the function:\n");
        printf("Sum: %d\n", sum);
        printf("Difference: %d\n", difference);
        printf("\n");
        printf("Sum and difference calculated using the MACROS:\n");
        printf("Sum: %d\n", SUM(arr[size - 2], arr[size - 1]));
        printf("Difference: %d\n", DIFF(arr[size - 2], arr[size - 1]));
        printf("\n");

        printf("Enter two integers to see their sum and difference (in absolute value) or 'quit' to exit: \n");
        input[0] = '\0'; /* Clear the input buffer */
        scanf(" %255[^\n]", input); /* Read a line of input, up to 255 characters */
    }
    
    printf("Goodbye! all entered value pairs are:\n");
    for (int i = 0; i < size; i += 2) {
        printf("%d %d\n", arr[i], arr[i + 1]);
    }

    if (arr) free(arr);
    return 0;
}