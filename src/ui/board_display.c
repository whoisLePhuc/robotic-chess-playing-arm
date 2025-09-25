#include "ui/board_display.h"
#include "game/chess_state.h"

void print_chess_board(const chess_state_t *chess) {
    if (!chess) return;
    
    printf("\n  +---+---+---+---+---+---+---+---+\n");
    for (int r = 0; r < BOARD_SIZE; r++) {
        printf("%d |", 8 - r);
        for (int c = 0; c < BOARD_SIZE; c++) {
            printf(" %c |", piece_to_char(chess->board[r][c]));
        }
        printf("\n  +---+---+---+---+---+---+---+---+\n");
    }
    printf("    a   b   c   d   e   f   g   h\n\n");
}

void print_game_status(const game_context_t *ctx) {
    if (!ctx) return;
    
    printf("Turn: %s (%s)\n", 
           (ctx->chess.turn == WHITE) ? "White" : "Black",
           ((ctx->chess.turn == WHITE) ? ctx->white_player : ctx->black_player) == PLAYER_HUMAN ? "Human" : "Engine");
    
    if (strlen(ctx->last_move) > 0) {
        printf("Last move: %s\n", ctx->last_move);
    }
    
    if (strlen(ctx->status_message) > 0) {
        printf("Status: %s\n", ctx->status_message);
    }
    
    printf("Move: %d, Halfmove clock: %d\n", 
           ctx->chess.fullmove_number, ctx->chess.halfmove_clock);
    printf("\n");
}

void print_move_history(const chess_state_t *chess, int last_moves) {
    if (!chess || chess->move_count == 0) return;
    
    printf("Move History (last %d):\n", last_moves);
    int start = (chess->move_count > last_moves) ? chess->move_count - last_moves : 0;
    
    for (int i = start; i < chess->move_count; i++) {
        move_t *move = &chess->move_history[i];
        printf("%d. %s", (i / 2) + 1, move->notation);
        if (move->is_castle) printf(" (castle)");
        if (move->is_en_passant) printf(" (e.p.)");
        if (move->is_promotion) printf(" (=%c)", move->promotion_piece);
        if (move->captured_piece.type != EMPTY) printf(" x%c", piece_to_char(move->captured_piece));
        
        if (i % 2 == 0) printf("  ");
        else printf("\n");
    }
    if (chess->move_count % 2 == 1) printf("\n");
    printf("\n");
}