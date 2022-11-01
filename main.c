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
    char *expr=NULL;
    //Accessing modes using command parameters
    if (argc > 1 && argv[1][0] == '-')
    {
        switch (argv[1][1])
        {
        case 's':
            scientific_calculator(expr, false);
            break;
        case 'c':
            complex_mode();
            break;
        case 'f':
            function_calculator();
            break;
        /*
        case 'e':
            equation_mode();
            break;
        case 'u':
            utility_features();
            break;
        case 'm':
            matrix_mode();
            break;
            */
        default:
            puts("Invalid parameter, refer to readme or the manpage for more informations.");
        }
        system(CLEAR_CONSOLE);
    }
    //A 1 calculation pass from the terminal.
    else if (argc == 2)
    {
        expr = (char *)malloc((strlen(argv[1])+1) * sizeof(char));
        strcpy(expr, argv[1]);
        scientific_complex_picker(expr);
        return 1;
    }
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
            /*
            case 'E':
                equation_mode();
                break;
            case 'U':
                utility_features();
                break;
            case 'M':
                matrix_mode();
                break;
            case 'e':
                rps();
                break;
                */
            default:
                scientific_calculator(expr, true);
            }
        else
        {
            scientific_calculator(expr, true);
            expr = NULL;
        }
        system(CLEAR_CONSOLE);
        free(expr);
        puts("Interactive mode:");
    }
    /*
    No point in freeing readline history (Linux), your OS will reclaim all memory at the process end
    If ever this project gets into systems that do not free on exit, you should probably find a way to 
    fix the ~200kB not freed memory caused by GNU readline()
    */
}
