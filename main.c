/*
Copyright (C) 2021-2022 Ahmad Ismail
SPDX-License-Identifier: GPL-3.0-or-later
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interactive.h"
double complex ans = 0;
int main(int argc, char **argv)
{
    // Calculate expressions sent as command line arguments.
    if(argc >1)
    {
        for (int i = 1; i < argc;++i)
        {
            ans = calculate_expr(argv[i], false);
            if(isnan(creal(ans)))
            {
                error_handler(NULL, 2);
                continue;
            }
            printf("%.14g\n", creal(ans));
        }
        return 0;
    }
    char *expr;
    puts("Interactive mode:");
    //pick the mode or call scientific mode if no mode is specified
    while (1)
    {
        s_input(&expr, NULL, -1);
        if (strcmp("exit", expr) == 0)
        {
            free(expr);
            return 0;
        }
        if (strlen(expr) == 1)
            switch (*expr)
            {
            case 'S':
                scientific_calculator(expr, false);
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

            default:
                scientific_calculator(expr, true);
            }
        else
        {
            scientific_calculator(expr, true);
            expr = NULL;
        }
        system(CLEAR_CONSOLE);
        puts("Interactive mode:");
    }
    /*
    No point in freeing readline history (Linux), your OS will reclaim all memory at the process end
    If ever this project gets into systems that do not free on exit, you should probably find a way to 
    fix the ~200kB not freed memory caused by GNU readline()
    */
}
