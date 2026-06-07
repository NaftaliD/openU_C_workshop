#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*MACROS to calc diff and sum*/
#define DIFF(a, b) ((a) > (b) ? (a) - (b) : (b) - (a))
#define SUM(a, b) ((a) + (b))

#define INITIAL_SIZE 2
#define BUFF_SIZE 256

/**
 * Calculates the sum and absolute difference of two integers.
 * Takes two integers passed by reference, computes their sum and absolute
 * difference, and stores the results back into the same variables. This
 * implementation uses standard operations rather than the predefined MACROS.
 * @param int* sum pointer to the first integer; stores the sum result
 * @param int* difference pointer to the second integer; stores the absolute difference result
 * @return void
 */
void sum_and_difference(int *sum, int *difference) {
    int a = *sum;
    int b = *difference;
    *sum = a + b;
    *difference = abs(a - b);
}

/**
 * Saves a pair of integers to a dynamic array with automatic resizing.
 * Appends two integer values to a dynamic array, automatically doubling the
 * allocated memory when needed to accommodate new elements. Handles memory
 * allocation failures by printing an error and exiting the program.
 * @param int a the first integer to save
 * @param int b the second integer to save
 * @param int** arr pointer to pointer to the dynamic array
 * @param int* size pointer to the current number of elements in the array
 * @param int* max_size pointer to the current maximum capacity of the array
 * @return void
 */
void saveArray(int a, int b, int** arr, int* size, int* max_size) {
    if (*size + 2 > *max_size) {
        *max_size *= 2;
        *arr = realloc(*arr, *max_size * sizeof(int));
        if (*arr == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
    (*arr)[(*size)++] = a;
    (*arr)[(*size)++] = b;
}


int main() {
    int sum, difference;
    char input[BUFF_SIZE];

    int* arr = malloc(2 * sizeof(int) * INITIAL_SIZE);
    int size = 0, i;
    int max_size = INITIAL_SIZE;

    printf("Hello! This program calculates the sum and absolute difference of any two integers you provide. To end the program press Ctrl+Z then Enter (Windows) or Ctrl+D (Unix/Linux)\n\n");

    printf("Enter two integers to see their sum and difference (in absolute value): \n");

    while (fgets(input, BUFF_SIZE, stdin) != NULL)
    {
        /* Remove trailing newline if present */
        int len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }
        /* Parse the input to extract two integers */
        if (sscanf(input, "%d %d", &sum, &difference) != 2) {
            printf("Invalid input. Please enter two integers separated by space.\n");
            printf("Enter two integers to see their sum and difference (in absolute value): \n");
            continue;
        }

        printf("You entered: %d and %d\n", sum, difference);

        /*  Save the enterd values in array & calculate sum and difference */
        saveArray(sum, difference, &arr, &size, &max_size);
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

        printf("Enter two integers to see their sum and difference (in absolute value): \n");
    }
    
    printf("Goodbye! all entered value pairs are:\n");
    for (i = 0; i < size; i += 2) {
        printf("%d %d\n", arr[i], arr[i + 1]);
    }

    if (arr) free(arr);
    return 0;
}