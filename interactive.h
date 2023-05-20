/*
Copyright (C) 2021-2023 Ahmad Ismail
SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef INTERACTIVE_H
#define INTERACTIVE_H
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#ifndef LOCAL_BUILD
#include <tmsolve/libtmsolve.h>
#else
#include "libtmsolve/libtmsolve.h"
#endif

#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
void get_input(char **buffer, char *prompt, size_t n);
void flush_stdin();
void scientific_mode();
void complex_mode();
void function_calculator();
void equation_mode();
void matrix_mode();
double get_value(char *prompt);
matrix_str *matrix_input(int a, int b);
void matrix_print(matrix_str *A);
void utility_mode();
void print_result(double complex result, bool verbose);
void equation_solver(int degree);

extern char _mode;
#endif
