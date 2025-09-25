#ifndef UCI_ENGINE_H
#define UCI_ENGINE_H

#include "common/chess_types.h"

bool uci_start_engine(uci_engine_t *engine, const char *path);
void uci_stop_engine(uci_engine_t *engine);
bool uci_send_command(uci_engine_t *engine, const char *command);
bool uci_read_response(uci_engine_t *engine, char *buffer, size_t buffer_size, int timeout_ms);
bool uci_get_best_move(uci_engine_t *engine, char *move_buffer, size_t buffer_size);
bool uci_set_position(uci_engine_t *engine, const chess_state_t *chess);

#endif