/*
Copyright (C) 2023 Ahmad Ismail
SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <stdio.h>
#include <stdbool.h>
#include "interactive.h"

void test_scientific(char *buffer)
{
    int field_separator = tms_f_search(buffer, ";", 0, false);
    if (field_separator == -1)
    {
        fputs("Incorrect format for test file, missing ;", stderr);
        return;
    }
    char expr[field_separator + 1];
    double complex expected_ans;
    // Not the most efficient way, but the calculator should be reliable for simple ops.
    expected_ans = tms_solve(buffer + field_separator + 1);

    strncpy(expr, buffer, field_separator);
    expr[field_separator] = '\0';
    puts(expr);

    tms_g_ans = tms_solve(expr);
    if (isnan(creal(tms_g_ans)))
    {
        tms_error_handler(EH_PRINT);
        exit(1);
    }

    // Relative error in case the expected answer is 0
    if (expected_ans == 0)
    {
        if (fabs(creal(expected_ans)) < 1e-15 && fabs(cimag(expected_ans)) < 1e-15)
            puts("Passed");
        else
            fputs("Expected answer is zero but actual answer is non negligible.", stderr);
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

void test_base_n(char *buffer)
{
    int field_separator = tms_f_search(buffer, ";", 0, false);
    if (field_separator == -1)
    {
        fputs("Incorrect format for test file, missing ;", stderr);
        return;
    }
    char expr[field_separator + 1];
    int64_t expected_ans;

    expected_ans = tms_sign_extend(tms_int_solve(buffer + field_separator + 1));

    strncpy(expr, buffer, field_separator);
    expr[field_separator] = '\0';
    puts(expr);

    tms_g_int_ans = tms_sign_extend(tms_int_solve(expr));
    if (tms_error_bit == 1)
    {
        tms_error_handler(EH_PRINT);
        exit(1);
    }

    if (tms_g_int_ans == expected_ans)
        puts("Passed");
    else
    {
        puts("Unexpected result");
        exit(1);
    }
}

void test_mode(char **argv)
{
    // Load the test file, should have the following format:
    // Mode_char:expression1;expected_answer1
    // Mode_char is either S or B

    FILE *test_file;
    test_file = fopen(argv[2], "r");
    if (test_file == NULL)
    {
        fputs("Unable to open test file.\n", stderr);
        exit(1);
    }

    // I won't make lines longer than that to test
    char buffer[1000];

    while (feof(test_file) == 0)
    {
        fgets(buffer, 1000, test_file);

        tms_remove_whitespace(buffer);
        _mode = buffer[0];
        switch (_mode)
        {
        case 'S':
            test_scientific(buffer + 2);
            break;

        case 'B':
            test_base_n(buffer + 2);
            break;

        default:
            fputs("Invalid test mode.\n", stderr);
        }
    }
}
