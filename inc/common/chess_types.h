#ifndef CHESS_TYPES_H
#define CHESS_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h>
#include <ctype.h>

#define BOARD_SIZE 8
#define MAX_MOVES 1000
#define MAX_UCI_RESPONSE 4096
#define MAX_ENGINE_PATH 256
#define MAX_MESSAGE_LEN 256

typedef enum {
    EMPTY = 0, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
} piece_type_t;

typedef enum {
    COLOR_NONE = 0, WHITE, BLACK
} color_t;

typedef struct {
    piece_type_t type;
    color_t color;
} piece_t;

typedef struct {
    char notation[6];
    piece_t moved_piece;
    piece_t captured_piece;
    int from_row, from_col;
    int to_row, to_col;
    bool is_castle;
    bool is_en_passant;
    bool is_promotion;
    char promotion_piece;
} move_t;

typedef struct {
    piece_t board[BOARD_SIZE][BOARD_SIZE];
    color_t turn;
    bool white_can_castle_kingside;
    bool white_can_castle_queenside;
    bool black_can_castle_kingside;
    bool black_can_castle_queenside;
    char en_passant_target[3];
    int halfmove_clock;
    int fullmove_number;
    move_t move_history[MAX_MOVES];
    int move_count;
} chess_state_t;

typedef struct {
    int engine_in[2];
    int engine_out[2];
    pid_t pid;
    bool is_running;
    char response_buffer[MAX_UCI_RESPONSE];
    char engine_path[MAX_ENGINE_PATH];
} uci_engine_t;

typedef enum {
    PLAYER_HUMAN = 0, PLAYER_ENGINE
} player_type_t;

typedef enum {
    GAME_MENU = 0,
    GAME_SETUP,
    GAME_PLAYING,
    GAME_ENGINE_THINKING,
    GAME_WAITING_HUMAN,
    GAME_GAME_OVER,
    GAME_ERROR,
    GAME_EXIT
} game_state_type_t;

typedef enum {
    MOVE_SUCCESS = 0,
    MOVE_INVALID_FORMAT,
    MOVE_INVALID_SQUARE,
    MOVE_NO_PIECE,
    MOVE_ILLEGAL,
    MOVE_KING_IN_CHECK,
    MOVE_GAME_OVER
} move_result_t;

typedef struct {
    game_state_type_t state;
    chess_state_t chess;
    uci_engine_t engine;
    player_type_t white_player;
    player_type_t black_player;
    char last_move[16];
    char status_message[MAX_MESSAGE_LEN];
    bool game_over;
    color_t winner;
} game_context_t;

#ifdef __cplusplus
}
#endif

#endif