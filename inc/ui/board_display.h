#ifndef BOARD_DISPLAY_H
#define BOARD_DISPLAY_H

#include "common/chess_types.h"

void print_chess_board(const chess_state_t *chess);
void print_game_status(const game_context_t *ctx);
void print_move_history(const chess_state_t *chess, int last_moves);

#endif