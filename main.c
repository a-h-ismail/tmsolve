/*
Copyright (C) 2021-2023 Ahmad Ismail
SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "interactive.h"
#include "version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char _mode = 'S';
#ifdef USE_READLINE
char _autocomplete_mode = 'S';
#endif

// Windows doesn't have timespec_get()
#ifdef __linux__
#include <time.h>
// Calculates the delta time between start (S) and end (E) times
struct timespec calculate_delta_time(struct timespec *S, struct timespec *E)
{
    struct timespec delta_time;
    delta_time.tv_sec = E->tv_sec - S->tv_sec;
    delta_time.tv_nsec = E->tv_nsec - S->tv_nsec;
    // Carry from seconds to nano seconds
    if (delta_time.tv_nsec < 0 && delta_time.tv_sec > 0)
    {
        --delta_time.tv_sec;
        delta_time.tv_nsec += 1e9;
    }
    return delta_time;
}

struct timespec timer_setup(char mode)
{
    static struct timespec start_time, end_time;
    switch (mode)
    {
    case 's':
        timespec_get(&start_time, TIME_UTC);
        return start_time;
    case 'e':
        timespec_get(&end_time, TIME_UTC);
        return calculate_delta_time(&start_time, &end_time);
    default:
        puts("Invalid mode for timer.");
        exit(1);
    }
}

void print_time(struct timespec T)
{
    printf("Time: %.6g\n\n", T.tv_sec + T.tv_nsec / (double)1e9);
}

int run_benchmark(char *expr)
{
    struct timespec delta_time;
    int iterations = 1e6, i;
    tms_math_expr *M, *MN;

    // Parser benchmark
    printf("Expression %s:\n", expr);
    printf("Running %d iterations of the parser\n", iterations);
    timer_setup('s');

    for (i = 0; i < iterations; ++i)
    {
        M = tms_parse_expr(expr, false, false);
        tms_delete_math_expr(M);
    }
    delta_time = timer_setup('e');
    print_time(delta_time);

    // Copy benchmark
    printf("Running %d iterations of mexpr duplication\n", iterations);
    timer_setup('s');

    M = tms_parse_expr(expr, false, false);
    for (i = 0; i < iterations; ++i)
    {
        MN = tms_dup_mexpr(M);
        tms_delete_math_expr(MN);
    }
    delta_time = timer_setup('e');
    print_time(delta_time);
    tms_delete_math_expr(M);

    // The evaluator is generally far faster than the parser, so more iterations
    iterations = 1e8;
    printf("Running %d iterations of the evaluator\n", iterations);
    M = tms_parse_expr(expr, false, false);
    timer_setup('s');
    for (i = 0; i < iterations; ++i)
        tms_g_ans = tms_evaluate(M);

    tms_delete_math_expr(M);
    delta_time = timer_setup('e');
    print_time(delta_time);
    return 0;
}
#endif

// To not clutter main()
void print_help()
{
    puts("usage: tmsolve [options] [expression]\n");
    puts("Available options:\n");
    puts("\t--debug\n\tEnables additional debugging output.\n");
    puts("\t--benchmark\n\tRuns a simple benchmark for the parser and evaluator (Linux only).\n");
    puts("\t--version\n\tPrints version information for the CLI and libtmsolve.\n");
    puts("\t--test TEST_FILE\n\tCalculates the expressions provided in the test file and compares them with the "
         "provided expected result.\n");
    puts("\t--help\n\tPrint this help prompt.\n");
    puts("The program will start by default in the scientific mode if no command line option is specified.");
}

int main(int argc, char **argv)
{
    // Initialize the library before anything else
    tmsolve_init();

    // Calculate expressions sent as command line arguments.
    if (argc > 1)
    {
        if (strcmp(argv[1], "--debug") == 0)
        {
            _tms_debug = true;
        }
        else if (strcmp(argv[1], "--test") == 0)
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

                char expr[separator + 1];
                double complex expected_ans;
                // Not the most efficient way, but the calculator should be reliable for simple ops.
                expected_ans = tms_solve(buffer + separator + 1);

                strncpy(expr, buffer, separator);
                expr[separator] = '\0';
                puts(expr);

                tms_g_ans = tms_solve(expr);
                if (isnan(creal(tms_g_ans)))
                {
                    tms_error_handler(EH_PRINT);
                    exit(1);
                }

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
            exit(0);
        }
        else if (strcmp(argv[1], "--version") == 0)
        {
            printf("tmsolve version %s\nlibtmsolve version %s ", TMSOLVE_VER, tms_lib_version);
#ifdef LOCAL_BUILD
            printf("(static)\n");
#else
            printf("(dynamic)\n");
#endif
            exit(0);
        }

#ifdef __linux__
        else if (strcmp(argv[1], "--benchmark") == 0)
        {
            char *benchmark_expr[] = {"15.75+3e2-4.8872/2.534e-4",
                                      "5+8+9*8/7.545+57.87^0.56+(5+562/95+7*7^3+(59^2.211)/(7*(pi/3-2))+5*4)",
                                      "light_speed=sqrt(1/(8.8541878128e-12*(4e-7*pi)))",
                                      "(((cos(pi/3))))+0.546545",
                                      "sin(0.7)^2+cos(0.7)^2",
                                      "rand()+827.837"};
            for (int i = 0; i < array_length(benchmark_expr); ++i)
                run_benchmark(benchmark_expr[i]);
            exit(0);
        }
#endif
        else if (strcmp(argv[1], "--help") == 0)
        {
            print_help();
            exit(0);
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

            tms_g_ans = tms_solve(all);

            if (isnan(creal(tms_g_ans)))
            {
                tms_error_handler(EH_PRINT);
                exit(1);
            }
            else
                print_result(tms_g_ans, false);
            exit(0);
        }
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
