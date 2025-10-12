#include "common/chess_types.h"
#include "ui/console_ui.h"

int main(void) {
    game_context_t ctx;
    init_game_context(&ctx);
    
    // Main state machine loop
    while (ctx.state != GAME_EXIT) {
        switch (ctx.state) {
            case GAME_MENU:
                ctx.state = handle_menu_state(&ctx);
                break;
            case GAME_SETUP:
                ctx.state = handle_setup_state(&ctx);
                break;
            case GAME_PLAYING:
                ctx.state = handle_playing_state(&ctx);
                break;
            case GAME_ENGINE_THINKING:
                ctx.state = handle_engine_thinking_state(&ctx);
                break;
            case GAME_WAITING_HUMAN:
                ctx.state = handle_waiting_human_state(&ctx);
                break;
            case GAME_GAME_OVER:
                ctx.state = handle_game_over_state(&ctx);
                break;
            case GAME_ERROR:
                printf("Error: %s\n", ctx.status_message);
                printf("Returning to menu...\n");
                usleep(2000000); // 2 seconds
                ctx.state = GAME_MENU;
                break;
            default:
                printf("Unknown state, exiting...\n");
                ctx.state = GAME_EXIT;
                break;
        }
    }
    cleanup_game_context(&ctx);
    printf("\nThanks for playing!\n");
    return 0;
}