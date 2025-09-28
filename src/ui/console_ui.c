#include "ui/console_ui.h"
#include "ui/board_display.h"
#include "game/chess_state.h"
#include "game/move_validation.h"
#include "engine/uci_engine.h"
#include "utils/string_utils.h"

void init_game_context(game_context_t *ctx) {
    if (!ctx) return;
    
    memset(ctx, 0, sizeof(game_context_t));
    ctx->state = GAME_MENU;
    ctx->white_player = PLAYER_HUMAN;
    ctx->black_player = PLAYER_ENGINE;
    strcpy(ctx->engine.engine_path, "stockfish");
    ctx->winner = COLOR_NONE;
    init_chess_board(&ctx->chess);
}

void cleanup_game_context(game_context_t *ctx) {
    if (!ctx) return;
    uci_stop_engine(&ctx->engine);
}

game_state_type_t handle_menu_state(game_context_t *ctx) {
    printf("\n=== Chess Game with UCI Engine ===\n");
    printf("1. Play as White vs Engine\n");
    printf("2. Play as Black vs Engine\n");
    printf("3. Engine vs Engine\n");
    printf("4. Human vs Human\n");
    printf("5. View last game moves\n");
    printf("6. Exit\n");
    printf("Choose option (1-6): ");
    
    int choice;
    if (scanf("%d", &choice) != 1) {
        clear_input_buffer();
        strcpy(ctx->status_message, "Invalid input. Please try again.");
        return GAME_MENU;
    }
    clear_input_buffer();
    
    switch (choice) {
        case 1:
            ctx->white_player = PLAYER_HUMAN;
            ctx->black_player = PLAYER_ENGINE;
            return GAME_SETUP;
        case 2:
            ctx->white_player = PLAYER_ENGINE;
            ctx->black_player = PLAYER_HUMAN;
            return GAME_SETUP;
        case 3:
            ctx->white_player = PLAYER_ENGINE;
            ctx->black_player = PLAYER_ENGINE;
            return GAME_SETUP;
        case 4:
            ctx->white_player = PLAYER_HUMAN;
            ctx->black_player = PLAYER_HUMAN;
            return GAME_PLAYING;
        case 5:
            if (ctx->chess.move_count > 0) {
                print_move_history(&ctx->chess, 20);
                printf("Press Enter to continue...");
                getchar();
            } else {
                printf("No moves to display.\n");
                usleep(1000000);
            }
            return GAME_MENU;
        case 6:
            return GAME_EXIT;
        default:
            strcpy(ctx->status_message, "Invalid option. Please try again.");
            return GAME_MENU;
    }
}

game_state_type_t handle_setup_state(game_context_t *ctx) {
    bool need_engine = (ctx->white_player == PLAYER_ENGINE || ctx->black_player == PLAYER_ENGINE);
    
    if (need_engine) {
        printf("\nStarting chess engine...\n");
        
        if (!uci_start_engine(&ctx->engine, ctx->engine.engine_path)) {
            strcpy(ctx->status_message, "Failed to start engine. Make sure Stockfish is installed and in PATH.");
            printf("Error: %s\n", ctx->status_message);
            printf("Press Enter to return to menu...");
            getchar();
            return GAME_MENU;
        }
        
        printf("Engine started successfully!\n");
    }
    
    strcpy(ctx->status_message, "Game ready to start");
    return GAME_PLAYING;
}

game_state_type_t handle_playing_state(game_context_t *ctx) {
    print_chess_board(&ctx->chess);
    print_game_status(ctx);
    
    // Check for game over conditions
    if (is_checkmate(&ctx->chess)) {
        ctx->game_over = true;
        ctx->winner = (ctx->chess.turn == WHITE) ? BLACK : WHITE;
        strcpy(ctx->status_message, "Checkmate!");
        return GAME_GAME_OVER;
    }
    
    if (is_stalemate(&ctx->chess)) {
        ctx->game_over = true;
        ctx->winner = COLOR_NONE;
        strcpy(ctx->status_message, "Stalemate!");
        return GAME_GAME_OVER;
    }
    
    if (ctx->chess.halfmove_clock >= 50) {
        ctx->game_over = true;
        ctx->winner = COLOR_NONE;
        strcpy(ctx->status_message, "50-move rule draw!");
        return GAME_GAME_OVER;
    }
    
    // Check/checkmate warning
    if (is_king_in_check(&ctx->chess, ctx->chess.turn)) {
        printf("*** %s KING IN CHECK! ***\n", 
               (ctx->chess.turn == WHITE) ? "WHITE" : "BLACK");
    }
    
    // Determine current player type
    player_type_t current_player = (ctx->chess.turn == WHITE) ? ctx->white_player : ctx->black_player;
    
    if (current_player == PLAYER_HUMAN) {
        return GAME_WAITING_HUMAN;
    } else {
        return GAME_ENGINE_THINKING;
    }
}

game_state_type_t handle_engine_thinking_state(game_context_t *ctx) {
    printf("Engine is thinking...\n");
    
    // Set position and get move
    if (!uci_set_position(&ctx->engine, &ctx->chess)) {
        strcpy(ctx->status_message, "Failed to set position");
        return GAME_ERROR;
    }
    
    if (!uci_send_command(&ctx->engine, "go movetime 2000")) { // 2 second think time
        strcpy(ctx->status_message, "Failed to send go command");
        return GAME_ERROR;
    }
    
    char best_move[16];
    if (uci_get_best_move(&ctx->engine, best_move, sizeof(best_move))) {
        printf("Engine plays: %s\n", best_move);
        
        move_result_t result = make_move(&ctx->chess, best_move);
        if (result == MOVE_SUCCESS) {
            strcpy(ctx->last_move, best_move);
            snprintf(ctx->status_message, sizeof(ctx->status_message), 
                    "Engine played: %s", best_move);
            return GAME_PLAYING;
        } else {
            strcpy(ctx->status_message, "Engine made invalid move!");
            return GAME_ERROR;
        }
    } else {
        strcpy(ctx->status_message, "Engine failed to respond or game is over");
        ctx->game_over = true;
        ctx->winner = (ctx->chess.turn == WHITE) ? BLACK : WHITE;
        return GAME_GAME_OVER;
    }
}

game_state_type_t handle_waiting_human_state(game_context_t *ctx) {
    printf("%s to move. Enter your move (e.g., e2e4), 'help', 'history', or 'quit': ", 
           (ctx->chess.turn == WHITE) ? "White" : "Black");
    char input[64];
    if (!fgets(input, sizeof(input), stdin)) {
        return GAME_EXIT;
    }
    char *move = trim_string(input);
    if (strcmp(move, "quit") == 0) {
        return GAME_EXIT;
    }
    if (strcmp(move, "help") == 0) {
        printf("\nHow to play:\n");
        printf("- Enter moves in UCI format: e2e4, g1f3, etc.\n");
        printf("- For promotion, add piece: e7e8q (queen), e7e8r (rook), etc.\n");
        printf("- Castling: e1g1 (kingside), e1c1 (queenside)\n");
        printf("- Commands: 'help', 'history', 'quit'\n\n");
        return GAME_WAITING_HUMAN;
    }
    if (strcmp(move, "history") == 0) {
        print_move_history(&ctx->chess, 10);
        return GAME_WAITING_HUMAN;
    }
    move_result_t result = make_move(&ctx->chess, move);
    switch (result) {
        case MOVE_SUCCESS:
            strcpy(ctx->last_move, move);
            snprintf(ctx->status_message, sizeof(ctx->status_message), 
                    "You played: %s", move);
            return GAME_PLAYING;
        case MOVE_INVALID_FORMAT:
            strcpy(ctx->status_message, "Invalid move format. Use format like 'e2e4'");
            break;
        case MOVE_INVALID_SQUARE:
            strcpy(ctx->status_message, "Invalid square. Use a1-h8 format");
            break;
        case MOVE_NO_PIECE:
            strcpy(ctx->status_message, "No piece at source square");
            break;
        case MOVE_ILLEGAL:
            strcpy(ctx->status_message, "Illegal move");
            break;
        case MOVE_KING_IN_CHECK:
            strcpy(ctx->status_message, "Move would leave king in check");
            break;
        default:
            strcpy(ctx->status_message, "Move failed");
            break;
    }
    return GAME_WAITING_HUMAN;
}

game_state_type_t handle_game_over_state(game_context_t *ctx) {
    printf("\n=== GAME OVER ===\n");
    
    if (ctx->winner == COLOR_NONE) {
        printf("Game ended in a draw!\n");
    } else {
        printf("%s wins!\n", (ctx->winner == WHITE) ? "White" : "Black");
    }
    
    if (strlen(ctx->status_message) > 0) {
        printf("Reason: %s\n", ctx->status_message);
    }
    
    print_move_history(&ctx->chess, 10);
    
    printf("\nOptions:\n");
    printf("1. Play again\n");
    printf("2. Return to menu\n");
    printf("3. Exit\n");
    printf("Choose (1-3): ");
    
    int choice;
    if (scanf("%d", &choice) == 1) {
        clear_input_buffer();
        switch (choice) {
            case 1:
                init_chess_board(&ctx->chess);
                ctx->game_over = false;
                ctx->winner = COLOR_NONE;
                strcpy(ctx->status_message, "New game started");
                strcpy(ctx->last_move, "");
                return GAME_SETUP;
            case 2:
                init_chess_board(&ctx->chess);
                ctx->game_over = false;
                ctx->winner = COLOR_NONE;
                strcpy(ctx->status_message, "");
                strcpy(ctx->last_move, "");
                return GAME_MENU;
            case 3:
                return GAME_EXIT;
        }
    }
    
    clear_input_buffer();
    return GAME_EXIT;
}