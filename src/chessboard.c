// chessboard_helpers.c
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "chessboard.h" // giả sử định nghĩa piece, piece_type, color...

void init_chessboard(piece board[8][8]) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            board[r][c].type = EMPTY;
            board[r][c].color = NONE;
        }
    }

    // Pawns
    for (int c = 0; c < 8; c++) {
        board[1][c].type = PAWN;
        board[1][c].color = BLACK;
        board[6][c].type = PAWN;
        board[6][c].color = WHITE;
    }

    // Major pieces
    piece_type order[8] = { ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK };

    for (int c = 0; c < 8; c++) {
        board[0][c].type = order[c];
        board[0][c].color = BLACK;
        board[7][c].type = order[c];
        board[7][c].color = WHITE;
    }
}

static char piece_to_char(piece p) {
    if (p.type == EMPTY) return '.';
    char ch;
    switch (p.type) {
        case PAWN:   ch = 'P'; break;
        case KNIGHT: ch = 'N'; break;
        case BISHOP: ch = 'B'; break;
        case ROOK:   ch = 'R'; break;
        case QUEEN:  ch = 'Q'; break;
        case KING:   ch = 'K'; break;
        default:     ch = '?'; break;
    }
    if (p.color == BLACK) ch = (char)tolower((unsigned char)ch);
    return ch;
}

void print_chessboard(piece board[8][8]) {
    for (int r = 0; r < 8; r++) {
        printf("%d ", 8 - r);
        for (int c = 0; c < 8; c++) {
            printf("%c ", piece_to_char(board[r][c]));
        }
        printf("\n");
    }
    printf("  a b c d e f g h\n");
}

/*
 * Chú ý: hàm này chỉ đọc 2 ký tự đầu của 'square'.
 * Nó chấp nhận:
 *  - "e4"
 *  - "E4" (chữ hoa)
 *  - "e4..." (chuỗi dài hơn, ví dụ "e8q") — chỉ đọc hai ký tự đầu.
 */
static int square_to_index(const char *square, int *row, int *col) {
    if (!square || !row || !col) return 0;
    if (square[0] == '\0' || square[1] == '\0') return 0; // đảm bảo có ít nhất 2 ký tự

    char file = square[0];
    char rank = square[1];

    // cho phép chữ hoa
    if (file >= 'A' && file <= 'H') file = (char)(file - 'A' + 'a');

    if (file < 'a' || file > 'h') return 0;
    if (rank < '1' || rank > '8') return 0;

    *col = file - 'a';
    *row = 8 - (rank - '0'); // '1' -> 7, '8' -> 0
    return 1;
}

/*
 * make_move: nhận board và chuỗi UCI (ví dụ "e2e4" hoặc "e7e8q")
 * Trả về 1 nếu thành công, 0 nếu lỗi.
 * Lưu ý: hàm chỉ thực hiện cập nhật trạng thái bảng (move + capture + promotion).
 * Không kiểm tra tính hợp lệ luật chơi (legal move), không xử lý castling/en-passant.
 */
int make_move(piece board[8][8], const char *uci_move) {
    if (!uci_move) return 0;
    size_t len = strlen(uci_move);
    if (len < 4) {
        printf("Lỗi: nước đi quá ngắn!\n");
        return 0;
    }

    int fromRow, fromCol, toRow, toCol;
    if (!square_to_index(uci_move, &fromRow, &fromCol)) {
        printf("Lỗi: ô nguồn không hợp lệ: '%.2s'\n", uci_move);
        return 0;
    }
    if (!square_to_index(uci_move + 2, &toRow, &toCol)) {
        printf("Lỗi: ô đích không hợp lệ: '%.2s'\n", uci_move + 2);
        return 0;
    }

    piece moving = board[fromRow][fromCol];
    if (moving.type == EMPTY) {
        printf("Lỗi: không có quân nào ở ô nguồn!\n");
        return 0;
    }

    // Xử lý promotion (ví dụ "e7e8q")
    piece_type promoteTo = EMPTY; // dùng EMPTY như sentinel (có thể đổi tên rõ ràng hơn)
    if (len >= 5) {
        char promo = (char) tolower((unsigned char)uci_move[4]);
        switch (promo) {
            case 'q': promoteTo = QUEEN; break;
            case 'r': promoteTo = ROOK;  break;
            case 'b': promoteTo = BISHOP;break;
            case 'n': promoteTo = KNIGHT;break;
            default: promoteTo = EMPTY;  break;
        }
        // kiểm tra sơ bộ: promotion chỉ hợp lý nếu quân là Pawn và tới hàng cuối
        if (promoteTo != EMPTY) {
            if (moving.type != PAWN) {
                printf("Cảnh báo: promotion nhưng quân nguồn không phải Pawn.\n");
                // ta vẫn cho phép nhưng bạn có thể return 0 nếu muốn chặt chẽ  
            }
            if (!((moving.color == WHITE && toRow == 0) || (moving.color == BLACK && toRow == 7))) {
                printf("Cảnh báo: promotion nhưng ô đích không phải hàng cuối.\n");
            }
        }
    }

    // Thực hiện di chuyển (ghi đè đích, xóa nguồn)
    board[toRow][toCol] = moving;

    if (promoteTo != EMPTY) {
        board[toRow][toCol].type = promoteTo;
    }

    board[fromRow][fromCol].type = EMPTY;
    board[fromRow][fromCol].color = NONE;

    return 1;
}
