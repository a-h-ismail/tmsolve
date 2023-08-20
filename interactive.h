/*
Copyright (C) 2022-2023 Ahmad Ismail
SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef INTERACTIVE_H
#define INTERACTIVE_H
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#ifndef LOCAL_BUILD
#include <tmsolve/libtmsolve.h>
#else
#include "libtmsolve.h"
#endif

#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
char **character_name_completion(const char *text, int start, int end);
#endif

#include <stdlib.h>

void get_input(char **buffer, char *prompt, size_t n);
void flush_stdin();
void scientific_mode();
void complex_mode();
void function_calculator();
void equation_mode();
void matrix_mode();
double get_value(char *prompt);
tms_matrix *matrix_input(int a, int b);
void matrix_print(tms_matrix *A);
void utility_mode();
void print_result(double complex result, bool verbose);
void equation_solver(int degree);
bool valid_mode(char mode);
void tic_tac_toe();

extern char _mode;
#ifdef USE_READLINE
extern char _autocomplete_mode;
#endif

#endif
