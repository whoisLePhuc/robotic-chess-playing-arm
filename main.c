/*
#include <stdio.h>
#include <stdlib.h>
#include "chessboard.h"

int main() {
    piece board[8][8];   // Coordinates (x,y) from left upper corner
    init_chessboard(board);
    print_chessboard(board);

    printf("\nĐi e2e4:\n");
    make_move(board, "e2e4");
    print_chessboard(board);

    printf("\nĐi e4c4:\n");
    make_move(board, "e4c4");
    print_chessboard(board);
    return 0;
}
*/


#include "chess_logic.h"
#include <stdio.h>

int main() {
    if (uci_start("stockfish") != 0) {
        printf("Không mở được Stockfish!\n");
        return 1;
    }

    uci_send("uci");
    uci_read_bestmove();   // chờ "uciok"

    uci_newgame();

    // Đặt vị trí ban đầu và cho engine tính
    uci_send("position startpos moves e2e4");
    uci_send("go depth 10");
    const char *bm = uci_read_bestmove();

    if (bm) {
        printf("Bestmove từ Stockfish: %s\n", bm);
    }

    uci_stop();
    return 0;
}
