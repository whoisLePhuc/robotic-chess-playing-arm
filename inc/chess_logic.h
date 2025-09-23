#ifndef CHESS_LOGIC_H
#define CHESS_LOGIC_H

#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>

// Function to start UCI chess engine
int uci_start(const char *engine_path);
// Function to send command to UCI engine
void uci_send(const char *cmd);
// Function to read response best move from UCI engine
const char *uci_read_bestmove();
// Function to start a new game
void uci_newgame(); 
// Function to stop stockfish engine
void uci_stop();
// Function to check logal move
bool uci_check_move(const char *fen, const char *move);


#endif