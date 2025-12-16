/*
Copyright (C) 2021-2025 Ahmad Ismail
SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "interactive.h"
#include "version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Exit with status zero for SIGINT to suppress some Linux terminals like konsole reporting tmsolve crashed
#ifdef __linux__
#include <signal.h>
#include <unistd.h>
void handle_termination(int signum)
{
    if (signum == SIGINT)
        exit(0);
}
#endif

char _mode = 'S';
#ifdef USE_READLINE
char _autocomplete_mode;
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

void print_time_and_rate(int iterations, struct timespec T)
{
    printf("Time: %.6g\n", T.tv_sec + T.tv_nsec / (double)1e9);
    printf("Rate: %.6g iterations/second\n\n", iterations / (T.tv_sec + T.tv_nsec / (double)1e9));
}

int run_solver_benchmark(char *expr)
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
        M = tms_parse_expr(expr, NO_LOCK, NULL);
        tms_delete_math_expr(M);
    }
    delta_time = timer_setup('e');
    print_time_and_rate(iterations, delta_time);

    // Copy benchmark
    printf("Running %d iterations of mexpr duplication\n", iterations);
    timer_setup('s');

    M = tms_parse_expr(expr, NO_LOCK, NULL);
    for (i = 0; i < iterations; ++i)
    {
        MN = tms_dup_mexpr(M);
        tms_delete_math_expr(MN);
    }
    delta_time = timer_setup('e');
    print_time_and_rate(iterations, delta_time);
    tms_delete_math_expr(M);

    // The evaluator is generally far faster than the parser, so more iterations
    iterations = 1e8;
    printf("Running %d iterations of the evaluator\n", iterations);
    M = tms_parse_expr(expr, NO_LOCK, NULL);
    timer_setup('s');
    for (i = 0; i < iterations; ++i)
        tms_g_ans = tms_evaluate(M, NO_LOCK);

    tms_delete_math_expr(M);
    delta_time = timer_setup('e');
    print_time_and_rate(iterations, delta_time);

    // Solver benchmark
    iterations = 1e6;
    printf("Running %d iterations of solver routine (parser+complex_switcher+evaluator)\n", iterations);
    timer_setup('s');
    for (i = 0; i < iterations; ++i)
        tms_g_ans = tms_solve(expr);

    delta_time = timer_setup('e');
    print_time_and_rate(iterations, delta_time);
    return 0;
}

int run_access_benchmark()
{
    int iterations = 1e8;
    printf("Running %d variable lookup operations\n", iterations);
    timer_setup('s');
    for (int i = 0; i < iterations; ++i)
        tms_get_var_by_name("pi");

    struct timespec delta_time = timer_setup('e');
    print_time_and_rate(iterations, delta_time);
    return 0;
}
#endif

// To not clutter main()
void print_help()
{
    puts("usage: tmsolve { [options] | [expression1] [expression2] [...] }\n");
    puts("Available options:\n");
    puts("  --debug       Enables additional debugging output.");
    puts("  --benchmark   Runs a simple benchmark for the parser and evaluator (Linux only).");
    puts("  --version     Prints version information for the CLI and libtmsolve.");
    puts("  --help        Print this help prompt.\n");
    puts("The program will start by default in the scientific mode if no command line option is specified.");
    puts("Expressions provided as arguments can be prefixed with \"I:\" to use integer mode instead of scientific mode.");
}

int main(int argc, char **argv)
{
#ifdef _WIN32
    system("color F0");
#endif

#ifdef __linux__
    // If started up by the desktop launcher, change SIGINT handling to avoid exit 130
    char *desktop_integration = getenv("DESKTOP_LAUNCHER");
    if (desktop_integration != NULL && desktop_integration[0] == '1')
        signal(SIGINT, handle_termination);
#endif

    // Initialize the library before anything else
    tmsolve_init();

    if (argc > 1)
    {
        if (tms_find_str_in_array("--debug", argv, argc, TMS_NOFUNC) != -1)
        {
            _tms_debug = true;
        }
        else if (tms_find_str_in_array("--version", argv, argc, TMS_NOFUNC) != -1)
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
        else if (tms_find_str_in_array("--benchmark", argv, argc, TMS_NOFUNC) != -1)
        {
            char *benchmark_expr[] = {"15.75+3e2-4.8872/2.534e-4",
                                      "5+8+9*8/7.545+57.87^0.56+(5+562/95+7*7^3+(59^2.211)/(7*(pi/3-2))+5*4)",
                                      "sqrt(1/(8.8541878128e-12*(4e-7*pi)))",
                                      "(((cos(pi/3))))+0.546545",
                                      "sin(0.7)^2+cos(0.7)^2",
                                      "rand()+827.837"};
            for (int i = 0; i < array_length(benchmark_expr); ++i)
                run_solver_benchmark(benchmark_expr[i]);
            run_access_benchmark();
            exit(0);
        }
#endif
        else if (tms_find_str_in_array("--help", argv, argc, TMS_NOFUNC) != -1)
        {
            print_help();
            exit(0);
        }

        // Calculate the expressions passed as arguments
        else
        {
            int i;
            double complex result;
            for (i = 1; i < argc; ++i)
            {
                tms_remove_whitespace(argv[i]);
                if (argv[i][1] == ':')
                {
                    switch (toupper(argv[i][0]))
                    {
                    case 'S':
                        argv[i] += 2;
                        result = tms_solve(argv[i]);
                        tms_set_ans(result);
                        if (isnan(creal(result)))
                            puts("nan");
                        else
                            print_result(tms_g_ans, false);
                        break;
                    case 'I':
                        int64_t result;
                        argv[i] += 2;
                        if (tms_int_solve(argv[i], &result) != 0)
                            puts("nan");
                        else
                            printf("%" PRId32 "\n", (int32_t)result);
                    default:
                        fputs("Invalid mode prefix.\n", stderr);
                    }
                }
                else
                {
                    result = tms_solve(argv[i]);
                    tms_set_ans(result);
                    if (isnan(creal(result)))
                        puts("nan");
                    else
                        print_result(tms_g_ans, false);
                }
            }
            exit(0);
        }
    }

// For readline autocompletion
#ifdef USE_READLINE
    rl_attempted_completion_function = character_name_completion;
#endif

    printf("tmsolve %s, library version %s\n", TMSOLVE_VER, tms_lib_version);
    puts("Enter \"mode\" to get a list of available modes.\n"
         "Enter \"help\" to view a description and generic usage of the current mode.\n");

    // pick the mode
    while (1)
    {
        switch (_mode)
        {
        case 'S':
#ifdef USE_READLINE
            _autocomplete_mode = 'S';
#endif
            scientific_mode();
            break;
        case 'I':
#ifdef USE_READLINE
            _autocomplete_mode = 'I';
#endif
            integer_mode();
            break;
        case 'F':
#ifdef USE_READLINE
            _autocomplete_mode = 'S';
#endif
            function_calculator();
            break;
        case 'E':
#ifdef USE_READLINE
            _autocomplete_mode = '\0';
#endif
            equation_mode();
            break;
        case 'U':
#ifdef USE_READLINE
            _autocomplete_mode = '\0';
#endif
            utility_mode();
            break;
        case 'G':
#ifdef USE_READLINE
            _autocomplete_mode = '\0';
#endif
            tic_tac_toe();
            break;

        default:
            exit(1);
        }
        printf("\n");
    }
}
