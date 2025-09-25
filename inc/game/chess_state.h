#ifndef CHESS_STATE_H
#define CHESS_STATE_H

#include "common/chess_types.h"

void init_chess_board(chess_state_t *chess);
bool square_to_index(const char *square, int *row, int *col);
void index_to_square(int row, int col, char *square);
char piece_to_char(piece_t piece);
void chess_state_to_fen(const chess_state_t *chess, char *fen_buffer, size_t buffer_size);

#endif