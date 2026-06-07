#include <stdio.h>
#include <math.h>
#include "complex.h"

/*
 * Initialize a complex number to zero
 */
void init_complex_zero(complex *c) {
    if (c != NULL) {
        c->real = 0.0;
        c->imaginary = 0.0;
    }
}

/*
 * Add two complex numbers: (a + bi) + (c + di) = (a+c) + (b+d)i
 */
complex add_complex(complex a, complex b) {
    complex result;
    result.real = a.real + b.real;
    result.imaginary = a.imaginary + b.imaginary;
    return result;
}

/*
 * Subtract two complex numbers: (a + bi) - (c + di) = (a-c) + (b-d)i
 */
complex sub_complex(complex a, complex b) {
    complex result;
    result.real = a.real - b.real;
    result.imaginary = a.imaginary - b.imaginary;
    return result;
}

/*
 * Multiply complex number by real number: real_num * (a + bi) = (real_num*a) + (real_num*b)i
 */
complex mult_complex_real(complex c, double real_num) {
    complex result;
    result.real = real_num * c.real;
    result.imaginary = real_num * c.imaginary;
    return result;
}

/*
 * Multiply complex number by imaginary: i * img_coeff * (a + bi) = -img_coeff*b + (img_coeff*a)i
 */
complex mult_complex_img(complex c, double img_coeff) {
    complex result;
    result.real = -img_coeff * c.imaginary;
    result.imaginary = img_coeff * c.real;
    return result;
}

/*
 * Multiply two complex numbers: (a + bi)(c + di) = (ac - bd) + (ad + bc)i
 */
complex mult_complex_complex(complex a, complex b) {
    complex result;
    result.real = (a.real * b.real) - (a.imaginary * b.imaginary);
    result.imaginary = (a.real * b.imaginary) + (a.imaginary * b.real);
    return result;
}

/*
 * Calculate absolute value of complex number: |a + bi| = sqrt(a^2 + b^2)
 */
double abs_complex(complex c) {
    return sqrt((c.real * c.real) + (c.imaginary * c.imaginary));
}

/*
 * Print complex number in required format: real_part + (imag_part)i
 */
void print_complex(complex c) {
    printf(OUTPUT_FORMAT_COMPLEX, c.real, c.imaginary);
}