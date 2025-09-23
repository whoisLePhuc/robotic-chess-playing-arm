#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <stdio.h>
#include <string.h>

typedef enum{
    EMPTY,
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
} piece_type;

typedef enum{
    WHITE, BLACK, NONE
} piece_color;

typedef struct{
    piece_type type;
    piece_color color;
} piece;

// Function to initialize the chessboard with pieces in their starting positions
void init_chessboard(piece board[8][8]);
// Function to print the chessboard to the console
void print_chessboard(piece board[8][8]);
// Function to make move
int make_move(piece board[8][8], const char *uci_move);

#endif