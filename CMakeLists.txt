cmake_minimum_required(VERSION 3.10)

project(RobotPlayChessProject VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
    src/engine/uci_engine.c
    src/game/chess_state.c
    src/game/move_validation.c
    src/ui/board_display.c
    src/ui/console_ui.c
    src/utils/string_utils.c
    main.c
)

add_executable(robot_play_chess ${PROJECT_SOURCES})

target_include_directories(robot_play_chess
    PRIVATE
        inc
)

target_compile_options(robot_play_chess
    PRIVATE
        -Wall -Wextra -Wpedantic
)

target_link_libraries(robot_play_chess
    PRIVATE
)