/*
Copyright (C) 2022-2024 Ahmad Ismail
SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "interactive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>

#ifdef USE_READLINE
char *character_name_generator(const char *text, int state)
{
    static int next;
    static char **completions = NULL;

    if (!state)
    {
        next = 0;
        free(completions);
        switch (_autocomplete_mode)
        {
        case 'S':
            completions = tms_smode_autocompletion_helper(text);
            break;

        case 'I':
            completions = tms_imode_autocompletion_helper(text);
            break;
        }
    }
    if (completions == NULL)
        return NULL;
    else
        return completions[next++];
}

char **character_name_completion(const char *text, int start, int end)
{
    // Disable normal filename autocomplete
    rl_attempted_completion_over = 1;
    switch (_mode)
    {
    case 'S':
        rl_completer_word_break_characters = " +-*/^%(,";
        break;
    case 'I':
        rl_completer_word_break_characters = " +-*/^%&|(,";
        break;
    default:
        rl_completer_word_break_characters = rl_basic_word_break_characters;
    }
    return rl_completion_matches(text, character_name_generator);
}
#else
// This function mimics some of the behavior of GNU readline function (printing prompt and returning a malloc'd string)
char *readline(char *prompt)
{
    size_t count = 0, bsize = 1024;
    char *buffer = (char *)malloc(bsize * sizeof(char)), c;

    if (prompt != NULL)
        printf("%s", prompt);
    do
    {
        c = getc(stdin);
        if (c == EOF)
        {
            free(buffer);
            return NULL;
        }
        ++count;
        if (count > bsize)
        {
            bsize *= 2;
            buffer = realloc(buffer, bsize);
            if (buffer == NULL)
                return NULL;
        }
        buffer[count - 1] = c;
    } while (c != '\n');
    buffer[count - 1] = '\0';
    buffer = realloc(buffer, (count + 1) * sizeof(char));
    return buffer;
}

#endif

/*
  Reads n characters from stdin.
  If dest is set to NULL, the function will return a malloc'd char * array (ignoring the max size).
  Otherwise, the characters are directly written to "dest".
  Adds the valid input to readline's history.
*/
char *get_input(char *dest, char *prompt, size_t n)
{
    char *tmp;
    while (1)
    {
        tmp = readline(prompt);
        // Properly handle end of piped input
        if (tmp == NULL)
            exit(0);

        size_t length = strlen(tmp);
        if (length == 0)
        {
            puts(NO_INPUT "\n");
            free(tmp);
        }
        else if (length > n)
        {
            printf("Input is longer than expected (%zu characters).\n\n", n);
            free(tmp);
        }
        else
        {
            if (dest != NULL)
            {
                strcpy(dest, tmp);
#ifdef USE_READLINE
                add_history(tmp);
#endif
                free(tmp);
                tmp = NULL;
                break;
            }
            else
            {
#ifdef USE_READLINE
                // Check if the input line already exists at the end of the history to avoid duplicates
                HIST_ENTRY *last_entry = history_get(history_length);
                if (last_entry == NULL || strcmp(last_entry->line, tmp) != 0)
                    add_history(tmp);
#endif
                break;
            }
        }
    }
    return tmp;
}
// Simple function to flush stdin
void flush_stdin()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

// Function that should be called repetitively to cleanly print an array of chars as groups of 3
// When the array is done, provide NULL as argument to reset the internal state of the function
void print_array_of_chars_helper(char *next)
{
    static int i;
    if (next != NULL)
    {
        if (i > 0)
            putchar(',');
        if (i > 2)
        {
            i = 0;
            putchar('\n');
        }
        printf(" %s", next);
        ++i;
    }
    else
    {
        puts("\n");
        i = 0;
    }
}

// Handles "management" input for all modes
// Ex: exit, mode switching
// It is lazy because the input copy/free is done in the caller
int _management_input_lazy(char *input)
{
    char *token = strtok(input, " ");

    // Not supposed to happen normally
    if (token == NULL)
        exit(1);

    if (strcmp(token, "exit") == 0)
        exit(0);

    else if (strcmp(token, "mode") == 0)
    {
        token = strtok(NULL, " ");
        if (token == NULL)
        {
            puts("Available modes:\n"
                 "* Scientific (S)\n"
                 "* Integer (I)\n"
                 "* Function (F)\n"
                 "* Equation (E)\n"
                 "* Utility (U)\n\n"
                 "To switch between modes, add the correct letter after the command \"mode\"\n"
                 "Example: mode I\n");
            return NEXT_ITERATION;
        }
        else
        {
            if (strlen(token) > 1)
            {
                puts("Expected a single letter, type \"mode\" for help.\n");
                return NEXT_ITERATION;
            }
            else
            {
                token[0] = toupper(token[0]);

                if (valid_mode(token[0]))
                {
                    _mode = token[0];
                    return SWITCH_MODE;
                }
                else
                {
                    puts("Invalid mode. Type \"mode\" to see a list of all available modes.\n");
                    return NEXT_ITERATION;
                }
            }
        }
    }
    // Per mode help
    else if (strcmp("help", token) == 0)
    {
        token = strtok(NULL, " ");
        if (token == NULL)
        {
            switch (_mode)
            {
            case 'S':
                puts("Calculate a math expression.\n"
                     "This mode supports hex, oct and bin input using prefixes \"0x\", \"0o\" and \"0b\".\n"
                     "Operator priority groups (high to low): () ^ [ * / % ] [ + - ]\n"
                     "Supports user defined variables and functions.\n\n"
                     "Examples:\n\"v1=5*pi\" will assign 5*pi to the variable v1.\n"
                     "\"f(x)=x^2\" creates a new function that returns the square of its argument.\n\n"
                     "To view available functions, type \"functions\"\n"
                     "To view currently defined variables, type \"variables\"\n"
                     "Use \"debug\" and \"undebug\" to enable/disable debugging output.");
                break;
            case 'I':
                puts("Calculate a math expression.\n"
                     "Compared to scientific mode, this mode uses integers instead of double precision floats.\n"
                     "Supports hex, oct and bin input using prefixes \"0x\", \"0o\" and \"0b\".\n"
                     "Operator priority groups (high to low): () [ * / % ] [ + - ] & ^ |\n"
                     "Supports user defined variables.\n\n"
                     "Example: \"v1=791 & 0xFF\" will assign 791 & 0xFF to the variable v1.\n\n"
                     "To change current variable size, use the \"set\" keyword.\n"
                     "To view available functions, type \"functions\"\n"
                     "To view currently defined variables, type \"variables\"\n"
                     "Use \"debug\" and \"undebug\" to enable/disable debugging output.");
                break;
            case 'F':
                puts("Function mode calculates a function over a specified interval.\n"
                     "Provide the function and start, end, step to get the results.");
                break;
            case 'E':
                puts("Equation mode solves equations up to the third degree.\n"
                     "Enter the degree and follow the on screen instructions.");
                break;
            case 'U':
                puts("Utility mode is meant for useful functions that don't fit in any other mode.\n"
                     "Currently, only factor() is available.");
                break;
            case 'G':
                puts("You are playing against the computer, and expecting it to help you?");
                break;
            default:
                return NO_ACTION;
            }
            putchar('\n');
            return NEXT_ITERATION;
        }
        else
            return NO_ACTION;
    }

    // Specific to every mode
    switch (_mode)
    {
    case 'S':
        if (strcmp("functions", token) == 0)
        {
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                size_t count;

                puts("Simple functions:");
                tms_rc_func *all_rcfunc = tms_get_all_rc_func(&count, true);
                for (size_t i = 0; i < count; ++i)
                    print_array_of_chars_helper(all_rcfunc[i].name);
                print_array_of_chars_helper(NULL);
                free(all_rcfunc);

                puts("Extended functions:");
                tms_extf *all_extf = tms_get_all_extf(&count, true);
                for (size_t i = 0; i < count; ++i)
                    print_array_of_chars_helper(all_extf[i].name);
                print_array_of_chars_helper(NULL);
                free(all_extf);

                puts("User defined functions:");
                tms_ufunc *all_ufunc = tms_get_all_ufunc(&count, true);
                if (all_ufunc == NULL)
                    puts("<None defined>");
                else
                {
                    char *argstring;
                    for (size_t i = 0; i < count; ++i)
                    {
                        argstring = tms_args_to_string(all_ufunc[i].F->labels);
                        printf("%s(%s) = %s\n", all_ufunc[i].name, argstring, all_ufunc[i].F->expr);
                        free(argstring);
                    }
                }
                putchar('\n');
                free(all_ufunc);
                return NEXT_ITERATION;
            }
        }
        else if (strcmp("variables", token) == 0)
        {
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                puts("List of defined variables:");
                printf("ans = ");
                tms_print_value(tms_g_ans);
                putchar('\n');
                // Retrieve all variables into an array using library call
                size_t count;
                tms_var *var_list = tms_get_all_vars(&count, true);
                if (var_list != NULL)
                {
                    for (size_t i = 0; i < count; ++i)
                    {
                        printf("%s = ", var_list[i].name);
                        tms_print_value(var_list[i].value);
                        if (var_list[i].is_constant)
                            puts(" (read-only)");
                        else
                            putchar('\n');
                    }
                }
                putchar('\n');
                free(var_list);
                return NEXT_ITERATION;
            }
        }
        else if (strcmp("debug", token) == 0)
        {
            _tms_debug = true;
            puts("Debug output enabled.\n");
            return NEXT_ITERATION;
        }
        else if (strcmp("undebug", token) == 0)
        {
            _tms_debug = false;
            puts("Debug output disabled.\n");
            return NEXT_ITERATION;
        }
        else if (strcmp("del", token) == 0)
        {
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                puts("Usage: del var1|func1 [var2|func2 ...]");
                putchar('\n');
                return NEXT_ITERATION;
            }
            const tms_var *target_var;
            const tms_ufunc *target_ufunc;
            do
            {
                // Lookup if the provided name is a var or a function
                target_var = tms_get_var_by_name(token);
                target_ufunc = tms_get_ufunc_by_name(token);
                if (target_var == NULL && target_ufunc == NULL)
                    printf("No variable or user function named \"%s\"\n", token);
                if (target_var != NULL)
                {
                    int status = tms_remove_var(token);
                    switch (status)
                    {
                    case 0:
                        printf("Variable \"%s\" removed\n", token);
                        break;
                    // This case should never happen, put here for completeness
                    case -1:
                        printf("Variable \"%s\" not found\n", token);
                        break;
                    case 1:
                        printf("Variable \"%s\" is read-only, it can't be removed\n", token);
                    }
                }
                else if (target_ufunc != NULL)
                {
                    int status = tms_remove_ufunc(token);
                    switch (status)
                    {
                    case 0:
                        printf("Function \"%s\" removed\n", token);
                        break;
                    // This case should never happen, put here for completeness
                    case -1:
                        printf("Function \"%s\" not found\n", token);
                        break;
                    }
                }
                token = strtok(NULL, " ");
            } while (token != NULL);
            putchar('\n');
            return NEXT_ITERATION;
        }
        else if (strcmp("reset", token) == 0)
        {
            tmsolve_reset();
            puts("Calculator reset complete\n");
            return NEXT_ITERATION;
        }
        break;
    case 'I':
        // Detect word size change request
        if (strcmp("set", token) == 0)
        {
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                printf("Current word size: %d bits\n", tms_int_mask_size);
                puts("Use the \"set\" keyword with w1, w2, w4, w8 to set the word size.\n"
                     "Example: set w8\n");
            }
            else
            {
                int size;
                if (strcmp("w1", token) == 0)
                    size = 8;
                else if (strcmp("w2", token) == 0)
                    size = 16;
                else if (strcmp("w4", token) == 0)
                    size = 32;
                else if (strcmp("w8", token) == 0)
                    size = 64;
                else
                {
                    fputs("Unrecognized option, try w1, w2, w4, w8\n\n", stderr);
                    return NEXT_ITERATION;
                }

                tms_set_int_mask(size);
                printf("Word size set to %d bits.\n\n", tms_int_mask_size);
            }
            return NEXT_ITERATION;
        }
        else if (strcmp("del", token) == 0)
        {
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                puts("Usage: del var1|func1 [var2|func2 ...]");
                putchar('\n');
                return NEXT_ITERATION;
            }
            const tms_int_var *target_int_var;
            const tms_int_ufunc *target_int_ufunc;
            do
            {
                // Lookup if the provided name is a var or a function
                target_int_var = tms_get_int_var_by_name(token);
                target_int_ufunc = tms_get_int_ufunc_by_name(token);
                if (target_int_var == NULL && target_int_ufunc == NULL)
                    printf("No variable or user function named \"%s\"\n", token);
                if (target_int_var != NULL)
                {
                    int status = tms_remove_int_var(token);
                    switch (status)
                    {
                    case 0:
                        printf("Variable \"%s\" removed\n", token);
                        break;
                    // This case should never happen, put here for completeness
                    case -1:
                        printf("Variable \"%s\" not found\n", token);
                        break;
                    case 1:
                        printf("Variable \"%s\" is read-only, it can't be removed\n", token);
                    }
                }
                else if (target_int_ufunc != NULL)
                {
                    int status = tms_remove_int_ufunc(token);
                    switch (status)
                    {
                    case 0:
                        printf("Function \"%s\" removed\n", token);
                        break;
                    // This case should never happen, put here for completeness
                    case -1:
                        printf("Function \"%s\" not found\n", token);
                        break;
                    }
                }
                token = strtok(NULL, " ");
            } while (token != NULL);
            putchar('\n');
            return NEXT_ITERATION;
        }
        else if (strcmp("functions", token) == 0)
        {
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                size_t count;

                puts("Simple functions:");
                tms_int_func *all_int_func = tms_get_all_int_func(&count, true);
                for (size_t i = 0; i < count; ++i)
                    print_array_of_chars_helper(all_int_func[i].name);
                print_array_of_chars_helper(NULL);
                free(all_int_func);

                puts("Extended functions:");
                tms_int_extf *all_int_extf = tms_get_all_int_extf(&count, true);
                for (size_t i = 0; i < count; ++i)
                    print_array_of_chars_helper(all_int_extf[i].name);
                print_array_of_chars_helper(NULL);
                free(all_int_extf);

                puts("User defined functions:");
                tms_int_ufunc *all_int_ufunc = tms_get_all_int_ufunc(&count, true);
                if (all_int_ufunc == NULL)
                    puts("<None defined>");
                else
                {
                    char *argstring;
                    for (size_t i = 0; i < count; ++i)
                    {
                        argstring = tms_args_to_string(all_int_ufunc[i].F->labels);
                        printf("%s(%s) = %s\n", all_int_ufunc[i].name, argstring, all_int_ufunc[i].F->expr);
                        free(argstring);
                    }
                }
                putchar('\n');
                free(all_int_ufunc);
                return NEXT_ITERATION;
            }
        }
        else if (strcmp("variables", token) == 0)
        {
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                puts("List of defined variables:");
                printf("ans = ");
                tms_print_hex(tms_g_int_ans);
                putchar('\n');
                // Retrieve all variables into an array using library call
                size_t count;
                tms_int_var *int_var_list = tms_get_all_int_vars(&count, true);
                if (int_var_list != NULL)
                    for (size_t i = 0; i < count; ++i)
                    {
                        printf("%s = ", int_var_list[i].name);
                        tms_print_hex(int_var_list[i].value);
                        if (int_var_list[i].is_constant)
                            puts(" (read-only)");
                        else
                            putchar('\n');
                    }
                putchar('\n');
                free(int_var_list);
                return NEXT_ITERATION;
            }
        }
        else if (strcmp("debug", token) == 0)
        {
            _tms_debug = true;
            puts("Debug output enabled.\n");
            return NEXT_ITERATION;
        }
        else if (strcmp("undebug", token) == 0)
        {
            _tms_debug = false;
            puts("Debug output disabled.\n");
            return NEXT_ITERATION;
        }
        else if (strcmp("reset", token) == 0)
        {
            tmsolve_reset();
            puts("Calculator reset complete\n");
            return NEXT_ITERATION;
        }
    default:
        break;
    }

    return NO_ACTION;
}

// This stub exists to avoid calling free() at every possible return of this function
int management_input(char *input)
{
    input = strdup(input);
    int decision = _management_input_lazy(input);
    free(input);
    return decision;
}

// Function that keeps running until a valid input is obtained, returning the result
double get_value(char *prompt)
{
    double value = 0;
    char *expr;
    while (1)
    {
        expr = get_input(NULL, prompt, -1);

        value = tms_solve_e(expr, 0, NULL);
        free(expr);
        return value;
    }
}
bool valid_mode(char mode)
{
    char all_modes[] = {"SIFEUG"};
    int i, length = strlen(all_modes);
    for (i = 0; i < length; ++i)
        if (mode == all_modes[i])
            return true;
    return false;
}
void scientific_mode()
{
    char *expr, *shifted_expr, *name = NULL;
    double complex result;
    int i;
    puts("Current mode: Scientific");
    while (1)
    {
        shifted_expr = expr = get_input(NULL, "> ", -1);

        switch (management_input(expr))
        {
        case SWITCH_MODE:
            free(expr);
            return;

        case NEXT_ITERATION:
            free(expr);
            continue;
        }

        // Search for assignment operator to handle user functions or variables
        i = tms_f_search(expr, "=", 0, false);
        if (i != -1)
        {
            // Set user function (has a name and parenthesis)
            if (i > 3 && expr[i - 1] == ')')
            {
                int name_len = tms_f_search(expr, "(", 0, false);
                if (name_len == -1)
                {
                    free(expr);
                    return;
                }
                name = tms_strndup(expr, name_len);
                char *argument_names = tms_strndup(expr + name_len + 1, i - name_len - 2);
                if (tms_set_ufunction(name, argument_names, expr + i + 1) == 0)
                    printf("Function set successfully.\n\n");
                else
                    tms_print_errors(TMS_PARSER);
                free(name);
                free(argument_names);
                free(expr);
                name = NULL;
                continue;
            }
            else
            {
                // Shift the expression beyond the '=' in case of var assignment since the parser doesn't support it
                name = tms_strndup(expr, i);
                shifted_expr += i + 1;
            }
        }
        // A normal expression to calculate
        result = tms_solve(shifted_expr);
        tms_set_ans(result);

        if (!isnan(creal(result)))
        {
            print_result(result, true);
            if (name != NULL)
                tms_set_var(name, result, false);
        }

        if (name != NULL)
        {
            free(name);
            name = NULL;
        }
        free(expr);
    }
}

void print_int_value_multibase(int64_t value)
{
    if (value == 0)
        puts("= 0\n");
    else
    {
        printf("= ");
        switch (tms_int_mask_size)
        {
        case 8:
            printf("%" PRId8, (int8_t)value);
            break;
        case 16:
            printf("%" PRId16, (int16_t)value);
            break;
        case 32:
            printf("%" PRId32, (int32_t)value);
            break;
        case 64:
            printf("%" PRId64, value);
            break;
        default:
            fputs("Unexpected mask size.\n\n", stderr);
            return;
        }
        printf("\n= ");
        tms_print_hex(value);
        printf(" = ");
        tms_print_oct(value);
        printf("\n= ");
        tms_print_bin(value);
        if (tms_int_mask_size > 15)
        {
            printf("\n= ");
            tms_print_dot_decimal(value);
        }
        printf("\n\n");
    }
}

void integer_mode()
{
    char *name = NULL, *expr, *shifted_expr;
    int i;
    int64_t result;
    puts("Current mode: Integer");
    while (1)
    {
        expr = get_input(NULL, "> ", -1);
        shifted_expr = expr;

        switch (management_input(expr))
        {
        case SWITCH_MODE:
            free(expr);
            return;

        case NEXT_ITERATION:
            free(expr);
            continue;
        }
        // Search for assignment operator to handle user functions or variables
        i = tms_f_search(expr, "=", 0, false);
        if (i != -1)
        {
            // Set user function (has a name and parenthesis)
            if (i > 3 && expr[i - 1] == ')')
            {
                int name_len = tms_f_search(expr, "(", 0, false);
                if (name_len == -1)
                {
                    free(expr);
                    return;
                }
                name = tms_strndup(expr, name_len);
                char *label_names = tms_strndup(expr + name_len + 1, i - name_len - 2);
                if (tms_set_int_ufunction(name, label_names, expr + i + 1) == 0)
                    printf("Function set successfully.\n\n");
                else
                    tms_print_errors(TMS_INT_PARSER);
                free(name);
                free(label_names);
                free(expr);
                name = NULL;
                continue;
            }
            else
            {
                // Shift the expression beyond the '=' in case of var assignment since the parser doesn't support it
                name = tms_strndup(expr, i);
                shifted_expr += i + 1;
            }
        }
        // Not a function
        if (tms_int_solve(shifted_expr, &result) != -1)
        {
            print_int_value_multibase(result);
            tms_g_int_ans = tms_sign_extend(result);
            if (name != NULL)
                tms_set_int_var(name, result, false);
        }

        if (name != NULL)
        {
            free(name);
            name = NULL;
        }

        free(expr);
    }
}

void print_result(double complex result, bool verbose)
{
    double real = creal(result), imag = cimag(result);
    if (isnan(real) || isnan(imag))
        return;

    if (verbose)
        printf("= ");

    if (imag != 0)
    {
        if (real != 0)
        {
            printf("%.10g", real);
            if (imag > 0)
                printf("+");
        }

        if (imag == 1)
            printf("i");

        else if (imag == -1)
            printf("-i");
        else
            printf("%.10g i", imag);
        if (verbose)
            printf("\nMod = %.10g, arg = %.10g rad = %.10g deg", cabs(result), carg(result), carg(result) * 180 / M_PI);
    }
    else
    {
        printf("%.10g", real);
        if (verbose)
        {
            tms_fraction fraction_str = tms_decimal_to_fraction(real, false);
            if (fraction_str.c != 0)
            {
                if (fraction_str.a != 0)
                    printf("\n= %d + %d / %d", fraction_str.a, fraction_str.b, fraction_str.c);
                printf("\n= %" PRId64 " / %d", ((int64_t)fraction_str.a * fraction_str.c + fraction_str.b), fraction_str.c);
            }
        }
    }
    printf("\n");
    // If verbose it set (interactive), add an extra newline for visibility
    if (verbose)
        printf("\n");
}

void equation_mode()
{
    int degree, status;
    char operation[7];
    puts("Current mode: Equation");

    while (1)
    {
        puts("Degree? (n<=3)");
        get_input(operation, "> ", 6);

        switch (management_input(operation))
        {
        case SWITCH_MODE:
            return;

        case NEXT_ITERATION:
            continue;
        }

        status = sscanf(operation, "%d", &degree);

        // For mode switching
        if (operation[1] == '\0' && status == 0)
        {
            if (valid_mode(operation[0]))
            {
                _mode = operation[0];
                return;
            }
        }

        equation_solver(degree);
    }
}

void function_calculator()
{
    double start, end, step, x;
    int i;
    char *expr, step_op, *function, *old_function = NULL;
    tms_math_expr *M;
    // We have one variable here: x
    tms_arg_list *the_x = tms_get_args("x");
    puts("Current mode: Function");
    while (1)
    {
        function = get_input(NULL, "f(x) = ", -1);

        switch (management_input(function))
        {
        case SWITCH_MODE:
            free(function);
            free(the_x);
            return;

        case NEXT_ITERATION:
            free(function);
            continue;
        }

        if (strcmp(function, "prev") == 0)
        {
            free(function);
            if (old_function != NULL)
                function = strdup(old_function);
            else
            {
                fputs("No previous function found.\n\n", stderr);
                continue;
            }
            printf("f(x) = %s\n", function);
        }

        M = tms_parse_expr(function, ENABLE_CMPLX, the_x);

        if (M == NULL)
        {
            tms_delete_math_expr(M);
            free(function);
            continue;
        }

        free(old_function);
        old_function = strdup(function);

        start = get_value("Start: ");
        printf("%.12g\n", start);
        // Read end value
        while (1)
        {
            end = get_value("End: ");
            if (start > end)
            {
                puts("Error: Start must be smaller than end.");
                continue;
            }
            else
            {
                printf("%.12g\n", end);
                break;
            }
        }
        // Read step and identify stepping operator
        while (1)
        {
            expr = get_input(NULL, "Step: ", -1);
            i = strlen(expr);
            if (i == 0)
                continue;
            if (expr[i - 1] == '+' || expr[i - 1] == '*' || expr[i - 1] == '^')
            {
                step_op = expr[i - 1];
                expr[i - 1] = '\0';
            }
            else
                step_op = '+';

            step = tms_solve_e(expr, 0, NULL);
            if (isnan(step))
            {
                printf("\n");
                free(expr);
                continue;
            }

            if (step_op == '*' && step <= 1)
            {
                puts("Step for multiplication should be greater than 1");
                free(expr);
                continue;
            }

            if (step + start == start)
            {
                puts("Error, the step is too small relative to start.");
                free(expr);
                continue;
            }

            else
            {
                puts(expr);
                free(expr);
                break;
            }

            if (step == 0)
            {
                puts("Error, step cannot be 0");
                free(expr);
                continue;
            }
        }
        x = start;
        double prev_x;
        double complex result, tmp;
        while (x <= end)
        {
            prev_x = x;
            // Set the value of x in the subexpr_ptr
            tmp = x;
            tms_set_labels_values(M, &tmp);
            // Solving the function then printing
            result = tms_evaluate(M, NO_LOCK);
            if (isnan(creal(result)))
            {
                tms_clear_errors(TMS_EVALUATOR);
                printf("f(%g)=Error", x);
            }
            else
            {
                printf("f(%g) = ", x);
                tms_print_value(result);
            }
            putchar('\n');
            // Calculating the next value of x according to the specified method
            switch (step_op)
            {
            case '+':
                x += step;
                break;
            case '*':
                x *= step;
                break;
            case '^':
                x = pow(x, step);
                break;
            }
            if (prev_x >= creal(x))
            {
                puts("Error, the step used makes it impossible to reach the end.");
                break;
            }
        }
        printf("\n");
        free(function);
        tms_delete_math_expr(M);
    }
}

void utility_mode()
{
    char input[24];
    int p;
    puts("Current mode: Utility");
    while (1)
    {
        get_input(input, "> ", 23);

        switch (management_input(input))
        {
        case SWITCH_MODE:
            return;

        case NEXT_ITERATION:
            continue;
        }

        p = tms_f_search(input, "(", 0, false);
        if (p > 0)
        {
            if (strncmp("factor(", input, p) == 0)
            {
                double tmp;
                int32_t value;
                int status;

                tms_int_factor *factors_list;
                status = sscanf(input + p + 1, "%lf", &tmp);

                if (status < 1)
                {
                    fprintf(stderr, "Syntax error.\n\n");
                    continue;
                }

                if (fabs(tmp) > INT32_MAX)
                {
                    fprintf(stderr, "Values must be in range [-2147483647;2147483647]\n\n");
                    continue;
                }
                else
                    value = tmp;

                factors_list = tms_find_factors(value);

                if (factors_list->factor == 0)
                    puts("= 0");

                // Print the factors in a clear format
                else
                {
                    printf("%" PRId32 " = 1", value);
                    for (int i = 1; factors_list[i].factor != 0; ++i)
                    {
                        // Print in format factor1 ^ power1 * factor2 ^ power2
                        printf(" * %" PRId32, factors_list[i].factor);
                        if (factors_list[i].power > 1)
                            printf(" ^ %" PRId32, factors_list[i].power);
                    }
                }
                free(factors_list);
                printf("\n\n");
            }
            else
                fprintf(stderr, "Invalid function. Supported: factor(int)\n\n");
        }
        else
            fprintf(stderr, "Invalid input. Supported: factor(int)\n\n");
    }
}

/*
Who said a calculator can't have some secrets?
Legacy game (rock paper scissors), replaced by the fancy tic-tac-toe
void rps()
{
    char *operation = (char *)malloc(10 * sizeof(char)), playerc;
    // playerc is the character entered by the player
    int wins = 0, total = 0, playerv, rnv;
    puts("You found the secret mode!");
    puts("Rock Paper Scissor, enter r for rock, p for paper, s for scissors.");
    while (1)
    {
        get_input(&operation, NULL, 10);
        if (strncmp(operation, "exit", 4) == 0)
            return;
        playerc = operation[0];
        if (strncmp(operation, "stat", 4) == 0)
        {
            printf("Wins=%d\nTotal=%d\n", wins, total);
            continue;
        }
        ++total;
        srand(time(NULL) * 6581);
        rnv = rand() % 3;
        if (playerc == 'r')
            playerv = 0;
        else if (playerc == 'p')
            playerv = 1;
        else if (playerc == 's')
            playerv = 2;
        else
        {
            puts("Invalid input");
            playerv = 3;
        }
        if (playerv == rnv)
        {
            switch (playerc)
            {
            case 'r':
                puts("Rock - Rock : Draw");
                break;
            case 's':
                puts("Scissor - Scissor : Draw");
                break;
            case 'p':
                puts("Paper - Paper: Draw");
                break;
            }
        }
        else
        {
            if (rnv == 0)
            {
                switch (playerv)
                {
                case 1:
                    puts("Paper - Rock : Win!");
                    ++wins;
                    break;
                case 2:
                    puts("Scissor - Rock: Lose :(");
                    break;
                }
            }
            else if (rnv == 1)
            {
                switch (playerv)
                {
                case 0:
                    puts("Rock - Paper : Lose :(");
                    break;
                case 2:
                    puts("Scissors - Paper : Win!");
                    ++wins;
                    break;
                }
            }
            else if (rnv == 2)
            {
                switch (playerv)
                {
                case 0:
                    puts("Rock - Scissor : Win!");
                    ++wins;
                    break;
                case 1:
                    puts("Paper - Scissor : Lose :(");
                    break;
                }
            }
        }
    }
}
*/