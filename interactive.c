/*
Copyright (C) 2022-2023 Ahmad Ismail
SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "interactive.h"

#ifdef USE_READLINE
char *character_name_generator(const char *text, int state)
{
    static int index1, index2, len;
    char *name;

    if (!state)
    {
        index1 = 0;
        index2 = 0;
        len = strlen(text);
    }
    switch (_autocomplete_mode)
    {
    case 'S':
        while (index1 < tms_g_func_count)
        {
            name = tms_g_all_func_names[index1++];
            if (strncmp(name, text, len) == 0)
            {
                char *dup_with_parenthesis = malloc((strlen(name) + 2) * sizeof(char));
                strcpy(dup_with_parenthesis, name);
                strcat(dup_with_parenthesis, "(");
                return dup_with_parenthesis;
            }
        }
        while (index2 < tms_g_var_count)
        {
            name = tms_g_vars[index2++].name;
            if (strncmp(name, text, len) == 0)
                return strdup(name);
        }
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
// Reads n characters to *buffer (if n = -1 , no character limit), print prompt
// Your buffer should have size of n+1 to accomodate the NULL terminator.
// Do not use a size of -1 with stack allocated strings
void get_input(char **buffer, char *prompt, size_t n)
{
    char *tmp;
    while (1)
    {
        tmp = readline(prompt);
        // Properly handle end of piped input
        if (tmp == NULL)
            exit(0);

        if (strlen(tmp) == 0)
        {
            puts(NO_INPUT "\n");
            free(tmp);
            continue;
        }
        if (n == -1)
        {
            *buffer = tmp;
            break;
        }
        else if (strlen(tmp) > n)
        {
            printf("Expression is very long, it must be at most %zu characters.\n\n", n);
            free(tmp);
        }
        else
        {
            strncpy(*buffer, tmp, n);
            (*buffer)[n] = '\0';
            free(tmp);
            break;
        }
    }
#ifdef USE_READLINE
    add_history(*buffer);
#endif
}
// Simple function to flush stdin
void flush_stdin()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}
// Function that keeps running until a valid input is obtained, returning the result
double get_value(char *prompt)
{
    double value = 0;
    char *expr;
    while (1)
    {
        get_input(&expr, prompt, -1);

        value = tms_solve_e(expr, false);
        free(expr);
        if (isnan(value))
            tms_error_handler(EH_PRINT);
        else
            return value;
    }
}
bool valid_mode(char mode)
{
    char all_modes[] = {"SFEUG"};
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
    puts("Current mode: Scientific");
    while (1)
    {
        get_input(&expr, "> ", -1);
        if (strcmp(expr, "exit") == 0)
            exit(0);

        // If the string has length 1, check for mode switching.
        if (expr[1] == '\0')
        {
            if (valid_mode(expr[0]))
            {
                _mode = expr[0];
                free(expr);
                return;
            }
        }

        result = tms_solve(expr);
        tms_set_ans(result);

        if (isnan(creal(result)))
            tms_error_handler(EH_PRINT);
        else
            print_result(result, true);

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
                printf("\n= %d / %d", (fraction_str.a * fraction_str.c + fraction_str.b), fraction_str.c);
            }
        }
    }
    printf("\n");
    // If verbose it set (interactive), add an extra space for visibility
    if (verbose)
        printf("\n");
}

void equation_mode()
{
    int degree, status;
    char *operation = malloc(5 * sizeof(char));
    puts("Current mode: Equation");

    while (1)
    {
        puts("Degree? (n<=3)");
        get_input(&operation, NULL, 4);
        status = sscanf(operation, "%d", &degree);

        // For mode switching
        if (operation[1] == '\0' && status == 0)
        {
            if (valid_mode(operation[0]))
            {
                _mode = operation[0];
                free(operation);
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
    char *expr, step_op, *function;
    tms_math_expr *M;
    puts("Current mode: Function");
    while (1)
    {
        get_input(&function, "f(x) = ", -1);
        if (strcmp(function, "exit") == 0)
            exit(0);
        // If the string has length 1, check for mode switching.
        if (function[1] == '\0')
        {
            if (valid_mode(function[0]))
            {
                _mode = function[0];
                free(function);
                return;
            }
        }

        if (tms_syntax_check(function) == false)
        {
            tms_error_handler(EH_PRINT);
            continue;
        }
        M = tms_parse_expr(function, true, false);

        if (M == NULL)
        {
            tms_error_handler(EH_PRINT);
            free(function);
            continue;
        }
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
            get_input(&expr, "Step: ", -1);
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
                continue;
            }
        }
        x = start;
        double prevx, result;
        while (x <= end)
        {
            prevx = x;
            // Set the value of x in the subexpr_ptr
            tms_set_unknown(M, x);
            // Solving the function then printing
            result = tms_evaluate(M);
            if (isnan(result))
            {
                tms_error_handler(EH_CLEAR, EH_MAIN_DB);
                printf("f(%g)=Error\n", x);
            }
            else
                printf("f(%g) = %.10g\n", x, result);
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
            if (prevx >= x)
            {
                puts("Error, the step used makes it impossible to reach the end.");
                break;
            }
        }
        printf("\n");
        free(function);
    }
}

void utility_mode()
{
    char *input = malloc(24 * sizeof(char));
    int p;
    puts("Current mode: Utility");
    while (1)
    {
        get_input(&input, "> ", 23);
        if (strcmp(input, "exit") == 0)
            exit(0);

        // Mode switcher
        if (input[1] == '\0')
        {
            if (valid_mode(input[0]))
            {
                _mode = input[0];
                free(input);
                return;
            }
        }

        p = tms_f_search(input, "(", 0, false);
        if (p > 0)
        {
            if (strncmp("factor", input, p - 1) == 0)
            {
                int32_t value;
                tms_int_factor *factors_list;
                sscanf(input + p + 1, "%" PRId32, &value);
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
        }
        tms_error_handler(EH_PRINT);
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