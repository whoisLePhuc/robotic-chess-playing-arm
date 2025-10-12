cmake_minimum_required(VERSION 3.10)
project(RobotPlayChessProject VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

set(PROJECT_SOURCES
    src/engine/uci_engine.c
    src/game/chess_state.c
    src/game/move_validation.c
    src/ui/board_display.c
    src/ui/console_ui.c
    src/utils/string_utils.c
    src/vision/chess_move_detect.cpp
    main.cpp
)

add_executable(robot_play_chess ${PROJECT_SOURCES})

target_include_directories(robot_play_chess
    PRIVATE
        inc
        ${OpenCV_INCLUDE_DIRS}
)

target_compile_options(robot_play_chess
    PRIVATE
        -Wall -Wextra -Wpedantic
)

target_link_libraries(robot_play_chess
    PRIVATE
        ${OpenCV_LIBS}
)