#include "common/chess_types.h"
#include "ui/console_ui.h"
#include "vision/chess_move_detect.h"
#include <iostream>
#include <unistd.h>

int main() {

    std::string prev_image_path = "/home/lephuc/robotic-chess-playing-arm/reference_image/previous_w.png";
    std::string curr_image_path = "/home/lephuc/robotic-chess-playing-arm/reference_image/previous_b.png";
    std::string output_image_path = "/home/lephuc/robotic-chess-playing-arm/result.jpg";
    
    bool success = detect_chess_move(prev_image_path, curr_image_path, output_image_path);
    
    if (!success) {
        std::cerr << "Không thể xử lý ảnh!" << std::endl;
        return -1;
    }
    
    return 0;
}

/*
int main(void) {
    game_context_t ctx;
    init_game_context(&ctx);

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
                std::cout << "Error: " << ctx.status_message << "\n";
                std::cout << "Returning to menu...\n";
                usleep(2000000);
                ctx.state = GAME_MENU;
                break;
            default:
                std::cout << "Unknown state, exiting...\n";
                ctx.state = GAME_EXIT;
                break;
        }
    }

    cleanup_game_context(&ctx);
    std::cout << "\nThanks for playing!\n";
    return 0;
}
*/

