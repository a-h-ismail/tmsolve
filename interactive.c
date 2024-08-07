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
    static int index1, index2, len;
    static char *to_complete = NULL, *prefix = NULL;
    char *name;

    if (!state)
    {
        index1 = 0;
        index2 = 0;
        // The previous partial text
        free(to_complete);
        to_complete = tms_get_name((char *)text, strlen(text) - 1, false);

        if (to_complete == NULL)
            to_complete = strdup("");

        len = strlen(to_complete);

        free(prefix);
        prefix = tms_strndup(text, strlen(text) - len);

        if (strlen(prefix) > 0)
        {
            char last_in_prefix = prefix[strlen(prefix) - 1];

            // If tms_get_name didn't find a valid name, but the previous char is valid for names
            // Then this is a syntax error, don't autocomplete
            if (tms_legal_char_in_name(last_in_prefix) && to_complete[0] == '\0')
                return NULL;
        }
    }
    switch (_autocomplete_mode)
    {
    case 'S':
        while (index1 < tms_g_all_func_count)
        {
            name = tms_g_all_func_names[index1++];
            if (strncmp(name, to_complete, len) == 0)
            {
                char *dup_with_parenthesis = malloc((strlen(name) + 2) * sizeof(char));
                strcpy(dup_with_parenthesis, name);
                strcat(dup_with_parenthesis, "(");
                char *tmp = tms_strcat_dup(prefix, dup_with_parenthesis);
                free(dup_with_parenthesis);
                return tmp;
            }
        }
        while (index2 < tms_g_var_count)
        {
            name = tms_g_vars[index2++].name;
            if (strncmp(name, to_complete, len) == 0)
                return tms_strcat_dup(prefix, name);
        }
        break;

    case 'I':
        while (index1 < tms_g_all_int_func_count)
        {
            name = tms_g_all_int_func_names[index1++];
            if (strncmp(name, text, len) == 0)
            {
                char *dup_with_parenthesis = malloc((strlen(name) + 2) * sizeof(char));
                strcpy(dup_with_parenthesis, name);
                strcat(dup_with_parenthesis, "(");
                char *tmp = tms_strcat_dup(prefix, dup_with_parenthesis);
                free(dup_with_parenthesis);
                return tmp;
            }
        }
        while (index2 < tms_g_int_var_count)
        {
            name = tms_g_int_vars[index2++].name;
            if (strncmp(name, text, len) == 0)
                return tms_strcat_dup(prefix, name);
        }
        break;
    }
    return NULL;
}

char **character_name_completion(const char *text, int start, int end)
{
    // Disable normal filename autocomplete
    rl_attempted_completion_over = 1;

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

void print_array_of_chars(char **c, int length)
{
    if (length < 1)
        return;

    for (int i = 0; i < length; ++i)
    {
        printf(" %s", c[i]);

        if (i != length - 1)
            putchar(',');

        if (i % 3 == 2 && i != length - 1)
            putchar('\n');
    }
    puts("\n");
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
                puts("List of defined functions:");
                print_array_of_chars(tms_g_all_func_names, tms_g_all_func_count);
                return NEXT_ITERATION;
            }
        }
        else if (strcmp("variables", token) == 0)
        {
            token = strtok(NULL, " ");
            if (token == NULL)
            {

                puts("List of defined variables:");
                printf("\"ans\" = ");
                tms_print_value(tms_g_ans);
                putchar('\n');
                for (int i = 0; i < tms_g_var_count; ++i)
                {
                    printf("\"%s\" = ", tms_g_vars[i].name);
                    tms_print_value(tms_g_vars[i].value);
                    if (tms_g_vars[i].is_constant)
                        printf(" (read-only)");
                    putchar('\n');
                }

                putchar('\n');
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

                // To easily calculate the mask, set all bits to 1, then shift right by 64-mask_size
                tms_set_int_mask(size);
                printf("Word size set to %d bits.\n\n", tms_int_mask_size);
            }
            return NEXT_ITERATION;
        }
        else if (strcmp("functions", token) == 0)
        {
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                puts("List of defined functions:");
                print_array_of_chars(tms_g_all_int_func_names, tms_g_all_int_func_count);
                return NEXT_ITERATION;
            }
        }
        else if (strcmp("variables", token) == 0)
        {
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                puts("List of defined variables:");
                printf("\"ans\" = ");
                tms_print_hex(tms_g_int_ans);
                putchar('\n');
                for (int i = 0; i < tms_g_int_var_count; ++i)
                {
                    printf("\"%s\" = ", tms_g_int_vars[i].name);
                    tms_print_hex(tms_g_int_vars[i].value);
                    putchar('\n');
                }

                putchar('\n');
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

        value = tms_solve_e(expr, false);
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
    char *expr;
    double complex result;
    int i;
    puts("Current mode: Scientific");
    while (1)
    {
        expr = get_input(NULL, "> ", -1);

        switch (management_input(expr))
        {
        case SWITCH_MODE:
            free(expr);
            return;

        case NEXT_ITERATION:
            free(expr);
            continue;
        }

        // Search for assignment operator to handle runtime functions or variables
        i = tms_f_search(expr, "=", 0, false);
        if (i != -1)
        {
            if (i > 3 && expr[i - 1] == ')')
            {
                int name_len = tms_f_search(expr, "(", 0, false);
                if (name_len == -1)
                    return;
                char *name = tms_strndup(expr, name_len), *unknowns_list = tms_strndup(expr + name_len + 1, i - name_len - 2);
                if (tms_set_ufunction(name, unknowns_list, expr + i + 1) == 0)
                    printf("Function set successfully.\n\n");
                else
                    tms_print_errors(TMS_PARSER);
            }
        }
        else
        {
            // A normal expression to calculate
            result = tms_solve(expr);
            tms_set_ans(result);

            if (!isnan(creal(result)))
                print_result(result, true);
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
        if (tms_int_mask_size > 15)
        {
            printf(" = ");
            tms_print_dot_decimal(value);
        }
        printf("\n= ");
        tms_print_hex(value);
        printf(" = ");
        tms_print_oct(value);
        printf("\n= ");
        tms_print_bin(value);
        printf("\n\n");
    }
}

void integer_mode()
{
    char *expr;
    int64_t result;
    puts("Current mode: Integer");
    while (1)
    {
        expr = get_input(NULL, "> ", -1);

        switch (management_input(expr))
        {
        case SWITCH_MODE:
            free(expr);
            return;

        case NEXT_ITERATION:
            free(expr);
            continue;
        }

        if (tms_int_solve(expr, &result) != -1)
        {
            print_int_value_multibase(result);
            tms_g_int_ans = tms_sign_extend(result);
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

        M = tms_parse_expr(function, TMS_ENABLE_CMPLX | TMS_ENABLE_UNK, the_x);

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

            step = tms_solve_e(expr, false);
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
            tms_set_unknowns(M, &tmp);
            // Solving the function then printing
            result = _tms_evaluate_unsafe(M);
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