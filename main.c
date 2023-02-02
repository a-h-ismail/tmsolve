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
