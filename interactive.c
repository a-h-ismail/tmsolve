/*
Copyright (C) 2021-2022 Ahmad Ismail
SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "interactive.h"
char *autocomplete_names[] = {"ans", "exp", "pi", "fact", "abs", "ceil", "floor", "acosh", "asinh", "atanh", "acos", "asin", "atan",
                              "cosh", "sinh", "tanh", "cos", "sin", "tan", "ln", "log", "int", "der", NULL};
char *character_name_generator(const char *text, int state)
{
    static int list_index, len;
    char *name;

    if (!state)
    {
        list_index = 0;
        len = strlen(text);
    }

    while ((name = autocomplete_names[list_index++]))
    {
        if (strncmp(name, text, len) == 0)
        {
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

// Reads n characters to *buffer (if n=-1 , no character limit), print prompt
// Do not use a size of -1 with stack allocated strings, the "-1" case changes the pointer stored in the buffer to
// a char array allocated with malloc, beware of leaks
void s_input(char **buffer, char *prompt, size_t n)
{
    rl_attempted_completion_function = character_name_completion;
    char *temp;
    while (1)
    {
        temp = readline(prompt);
        if (strlen(temp) == 0)
        {
            puts(NO_INPUT "\n");
            free(temp);
            continue;
        }
        if (n == -1)
        {
            *buffer = temp;
            add_history(temp);
            remove_whitespace(*buffer);
            return;
        }
        else if (strlen(temp) > n)
        {
            printf("Expression is very long, it must be at most %zu characters.\n\n", n);
            free(temp);
        }
        else
        {
            strcpy(*buffer, temp);
            free(temp);
            return;
        }
    }
}
// Simple function to flush stdin, do not use with fgets or it will get blocked due to possible lack of \n
void flush_stdin()
{
    while (fgetc(stdin) != '\n')
        ;
}
// Function that keeps running until a valid input is obtained, returning the result
double get_value(char *prompt)
{
    double value = 0;
    char *expr;
    while (1)
    {
        s_input(&expr, prompt, -1);
        glob_expr = expr;
        if (parenthesis_check(expr) == false)
        {
            error_handler(NULL, 2);
            continue;
        }
        if (implicit_multiplication(&expr) == false)
        {
            error_handler(NULL, 2);
            continue;
        }
        value = calculate_expr(expr, true);
        free(expr);
        if (isnan(value))
            error_handler(NULL, 2);
        else
            return value;
    }
}
void scientific_calculator(char *expr, bool called_by_default)
{
    // clear the console
    system(CLEAR_CONSOLE);
    puts("Current mode: Scientific");
    if (called_by_default == false)
    {
        free(expr);
        s_input(&expr, "> ", -1);
    }
    else
        printf("> %s\n", expr);
    while (1)
    {
        if (strcmp(expr, "exit") == 0)
        {
            free(expr);
            return;
        }
        scientific_complex_picker(expr);
        free(expr);
        // Expression input is at last because the first expression is entered by main
        s_input(&expr, "> ", -1);
    }
}
void print_result(double complex result)
{
    double real = creal(result), imag = cimag(result);
    if (isnan(real) || isnan(imag))
        return;

    printf("= ");
    if (imag != 0)
    {
        if (real > 0)
        {
            printf("%.14g", real);
            if (imag > 0)
                printf("+");
        }
        if (imag == 1)
            printf("i");
        else if (imag == -1)
            printf("-i");
        else
            printf("%.14gi", imag);

        printf("\nModulus = %.14g, argument = %.14g rad = %.14g deg", cabs(result), carg(result), carg(result) * 180 / M_PI);
    }
    else
    {
        printf("%.14g", real);
        fraction fraction_str = decimal_to_fraction(real, false);
        if (fraction_str.c != 0)
        {
            if (fraction_str.a != 0)
                printf("\n= %g + %g / %g", fraction_str.a, fraction_str.b, fraction_str.c);
            printf("\n= %g / %g", (fraction_str.a * fraction_str.c + fraction_str.b), fraction_str.c);
        }
    }
    printf("\n\n");
}
// Function that chooses complex or scientific interpreter based on expression and solves it
void scientific_complex_picker(char *expr)
{
    char *real_characteristics[] = {"int", "der", "%", "!"};
    int i, is_complex;
    i = strlen(expr);
    if (i == 0)
    {
        puts("Empty input.\n");
        return;
    }
    glob_expr = expr;
    // Seeking for integration,derivation,modulo,factorial operators (implying real operations)
    is_complex = 0;
    for (i = 0; i < 4; ++i)
    {
        if (f_search(expr, real_characteristics[i], 0) != -1)
        {
            is_complex = -1;
            break;
        }
    }
    if (cimag(ans) != 0 && f_search(expr, "ans", 0) != -1)
        is_complex = 1;
    if (is_complex == 0)
    {

        i = f_search(expr, "i", 0);
        while (i != -1)
        {
            // Seeking for the complex number "i"
            if (part_of_keyword(expr, "i", "sin", i) || part_of_keyword(expr, "i", "pi", i) || part_of_keyword(expr, "i", "ceil", i))
                i = f_search(expr, "i", i + 1);
            else
            {
                is_complex = 1;
                break;
            }
        }
    }
    // Case where a complex number was found
    if (is_complex == 1)
    {
        ans = calculate_expr(expr, true);
        print_result(ans);
        error_handler(NULL, 2);
        return;
    }
    // Case where the expression seems real, but may yield imaginary numbers
    else
    {
        ans = calculate_expr(expr, false);
        if (!isnan(creal(ans)))
        {
            print_result(ans);
        }
        // If evaluating without enabling complex
        else if (is_complex == 0 && error_handler(NULL, 5, 0) != 0)
        {
            // Backup scientific mode errors
            error_handler(NULL, 6);
            ans = calculate_expr(expr, true);
            // If complex mode also returned errors, print the errors of scientific mode
            if (error_handler(NULL, 5, 0) != 0)
            {
                error_handler(NULL, 7);
                error_handler(NULL, 2, 0);
            }
        }
    }
    error_handler(NULL, 2);
}
void complex_mode()
{
    char *expr;
    puts("Current mode: Complex");
    while (1)
    {
        s_input(&expr, NULL, -1);
        if (strcmp(expr, "exit") == 0)
        {
            free(expr);
            return;
        }
        ans = calculate_expr(expr, true);
        free(expr);
        error_handler(NULL, 2);
    }
}

void function_calculator()
{
    double start, end, step, x;
    int i;
    char *expr, step_op, *function;
    math_expr *math_struct;
    system(CLEAR_CONSOLE);
    puts("Current mode: Function");
    while (1)
    {
        s_input(&function, "f(x) = ", -1);
        if (strcmp(function, "exit") == 0)
        {
            free(function);
            return;
        }
        // Anticipating the extra space caused by implied multiplication
        function = realloc(function, strlen(function) * 4 * sizeof(char));

        if (implicit_multiplication(&function) == false)
        {
            error_handler(NULL, 2);
            continue;
        }
        if (parenthesis_check(function) == false)
        {
            printf("\n");
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
            s_input(&expr, "Step: ", -1);
            i = strlen(expr);
            if (*(expr + i - 1) == '+' || *(expr + i - 1) == '*' || *(expr + i - 1) == '^')
            {
                step_op = *(expr + i - 1);
                *(expr + i - 1) = '\0';
            }
            else
                step_op = '+';
            step = calculate_expr(expr, false);
            if (isnan(step))
            {
                printf("\n");
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
        math_struct = parse_expr(function, true, false);
        while (x <= end)
        {
            prevx = x;
            // Set the value of x in the subexpr_ptr
            set_variable(math_struct, x);
            // Solving the function then printing
            result = evaluate(math_struct);
            if (isnan(result))
            {
                error_handler(NULL, 3, 0);
                printf("f(%g)=Error\n", x);
            }
            else
                printf("f(%g) = %.14g\n", x, result);
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
/*
void utility_functions(char *expr)
{
    int length, i;
    length = strlen(expr);
    for (i = 1; i < length; ++i)
    {
        if (*(expr + i) == '|')
        {
            nroot_solver(expr);
            return;
        }
    }
    for (i = 1; i < length; ++i)
    {
        if (*(expr + i) == '/')
        {
            reduce_fraction(expr, false);
            return;
        }
    }
    for (i = 1; i < length; ++i)
    {
        if (*(expr + i) == '.')
        {
            if (decimal_to_fraction(expr, false) == true)
                printf("\n");
            else
                puts("Unable to process number.");
            return;
        }
    }
    factorize(expr);
}
void utility_mode()
{
    char input[25];
    system(CLEAR_CONSOLE);
    puts("Current mode: Utility");
    while (1)
    {
        s_input(input, NULL, 25);
        if (strcmp(input, "exit") == 0)
            return;
        utility_functions(input);
        error_handler(NULL, 2);
    }
}

//Who said a calculator can't have some secrets?
void rps()
{
    char *operation = (char *)malloc(10 * sizeof(char)), playerc;
    //playerc is the character entered by the player
    int wins = 0, total = 0, playerv, rnv;
    system(CLEAR_CONSOLE);
    puts("You found the secret mode!");
    puts("Rock Paper Scissor, enter r for rock, p for paper, s for scissors.");
    while (1)
    {
        s_input(&operation, NULL, 10);
        if (strncmp(operation, "exit", 4) == 0)
            return;
        playerc = operation[0];
        if (strncmp(operation, "stat", 4) == 0)
        {
            printf("Wins=%d\nTotal=%d\n", wins, total);
            playerc = 'n';
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

void factorize(char *expr)
{
    int32_t *factors, n;
    double check;
    int i, success_count;
    success_count = sscanf(expr, "%lf", &check);
    if (success_count != 1)
    {
        puts("Syntax error.");
        return;
    }
    if (fabs(check) > 2147483647)
    {
        puts("Values must be in range [-2147483647,2147483647]");
        return;
    }
    n = check;
    factors = find_factors(n);
    if (*(factors + 2) == fabs(n))
    {
        printf("%" PRId32 " is a prime number.\n", (int32_t)n);
        free(factors);
        return;
    }
    if (check < 0)
        *(factors + 2) = -*(factors + 2);
    if (*(factors + 3) > 1)
        printf("%" PRId32 " = %" PRId32 "^%" PRId32 "", n, *(factors + 2), *(factors + 3));
    else
        printf("%" PRId32 " = %" PRId32 "", n, *(factors + 2));
    i = 4;
    while (*(factors + i) != 0)
    {
        if (*(factors + i + 1) > 1)
            printf(" * %" PRId32 "^%" PRId32 "", *(factors + i), *(factors + i + 1));
        else
            printf(" * %" PRId32 "", *(factors + i));
        i += 2;
    }
    printf("\n");
    free(factors);
}
// p here must be the index of the root symbol (n)\|value
void nroot_solver(char *expr)
{
    int32_t root = 1, root_power, remain, *factors;
    int i = 2, success_count;
    double check1, check2;
    // Taking input
    if (*expr == '(')
    {
        success_count = sscanf(expr, "(%lf)\\|%lf", &check1, &check2);
        if (success_count != 2)
        {
            puts("Syntax error.");
            return;
        }
    }
    else
    {
        success_count = sscanf(expr, "\\|%lf", &check2);
        check1 = 2;
        if (success_count != 1)
        {
            puts("Syntax error.");
            return;
        }
    }
    if (fabs(check1) > 2147483647 || fabs(check2) > 2147483647 || fabs(check1) < 1 || fabs(check2) < 1)
    {
        puts("Values must be in range [1,2147483647]");
        return;
    }
    root_power = check1;
    remain = check2;
    if (remain == 0)
    {
        puts("= 0");
        return;
    }
    factors = find_factors(remain);
    while (*(factors + i) != 0)
    {
        if (*(factors + i + 1) >= root_power)
            root *= pow(*(factors + i), *(factors + i + 1) / root_power);
        i += 2;
    }
    i = 2;
    remain = 1;
    while (*(factors + i) != 0)
    {
        if (*(factors + i + 1) % root_power > 0)
            remain *= pow(*(factors + i), *(factors + i + 1) % root_power);
        i += 2;
    }
    free(factors);
    if (remain == 1)
        printf("= %" PRId32 "\n", root);
    if (remain > 1 && root > 1)
        printf("= %" PRId32 " * (%" PRId32 ")\\|%" PRId32 " ~ %.10g\n", root, root_power, remain, root * pow(remain, (double)1 / root_power));
    if (root == 1 && remain > 1)
        printf("= (%" PRId32 ")\\|%" PRId32 " ~ %.10g\n", root_power, remain, pow(remain, (double)1 / root_power));
}
*/
matrix_str *matrix_input(int rows, int columns)
{
    matrix_str *matrix = new_matrix(rows, columns);
    int i, j;
    puts("Enter data:");
    char mem_prompt[28];
    for (i = 0; i < rows; ++i)
        for (j = 0; j < columns; ++j)
        {
            sprintf(mem_prompt, "(%d,%d):", i + 1, j + 1);
            matrix->data[i][j] = get_value(mem_prompt);
        }
    // Don't forget that we previously incremented the pointer by 2
    return matrix;
}
// Read data into matrix M from stdin
void matrix_edit(matrix_str **M)
{
    int a = 0, b = 0, success_count = 0;
    char *buffer;
    if (*M != NULL)
        free(*M);
    do
    {
        buffer = readline("Enter dimensions (format: m*n): ");
        if (strlen(buffer) == 0)
        {
            puts("Empty input.\n");
            free(buffer);
            continue;
        }
        success_count = sscanf(buffer, "%d*%d", &a, &b);
        free(buffer);
        if (a <= 0 || b <= 0)
            puts("Error, dimensions must be strictly positive.");
        if (success_count != 2)
            puts("Error, invalid input\n");
    } while (a <= 0 || b <= 0 || success_count != 2);
    *M = matrix_input(a, b);
}
void matrix_print(matrix_str *A)
{
    int i, j;
    if (A == NULL)
    {
        puts("No data in the selected matrix.");
        return;
    }
    for (i = 0; i < A->rows; ++i)
    {
        printf("|");
        for (j = 0; j < A->columns; ++j)
            printf("\t%8.8g", A->data[i][j]);
        printf("\t|\n");
    }
}

// The matrixes as global variables
matrix_str *A, *B, *C, *D, *E, *R, *prevR;
// Function that matches the letter to the matrix it represents.
bool match_matrix(matrix_str ***pointer, char operand)
{
    switch (operand)
    {
    case 'A':
        *pointer = &A;
        break;
    case 'B':
        *pointer = &B;
        break;
    case 'C':
        *pointer = &C;
        break;
    case 'D':
        *pointer = &D;
        break;
    case 'E':
        *pointer = &E;
        break;
    case 'R':
        *pointer = &R;
        break;
    default:
        return false;
    }
    return true;
}
void matrix_mode()
{
    char buffer[15], *operation = buffer;
    char op1, op2;
    matrix_str **operand1 = NULL, **operand2 = NULL;
    double det;
    A = B = C = D = E = R = prevR = NULL;
    system(CLEAR_CONSOLE);
    puts("Current mode: Matrix.");
    puts("Supported operations: editX, multiplyXY, detX, comX, invX, trX, copyXY, printX.");
    puts("Available matrixes: A, B, C, D, E, R.\nR stores the result of operations.");
    while (1)
    {
        puts("\nOperation:");
        s_input(&operation, NULL, 15);
        if (strncmp(operation, "exit", 4) == 0)
            return;
        // If R is pointing to a matrix, copy the pointer to prevR so R can be used to store result of the next operation
        if (R != NULL)
            prevR = R;
        if (strncmp("multiply", operation, 8) == 0)
        {
            sscanf(operation, "multiply%c%c", &op1, &op2);
            // Match letter with corresponding operator
            if (match_matrix(&operand1, op1) == false || match_matrix(&operand2, op2) == false)
            {
                puts("Invalid operand.");
                continue;
            }
            if (*operand1 != NULL && *operand2 != NULL)
                R = matrix_multiply(*operand1, *operand2);
            else
                puts("Error, no data.");
            // In case of failure, copy the previous pointer of R back to R
            if (R == NULL)
                R = prevR;
            else
            {
                puts("Result stored in R:");
                matrix_print(R);
            }
        }
        else if (strncmp(operation, "copy", 4) == 0)
        {
            sscanf(operation, "copy%c%c", &op1, &op2);
            // Match letter with corresponding operator
            if (match_matrix(&operand1, op1) == false || match_matrix(&operand2, op2) == false)
            {
                puts("Invalid operand.");
                continue;
            }
            if (operand1 == operand2)
                continue;
            matrix_str *tmp;
            tmp = matrix_copy(*operand1);
            if (tmp != NULL)
            {
                puts("Copy done successfuly.");
                free(*operand2);
                *operand2 = tmp;
            }
            else
                puts("Copy failure.");
        }
        else if (strncmp(operation, "edit", 4) == 0)
        {
            sscanf(operation, "edit%c", &op1);
            if (match_matrix(&operand1, op1) == false)
            {
                puts("Invalid operand.");
                continue;
            }
            matrix_edit(operand1);
            matrix_print(*operand1);
        }
        else if (strncmp(operation, "tr", 2) == 0)
        {
            sscanf(operation, "tr%c", &op1);
            if (match_matrix(&operand1, op1) == false)
            {
                puts("Invalid operand.");
                continue;
            }
            R = matrix_tr(*operand1);
            puts("Result:");
            matrix_print(R);
        }
        else if (strncmp(operation, "print", 5) == 0)
        {
            sscanf(operation, "print%c", &op1);
            if (match_matrix(&operand1, op1) == false)
            {
                puts("Invalid operand.");
                continue;
            }
            matrix_print(*operand1);
        }
        else if (strncmp(operation, "det", 3) == 0)
        {
            sscanf(operation, "det%c", &op1);
            if (match_matrix(&operand1, op1) == false)
            {
                puts("Invalid operand.");
                continue;
            }
            det = matrix_det(*operand1);
            if (isnan(det))
                puts("Indeterminate determinant.");
            else
                printf("%.9g\n", det);
        }
        else if (strncmp(operation, "inv", 3) == 0)
        {
            sscanf(operation, "inv%c", &op1);
            if (match_matrix(&operand1, op1) == false)
            {
                puts("Invalid operand.");
                continue;
            }
            R = matrix_inv(*operand1);
            if (R == NULL)
                R = prevR;
            else
            {
                puts("Result stored in R:");
                matrix_print(R);
            }
        }
        else if (strncmp(operation, "com", 3) == 0)
        {
            prevR = R;
            sscanf(operation, "com%c", &op1);
            if (match_matrix(&operand1, op1) == false)
            {
                puts("Invalid operand.");
                continue;
            }
            R = comatrix(*operand1);
            if (R == NULL)
                R = prevR;
            else
            {
                puts("Result stored in R:");
                matrix_print(R);
            }
        }
        else
            puts("Invalid operation.");
        if (R != prevR)
            free(prevR);
    }
    free(A);
    free(B);
    free(C);
    free(D);
    free(E);
    free(R);
}