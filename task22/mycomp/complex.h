#ifndef COMPLEX_H
#define COMPLEX_H

#include "common.h"

/* Complex number type definition */
typedef struct {
    double real;
    double imaginary;
} complex;

/* Function prototypes for complex number operations */

/**
 * Initializes a complex number to zero (0+0i).
 * @param c pointer to the complex number to initialize
 */
void init_complex_zero(complex *c);

/**
 * Adds two complex numbers: (a + bi) + (c + di) = (a+c) + (b+d)i
 * @param a first complex number
 * @param b second complex number
 * @return the sum of the two complex numbers
 */
complex add_complex(complex a, complex b);

/**
 * Subtracts two complex numbers: (a + bi) - (c + di) = (a-c) + (b-d)i
 * @param a first complex number
 * @param b second complex number
 * @return the difference of the two complex numbers
 */
complex sub_complex(complex a, complex b);

/**
 * Multiplies a complex number by a real number: real_num × (a + bi) = (real_num×a) + (real_num×b)i
 * @param c the complex number
 * @param real_num the real number multiplier
 * @return the product
 */
complex mult_complex_real(complex c, double real_num);

/**
 * Multiplies a complex number by an imaginary number: i×img_coeff × (a + bi) = -img_coeff×b + (img_coeff×a)i
 * @param c the complex number
 * @param img_coeff the imaginary coefficient (without the 'i')
 * @return the product
 */
complex mult_complex_img(complex c, double img_coeff);

/**
 * Multiplies two complex numbers: (a + bi)(c + di) = (ac - bd) + (ad + bc)i
 * @param a first complex number
 * @param b second complex number
 * @return the product
 */
complex mult_complex_complex(complex a, complex b);

/**
 * Calculates the absolute value of a complex number: |a + bi| = √(a² + b²)
 * @param c the complex number
 * @return the absolute value (real number)
 */
double abs_complex(complex c);

/**
 * Prints a complex number in the required format: real_part + (imag_part)i
 * @param c the complex number to print
 */
void print_complex(complex c);

#endif /* COMPLEX_H */