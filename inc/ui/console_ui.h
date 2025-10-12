#ifndef CONSOLE_UI_H
#define CONSOLE_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/chess_types.h"

void init_game_context(game_context_t *ctx);
void cleanup_game_context(game_context_t *ctx);
game_state_type_t handle_menu_state(game_context_t *ctx);
game_state_type_t handle_setup_state(game_context_t *ctx);
game_state_type_t handle_playing_state(game_context_t *ctx);
game_state_type_t handle_engine_thinking_state(game_context_t *ctx);
game_state_type_t handle_waiting_human_state(game_context_t *ctx);
game_state_type_t handle_game_over_state(game_context_t *ctx);

#ifdef __cplusplus
}
#endif

#endif
