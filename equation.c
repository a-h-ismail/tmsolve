/*
Copyright (C) 2021-2023 Ahmad Ismail
SPDX-License-Identifier: GPL-3-only
*/
#include "interactive.h"

// Function to help with printing formatted equations to stdout.
void _print_eqt(char *format, double value, bool is_first)
{
    // Printing nothing if the value is 0 or -0
    if (fabs(value) == 0)
        return;
    if (is_first == true)
    {
        if (value > 0)
            printf(" ");
        else
            printf(" - ");
    }
    else
    {
        if (value > 0)
            printf(" + ");
        else
            printf(" - ");
    }
    printf(format, fabs(value));
}

void equation_solver(int degree)
{
    double a, b, c, d, p, q, delta;
    double complex x1, x2, x3, cdelta;
    switch (degree)
    {
    case 1:
        puts("a*x = b");
        do
        {
            a = get_value("a = ");
            if (a == 0)
                puts("Variable 'a' can't be zero");
        } while (a == 0);

        b = get_value("b = ");

        printf("Equation: %.10g x = %.10g\n", a, b);
        printf("Solution: x = %.10g\n", b / a);
        break;

    case 2:
        puts("a*x^2 + b*x + c = 0");
        do
        {
            a = get_value("a = ");
            if (a == 0)
                puts("Variable 'a' can't be zero");
        } while (a == 0);
        b = get_value("b = ");
        c = get_value("c = ");

        printf("\nEquation:");
        _print_eqt("%.10g x^2", a, true);
        _print_eqt("%.10g x", b, false);
        _print_eqt("%.10g", c, false);
        printf(" = 0\nSolutions:\n");

        delta = b * b - 4 * a * c;
        if (delta == 0)
        {
            printf("x1 = x2 = %.10g", -b / (2 * a));
        }
        else if (delta > 0)
        {
            printf("x1 = %.10g\n", (-b - sqrt(delta)) / (2 * a));
            printf("x2 = %.10g", (-b + sqrt(delta)) / (2 * a));
        }
        else if (delta < 0)
        {
            printf("x1 =");
            x1 = -b / (2 * a) + I * sqrt(-delta) / (2 * a);
            print_result(x1, false);
            printf("\nx2 =");
            x2 = conj(x1);
            print_result(x2, false);
        }
        break;

    case 3:
        puts("a*x^3 + b*x^2 + c*x + d = 0");
        do
        {
            a = get_value("a = ");
            if (a == 0)
                puts("Variable 'a' can't be zero");
        } while (a == 0);

        b = get_value("b = ");
        c = get_value("c = ");
        d = get_value("d = ");
        p = (3 * a * c - b * b) / (3 * a * a);
        q = (2 * b * b * b - 9 * a * b * c + 27 * a * a * d) / (27 * a * a * a);
        // delta1
        cdelta = q * q + (4 * p * p * p) / 27;
        cdelta = csqrt(cdelta);
        x1 = cbrt((-q - cdelta) / 2) + cbrt((-q + cdelta) / 2) - b / (3 * a);
        // delta2
        cdelta = cpow(b + a * x1, 2) - 4 * a * (c + x1 * (b + a * x1));
        cdelta = csqrt(cdelta);
        x2 = (-b - a * x1 - cdelta) / (2 * a);
        x3 = (-b - a * x1 + cdelta) / (2 * a);

        printf("\nEquation: ");
        _print_eqt("%.10g x^3", a, true);
        _print_eqt("%.10g x^2", b, false);
        _print_eqt("%.10g x", c, false);
        _print_eqt("%.10g", d, false);

        printf(" = 0\nSolutions:\n");
        if (x1 == x2 && x1 == x3)
            printf("x1 = x2 = x3 = %.10g", creal(x1));
        else if (x2 == x3)
        {
            printf("x1 =");
            print_result(x1, false);
            printf("\nx2 = x3 =");
            print_result(x2, false);
        }
        else
        {
            printf("x1 =");
            print_result(x1, false);
            printf("\nx2 =");
            print_result(x2, false);
            printf("\nx3 =");
            print_result(x3, false);
        }
        
        break;

    default:
        puts("Degree not supported.");
    }
    printf("\n");
}