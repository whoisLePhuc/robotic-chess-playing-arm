#ifndef MOVE_VALIDATION_H
#define MOVE_VALIDATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/chess_types.h"

bool is_legal_move(const chess_state_t *chess, const char *uci_move);
bool is_square_attacked(const chess_state_t *chess, int row, int col, color_t by_color);
bool is_king_in_check(const chess_state_t *chess, color_t king_color);
bool is_checkmate(const chess_state_t *chess);
bool is_stalemate(const chess_state_t *chess);
move_result_t make_move(chess_state_t *chess, const char *uci_move);

#ifdef __cplusplus
}
#endif

#endif