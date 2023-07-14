/*
Copyright (C) 2021-2023 Ahmad Ismail
SPDX-License-Identifier: GPL-3.0-or-later
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interactive.h"

char _mode = 'S';
int main(int argc, char **argv)
{
    // Calculate expressions sent as command line arguments.
    if (argc > 1)
    {
        if (strcmp(argv[1], "--test") == 0)
        {
            // Load the test file, should have the following format:
            // expression1;expected_answer1
            // expression2;expected_answer2
            // ...
            FILE *test_file;
            test_file = fopen(argv[2], "r");
            if (test_file == NULL)
            {
                fputs("Unable to open test file.\n", stderr);
                exit(1);
            }
            char buffer[1000];
            int separator;
            while (feof(test_file) == 0)
            {
                fgets(buffer, 1000, test_file);
                int length = strlen(buffer);
                if (buffer[length - 1] == '\n')
                    buffer[length - 1] = '\0';
                else if (!feof(test_file))
                {
                    puts("Buffer size possibly exceeded.");
                    exit(2);
                }
                tms_remove_whitespace(buffer);
                separator = tms_f_search(buffer, ";", 0, false);
                if (separator == -1)
                {
                    fputs("Incorrect format for test file, missing ;", stderr);
                    continue;
                }

                char expr[separator];
                double complex expected_ans;
                // Not the most efficient way, but the calculator should be reliable for simple ops.
                // I could have used sscanf or tms_read_value, but solving it will handle complex easier.
                expected_ans = tms_solve(buffer + separator + 1);

                strncpy(expr, buffer, separator);
                expr[separator] = '\0';
                puts(expr);
                // Automatically sets ans
                tms_solve(expr);
                // Calculate relative error
                if (expected_ans == 0)
                {
                    if (fabs(creal(expected_ans)) < 1e-15 && fabs(cimag(expected_ans)) < 1e-15)
                        puts("Passed");
                    else
                        puts("Expected answer is zero but actual answer is non negligible.");
                }
                else
                {
                    double relative_error = cabs((expected_ans - tms_g_ans) / expected_ans);
                    printf("relative error=%g\n", relative_error);
                    if (relative_error > 1e-3)
                    {
                        fputs("Relative error is too high.", stderr);
                        exit(1);
                    }
                    else
                        puts("Passed");
                }
            }
        }

        // Calculate the expressions passed as arguments
        else
        {
            int i, length = 0;
            for (i = 1; i < argc; ++i)
                length += strlen(argv[i]);
            char *all = malloc((length + 1) * sizeof(char));
            // Initialize the "all" array using the first argument (strcat expects an initialized string)
            strcpy(all, argv[1]);

            // Concatenate arguments
            for (i = 2; i < argc; ++i)
                strcat(all, argv[i]);
            // This function will set ans if the calculation succeeds.
            // Force set ans to nan if the calculation fails.
            tms_g_ans = tms_solve(all);

            if (isnan(creal(tms_g_ans)))
            {
                tms_error_handler(NULL, EH_PRINT);
                exit(1);
            }
            else
                print_result(tms_g_ans, false);
        }
        exit(0);
    }

// For readline autocompletion
#ifdef USE_READLINE
    rl_attempted_completion_function = character_name_completion;
#endif

    // pick the mode
    while (1)
    {
        switch (_mode)
        {
        case 'S':
            scientific_mode();
            break;
        case 'F':
            function_calculator();
            break;
        case 'E':
            equation_mode();
            break;
        case 'M':
            matrix_mode();
            break;
        case 'U':
            utility_mode();
            break;
        case 'G':
            tic_tac_toe();
            break;

        default:
            exit(1);
        }
        printf("\n");
    }
}
