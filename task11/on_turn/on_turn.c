#include <stdio.h>
#include <ctype.h>

#define BIT_17_MASK (1L << 16)
#define BITS_IN_BYTE 8

/**
 * Prints the binary representation of a long integer.
 * Converts and displays a long integer as its binary equivalent by examining
 * each bit position from most significant to least significant and printing
 * '1' or '0' for each bit accordingly.
 * @param long n the long integer to print in binary
 * @return void
 */
void print_binary(long n) {
    int i;
    /* Go over all bits and using bitwise operations judge whether each bit is set */
    for (i=sizeof(long) * BITS_IN_BYTE - 1; i>=0; i--) {
        printf("%c", (n & (1L << i)) ? '1' : '0');
    }
}

/**
 * Checks if the 17th bit is set and turns it on if not already set.
 * Examines the 17th bit of the input number and either reports that it is
 * already set or sets the bit and displays the modified number in both
 * decimal and binary representations.
 * @param long n the long integer to process
 * @return void
 * @see print_binary()
 */
void on_turn(long n) {
    printf("inputted number in base 10: %ld\n", n);
    printf("inputted number in base 2: ");
    print_binary(n);
    printf("\n\n");

    printf("checking if turning the 17th bit changes the number: ");

    if (n & BIT_17_MASK) {
        printf("No change, the 17th bit is already set.\n");
    } else {
        printf("The number changed.\n");
        n |= BIT_17_MASK; /* Turn on the 17th bit */
        printf("resulting number in base 10: %ld\n", n);
        printf("resulting number in base 2: ");
        print_binary(n);
        printf("\n");
    }
    
}

int main() {
    long num;

    printf("Hello, please enter a number to see its binary representation and whether turning the 17th bit changes it!\n");

    scanf("%ld", &num); /* Read a long number as input */
    on_turn(num);
    return 0;
}