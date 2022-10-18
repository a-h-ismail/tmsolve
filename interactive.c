/*
Copyright (C) 2021-2022 Ahmad Ismail
SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "interactive.h"
#if defined(_WIN64)
// This function mimics some of the behavior of GNU readline function, but for non Linux systems
char *readline(char *prompt)
{
    char *buffer = (char *)malloc(10000 * sizeof(char)), c;
    size_t count = 0, size = 10000;
    if (prompt != NULL)
        printf("%s", prompt);
    do
    {
        c = getc(stdin);
        ++count;
        if (count > size)
        {
            size += 10000;
            buffer = realloc(buffer, size);
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
//Reads n characters to *buffer (if n=-1 , no character limit), print prompt
//Do not use a size of -1 with stack allocated strings, the "-1" case changes the pointer stored in the buffer to
//a char array allocated with malloc, beware of leaks
void s_input(char **buffer, char *prompt, size_t n)
{
    char *temp;
    while (1)
    {
        temp = readline(prompt);
        if (n == -1)
        {
            *buffer = temp;
#if defined(__linux)
            add_history(temp);
#endif
            //string cleaner removes spaces used to improve visibility while entering an expression
            string_cleaner(*buffer);
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
//Simple function to flush stdin, do not use with fgets or it will get blocked due to possible lack of \n
void flush_stdin()
{
    while (fgetc(stdin) != '\n')
        ;
}
// Function that keeps running until a valid input is obtained, returning the result
double get_value(char *prompt)
{
    double value = 0;
    char *exp;
    while (1)
    {
        s_input(&exp, prompt, -1);
        exp = realloc(exp, EXP_SIZE(strlen(exp)) * sizeof(char));
        g_exp = exp;
        if (parenthesis_check(exp) == false)
        {
            error_handler(NULL, 2);
            continue;
        }
        if (implicit_multiplication(exp) == false)
        {
            error_handler(NULL, 2);
            continue;
        }
        variable_matcher(exp);
        value = scientific_interpreter(exp);
        free(exp);
        if (isnan(value))
            error_handler(NULL, 2);
        else
            return value;
    }
}
void scientific_calculator(char *exp, bool called_by_default)
{
    //clear the console
    system(CLEAR_CONSOLE);
    puts("Current mode: Scientific");
    if (called_by_default == false)
    {
        free(exp);
        s_input(&exp, NULL, -1);
    }
    else
        puts(exp);
    while (1)
    {
        if (strcmp(exp, "exit") == 0)
        {
            free(exp);
            return;
        }
        scientific_complex_picker(exp);
        //Expression input is at last because the first expression is entered by main
        s_input(&exp, NULL, -1);
    }
}
//Function that chooses complex or scientific interpreter based on expression and solves it
void scientific_complex_picker(char *exp)
{
    char *real_characteristics[] = {"int", "d/dx", "%", "!"};
    int i, is_complex;
    i = strlen(exp);
    if (i == 0)
    {
        puts("Empty input.\n");
        free(exp);
        return;
    }
    exp = realloc(exp, EXP_SIZE(i) * sizeof(char));
    g_exp = exp;
    //Seeking for integration,derivation,modulo,factorial operators (implying real operations)
    is_complex = 0;
    for (i = 0; i < 4; ++i)
    {
        if (s_search(exp, real_characteristics[i], 0) != -1)
        {
            is_complex = -1;
            break;
        }
    }
    if (is_complex == 0)
    {
        i = s_search(exp, "i", 0);
        while (i != -1)
        {
            //Seeking for the complex number "i"
            if (part_of_keyword(exp, "i", "sin", i) || part_of_keyword(exp, "i", "pi", i) || part_of_keyword(exp, "i", "ceil", i))
                i = s_search(exp, "i", i + 1);
            else
            {
                is_complex = 1;
                break;
            }
        }
    }
    //Case where a complex number was found
    if (is_complex == 1)
    {
        complex_interpreter(exp);
        error_handler(NULL, 2, 1);
        free(exp);
        return;
    }
    //Case where the expression seems real, but may yield imaginary numbers
    else
    {
        ans = scientific_interpreter(exp);
        if (!isnan(creal(ans)))
        {
            printf("= %.12g\n", creal(ans));
            if (decimal_to_fraction(exp, false) == true)
                printf("\n");
        }
        //Case where the error handler collected errors from the scientific interpreter, try the complex interpreter
        else if (is_complex == 0 && error_handler(NULL, 5, 0) != 0)
        {
            //Backup scientific mode errors
            error_handler(NULL, 6);
            complex_interpreter(exp);
            //If complex mode also returned errors, print the errors of scientific mode
            if (error_handler(NULL, 5, 0) != 0)
            {
                error_handler(NULL, 7);
                error_handler(NULL, 2, 0);
            }
        }
    }
    error_handler(NULL, 2, 1);
    free(exp);
}
void complex_mode()
{
    char *exp;
    system(CLEAR_CONSOLE);
    puts("Current mode: Complex");
    while (1)
    {
        s_input(&exp, NULL, -1);
        if (strcmp(exp, "exit") == 0)
        {
            free(exp);
            return;
        }
        exp = realloc(exp, EXP_SIZE(strlen(exp)));
        complex_interpreter(exp);
        free(exp);
        error_handler(NULL, 2, 1);
    }
}

void function_calculator()
{
    double start, end, step, x;
    int i, length;
    char *exp, step_op, *function;
    s_expression *compiled_func;
    system(CLEAR_CONSOLE);
    puts("Current mode: Function");
    while (1)
    {
        s_input(&exp, "f(x) = ", -1);
        if (strcmp(exp, "exit") == 0)
        {
            free(exp);
            return;
        }
        length = strlen(exp);
        exp = realloc(exp, EXP_SIZE(length) * sizeof(char));
        if (*exp == '\0')
        {
            puts("Empty input.\n");
            continue;
        }
        if (s_search(exp, "int", 0) != -1)
        {
            puts("Error: integration is not allowed in function mode.\n");
            continue;
        }
        if (s_search(exp, "d/dx", 0) != -1)
        {
            puts("Error: derivation is not allowed in function mode.\n");
            continue;
        }
        if (implicit_multiplication(exp) == false)
        {
            error_handler(NULL, 2, 1);
            continue;
        }
        if (parenthesis_check(exp) == false)
        {
            printf("\n");
            continue;
        }
        variable_matcher(exp);
        //Copy the entered funtion to a separate array
        length = strlen(exp);
        function = (char *)malloc((length + 1) * sizeof(char));
        strcpy(function, exp);
        free(exp);
        //Use exp to calculate any possible expression entered in start, end
        //Entering start
        start = get_value("Start: ");
        printf("%.12g\n", start);
        //Entering end
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
        //Entering step and identifing step operator
        while (1)
        {
            s_input(&exp, "Step: ", -1);
            i = strlen(exp);
            exp = realloc(exp, EXP_SIZE(i) * sizeof(char));
            if (*(exp + i - 1) == '+' || *(exp + i - 1) == '*' || *(exp + i - 1) == '^')
            {
                step_op = *(exp + i - 1);
                *(exp + i - 1) = '\0';
            }
            else
                step_op = '+';
            variable_matcher(exp);
            step = scientific_interpreter(exp);
            if (isnan(step))
            {
                printf("\n");
                free(exp);
                continue;
            }
            else
            {
                puts(exp);
                free(exp);
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
        variable_data *variables;
        compiled_func = scientific_compiler(function,true);
        variables = variable_nodes(compiled_func);
        while (x <= end)
        {
            prevx = x;
            //Set the value of x in the subexps
            replace_variable(variables, x);
            //solving the function then printing
            result = solve_s_exp(compiled_func);
            if (isnan(result))
            {
                error_handler(NULL, 3, 0);
                printf("f(%g)=Error\n", x);
            }
            else
            {
                printf("f(%g) = %.14g\n", x, result);
                if (decimal_to_fraction(exp, false) == true)
                    printf("\n");
            }
            //calculating the next value of x according to the specified method
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
void utility_functions(char *exp)
{
    int length, i;
    length = strlen(exp);
    for (i = 1; i < length; ++i)
    {
        if (*(exp + i) == '|')
        {
            nroot_solver(exp);
            return;
        }
    }
    for (i = 1; i < length; ++i)
    {
        if (*(exp + i) == '/')
        {
            fraction_processor(exp, false);
            return;
        }
    }
    for (i = 1; i < length; ++i)
    {
        if (*(exp + i) == '.')
        {
            if (decimal_to_fraction(exp, false) == true)
                printf("\n");
            else
                puts("Unable to process number.");
            return;
        }
    }
    factorize(exp);
}
void utility_mode()
{
    char stack_exp[25], *exp = (char *)stack_exp;
    system(CLEAR_CONSOLE);
    puts("Current mode: Utility");
    while (1)
    {
        s_input(&exp, NULL, 25);
        if (strcmp(exp, "exit") == 0)
            return;
        utility_functions(exp);
        error_handler(NULL, 2);
    }
}

I will look into this section later, I should first make the scientific mode stable
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

void factorize(char *exp)
{
    int32_t *factors, n;
    double check;
    int i, success_count;
    success_count = sscanf(exp, "%lf", &check);
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
void nroot_solver(char *exp)
{
    int32_t root = 1, root_power, remain, *factors;
    int i = 2, success_count;
    double check1, check2;
    // Taking input
    if (*exp == '(')
    {
        success_count = sscanf(exp, "(%lf)\\|%lf", &check1, &check2);
        if (success_count != 2)
        {
            puts("Syntax error.");
            return;
        }
    }
    else
    {
        success_count = sscanf(exp, "\\|%lf", &check2);
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

// Function that keeps running until a valid input is obtained, returning the result
double get_value(char *prompt)
{
    double value = 0;
    char *exp;
    while (1)
    {
        s_input(&exp, prompt, -1);
        exp = realloc(exp, EXP_SIZE(strlen(exp)) * sizeof(char));
        if (parenthesis_check(exp) == false)
        {
            error_handler(NULL, 2);
            continue;
        }
        if (implicit_multiplication(exp) == false)
        {
            error_handler(NULL, 2);
            continue;
        }
        variable_matcher(exp);
        value = scientific_interpreter(exp, false);
        free(exp);
        if (isnan(value))
            error_handler(NULL, 2);
        else
            return value;
    }
}
void equation_mode()
{
    int degree;
    char *operation = (char *)malloc(10 * sizeof(char));
    system(CLEAR_CONSOLE);
    puts("Current mode: Equation");
    puts("Enter S followed by a number n for a linear system of n equations.");
    puts("Enter E followed by a number to calculate a linear equation of degree n.");
    while (1)
    {
        printf("\n");
        s_input(&operation, NULL, 9);
        if (strcmp(operation, "exit") == 0)
            break;
        switch (*operation)
        {
        case 'S':
            if (sscanf(operation, "S%d", &degree) == 0)
            {
                puts("Invalid input.");
                break;
            }
            else
                system_solver(degree);
            break;
        case 'E':
            if (sscanf(operation, "E%d", &degree) == 0)
            {
                puts("Invalid input.");
                break;
            }
            else
                equation_solver(degree);
            break;
        default:
            puts("Invalid operation.");
        }
    }
    free(operation);
}
double *matrix_input(int a, int b)
{
    char *exp;
    double *matrix;
    int i, j, success_count;
    // We reserve 2 extra spaces at the beginning for the dimensions
    matrix = (double *)malloc((2 + a * b) * sizeof(double));
    // We save the dimensions like mentioned before
    *matrix = a;
    *(matrix + 1) = b;
    // offset the pointer by 2 to use the array as if there was no 2 spaces reserved for dimensions
    matrix = matrix + 2;
    puts("Enter data:");
    char buffer[28];
    for (i = 0; i < a; ++i)
        for (j = 0; j < b; ++j)
        {
            do
            {
                success_count = 0;
                sprintf(buffer, "(%d,%d): ", i + 1, j + 1);
                s_input(&exp, buffer, -1);
                if (strcmp(exp, "exit") == 0)
                {
                    free(exp);
                    free(matrix - 2);
                    return NULL;
                }
                if (strlen(exp) == 0)
                {
                    puts("Empty input.\n");
                    free(exp);
                    continue;
                }
                exp = realloc(exp, EXP_SIZE(strlen(exp)));
                *(matrix + (i * b) + j) = pre_scientific_interpreter(exp);
                if (isnan(*(matrix + (i * b) + j)))
                {
                    error_handler(NULL, 2, 1);
                    success_count = 0;
                    free(exp);
                    continue;
                }
                free(exp);
            } while (success_count != 1);
            printf("%s\n", exp);
            free(exp);
        }
    // Don't forget that we previously incremented the pointer by 2
    return matrix - 2;
}
// Read data into matrix M
void matrix_edit(double **M)
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
void matrix_print(double *A)
{
    int i, j, a, b;
    if (A == NULL)
    {
        puts("No data in the selected matrix");
        return;
    }
    a = *A;
    b = *(A + 1);
    A = A + 2;
    for (i = 0; i < a; ++i)
    {
        printf("|");
        for (j = 0; j < b; ++j)
            printf("\t%8.8g", *(A + (b * i) + j));
        printf("\t|\n");
    }
}

// The matrixes as global variables
double *A, *B, *C, *D, *E, *R, *prevR;
// Function that matches the letter to the matrix it represents.
bool match_matrix(double ***pointer, char operand)
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
    double **operand1 = NULL, **operand2 = NULL;
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
            double *temp;
            temp = matrix_copy(*operand1);
            if (temp != NULL)
            {
                puts("Copy done successfuly.");
                free(*operand2);
                *operand2 = temp;
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
*/