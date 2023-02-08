/*
Copyright (C) 2021-2023 Ahmad Ismail
SPDX-License-Identifier: GPL-3.0-or-later
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interactive.h"
double complex ans = 0;
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
            char buffer[500];
            int separator;
            while (feof(test_file) == 0)
            {
                fgets(buffer, 500, test_file);
                separator = f_search(buffer, ";", 0,false);
                if (separator == -1)
                {
                    fputs("Incorrect format for test file, missing ;", stderr);
                    continue;
                }

                char expr[separator];
                double expected_ans;
                sscanf(buffer+separator+1, "%lf", &expected_ans);
                strncpy(expr,buffer,separator);
                expr[separator]='\0';
                puts(expr);
                scientific_complex_picker(expr);
                // Calculate relative error
                double relative_error = fabs((expected_ans - ans) / expected_ans);
                printf("Expected=%g, result=%g, relative error=%g",expected_ans,ans,relative_error);
                if (relative_error > 1e-3)
                {
                    fputs("Relative error is too high.", stderr);
                    exit(1);
                }
                else
                    puts("Passed");
            }
        }
        else
            for (int i = 1; i < argc; ++i)
            {
                ans = calculate_expr(argv[i], false);
                if (isnan(creal(ans)))
                {
                    error_handler(NULL, 2);
                    continue;
                }
                printf("%.14g\n", creal(ans));
            }
        return 0;
    }

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
        case 'C':
            complex_mode();
            break;
        case 'M':
            matrix_mode();
            break;
        case 'U':
            utility_mode();
            break;

        default:
            exit(1);
        }
        printf("\n");
    }
}
