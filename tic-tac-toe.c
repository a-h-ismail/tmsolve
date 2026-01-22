/*
Copyright (C) 2023 Ahmad Ismail
SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "interactive.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern bool pref_suppress_output;

int free_fields[9][2];
void display_board(char **board)
{
    for (int i = 0; i < 3; ++i)
    {
        puts("+-------+-------+-------+");
        puts("|       |       |       |");
        printf("|   %c   |   %c   |   %c   |\n", board[i][0], board[i][1], board[i][2]);
        puts("|       |       |       |");
    }
    puts("+-------+-------+-------+\n");
}

// Get free fields into a 9*2 array
void get_free_fields(char **board)
{
    int i, j, k = 0;
    for (i = 0; i < 3; ++i)
        for (j = 0; j < 3; ++j)
        {
            if (board[i][j] != 'X' && board[i][j] != 'O')
            {
                free_fields[k][0] = i;
                free_fields[k][1] = j;
                ++k;
            }
        }
    if (k < 9)
        free_fields[k][0] = -1;
}

bool is_free_field(int row, int column)
{
    int i;
    for (i = 0; i < 9 && free_fields[i][0] != -1; ++i)
    {
        if (free_fields[i][0] == row && free_fields[i][1] == column)
            return true;
    }
    return false;
}

// Enters the player move
int enter_move(char **board)
{
    int status, position, row, column;
    char input[9];
    get_free_fields(board);
    // No fields are free
    if (free_fields[0][0] == -1)
        return 1;

    // Case of 1 more element remaining, take it automatically
    else if (free_fields[1][0] == -1)
    {
        board[free_fields[0][0]][free_fields[0][1]] = 'O';
        return 2;
    }
    while (1)
    {
        get_input(input, "Enter position on board [1-9]: ", 9);
        switch (management_input(input))
        {
        case SWITCH_MODE:
            return -1;

        case NEXT_ITERATION:
            continue;
        }

        status = sscanf(input, "%d", &position);
        if (status == 0)
        {
            puts("Invalid input.");
            continue;
        }

        else if (position < 1 || position > 9)
        {
            puts("Field out of range.");
            continue;
        }
        --position;
        row = position / 3;
        column = position % 3;
        if (is_free_field(row, column))
        {
            board[row][column] = 'O';
            display_board(board);
            break;
        }
        else
            puts("Field already taken, try again.");
    }
    return 0;
}

// Analyse the board status and return true if the player using the "symbol" won
bool victory_for(char **board, char symbol)
{
    int i;
    for (i = 0; i < 3; ++i)
    {
        // Row win
        if (board[i][0] == symbol && board[i][1] == symbol && board[i][2] == symbol)
            return true;
        // Column win
        else if (board[0][i] == symbol && board[1][i] == symbol && board[2][i] == symbol)
            return true;
    }
    // Diagonal win
    if (board[1][1] == symbol &&
        ((board[0][0] == symbol && board[2][2] == symbol) || (board[0][2] == symbol && board[2][0] == symbol)))
        return true;

    return false;
}

// Analyses the board to decide the move of the computer
int draw_move(char **board)
{
    int i, j, k, field_to_take[2];
    get_free_fields(board);
    int number_of_symbols = 0;
    char symbols[2] = {'X', 'O'}, cur_symbol;
    if (free_fields[0][0] == -1)
        return -1;

    // Detect immediate win/loss (missing one symbol)
    for (k = 0; k < 2; ++k)
    {
        cur_symbol = symbols[k];

        // Analyze board rows
        for (i = 0; i < 3; ++i)
        {
            field_to_take[0] = -1;
            number_of_symbols = 0;
            for (j = 0; j < 3; ++j)
            {
                if (is_free_field(i, j))
                {
                    field_to_take[0] = i;
                    field_to_take[1] = j;
                }
                else if (board[i][j] == cur_symbol)
                    ++number_of_symbols;

                if (number_of_symbols == 2 && field_to_take[0] != -1)
                    return field_to_take[0] * 3 + field_to_take[1];
            }
        }

        // Analyze boad columns
        for (i = 0; i < 3; ++i)
        {
            field_to_take[0] = field_to_take[1] = -1;
            number_of_symbols = 0;
            for (j = 0; j < 3; ++j)
            {
                if (is_free_field(j, i))
                {
                    field_to_take[0] = j;
                    field_to_take[1] = i;
                }
                else if (board[j][i] == cur_symbol)
                    ++number_of_symbols;

                if (number_of_symbols == 2 && field_to_take[0] != -1)
                    return field_to_take[0] * 3 + field_to_take[1];
            }
        }

        // Left to right diagonal
        field_to_take[0] = -1;
        i = number_of_symbols = 0;
        while (i < 3)
        {
            //[0][0], [1][1], [2][2]
            if (is_free_field(i, i))
                field_to_take[0] = field_to_take[1] = i;
            else if (board[i][i] == cur_symbol)
                number_of_symbols += 1;
            i += 1;
        }
        if (number_of_symbols == 2 && field_to_take[0] != -1)
            return field_to_take[0] * 3 + field_to_take[1];

        // Left to right diagonal
        field_to_take[0] = -1;
        i = number_of_symbols = 0;
        while (i < 3)
        {
            //[0][2], [1][1], [2][0]
            j = 2 - i;
            if (is_free_field(i, j))
            {
                field_to_take[0] = i;
                field_to_take[1] = j;
            }
            else if (board[i][j] == cur_symbol)
                number_of_symbols += 1;
            i += 1;
        }
        if (number_of_symbols == 2 && field_to_take[0] != -1)
            return field_to_take[0] * 3 + field_to_take[1];
    }
    // Take an available strategic field (center, corners)
    int strategic_field[5][2] = {{1, 1}, {0, 0}, {2, 2}, {2, 0}, {0, 2}};
    for (k = 0; k < 5; ++k)
        if (is_free_field(strategic_field[k][0], strategic_field[k][1]))
            return strategic_field[k][0] * 3 + strategic_field[k][1];

    // Last resort: randomness
    while (1)
    {
        i = rand() % 9;
        if (is_free_field(i / 3, i % 3))
            return i;
    }
}

void computer_move(char **board)
{
    int move = draw_move(board);
    if (move != -1)
    {
        board[move / 3][move % 3] = 'X';
        printf("Computer plays at position %d\n", move + 1);
    }
}

void tic_tac_toe()
{
    pref_suppress_output = false;

    char **board;
    int i, status, wins = 0, losses = 0, draws = 0;
    bool computer_start = false;
    board = malloc(3 * sizeof(char *));
    for (i = 0; i < 3; ++i)
        board[i] = malloc(3 * sizeof(char));
    puts("Current mode: Tic-Tac-Toe");
    while (1)
    {
        for (i = 0; i < 9; ++i)
            board[i / 3][i % 3] = '1' + i;
        sleep(2);
        printf("Wins = %d\nLosses = %d\nDraws = %d\n", wins, losses, draws);

        if (computer_start)
            computer_move(board);
        display_board(board);
        while (1)
        {
            status = enter_move(board);
            sleep(1);
            if (status == -1)
            {
                for (i = 0; i < 3; ++i)
                    free(board[i]);
                free(board);
                return;
            }

            if (status == 1)
            {
                puts("Draw");
                ++draws;
                break;
            }

            if (victory_for(board, 'O'))
            {
                puts("You win!");
                ++wins;
                break;
            }

            computer_move(board);
            display_board(board);
            if (victory_for(board, 'X'))
            {
                puts("You lose :(");
                ++losses;
                break;
            }
        }
        // Flip the bool to alternate between user and computer starting
        computer_start = !computer_start;
    }
}