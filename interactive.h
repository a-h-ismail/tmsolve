/*
Copyright (C) 2021-2022 Ahmad Ismail
SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef INTERACTIVE_H
#define INTERACTIVE_H
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include "libtmsolve.h"
// Defining the clear console command
#if defined(__linux__)
#include <readline/readline.h>
#include <readline/history.h>
#define CLEAR_CONSOLE "clear"
#elif defined(_WIN64)
#define CLEAR_CONSOLE "cls"
char *readline(char *prompt);
#endif
#include <stdlib.h>
void s_input(char **buffer, char *prompt, size_t n);
void flush_stdin();
void scientific_calculator(char *expr, bool calledbydefault);
void scientific_complex_picker(char *expr);
void complex_mode();
void function_calculator();
void equation_mode();
void matrix_mode();
double get_value(char *prompt);
matrix_str *matrix_input(int a, int b);
void matrix_print(matrix_str *A);
void utility_mode();
void utility_functions(char *expr);
#endif
