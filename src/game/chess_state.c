#include "game/chess_state.h"

void init_chess_board(chess_state_t *chess) {
    if (!chess) return;
    
    memset(chess, 0, sizeof(chess_state_t));
    
    // Clear board
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            chess->board[r][c].type = EMPTY;
            chess->board[r][c].color = COLOR_NONE;
        }
    }
    
    // Setup pawns
    for (int c = 0; c < BOARD_SIZE; c++) {
        chess->board[1][c] = (piece_t){PAWN, BLACK};
        chess->board[6][c] = (piece_t){PAWN, WHITE};
    }
    
    // Setup major pieces
    piece_type_t back_row[8] = {ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK};
    for (int c = 0; c < BOARD_SIZE; c++) {
        chess->board[0][c] = (piece_t){back_row[c], BLACK};
        chess->board[7][c] = (piece_t){back_row[c], WHITE};
    }
    
    // Initial game state
    chess->turn = WHITE;
    chess->white_can_castle_kingside = true;
    chess->white_can_castle_queenside = true;
    chess->black_can_castle_kingside = true;
    chess->black_can_castle_queenside = true;
    strcpy(chess->en_passant_target, "-");
    chess->fullmove_number = 1;
    chess->halfmove_clock = 0;
    chess->move_count = 0;
}

bool square_to_index(const char *square, int *row, int *col) {
    if (!square || !row || !col || strlen(square) < 2) return false;
    
    char file = square[0];
    char rank = square[1];
    
    if (file >= 'A' && file <= 'H') file = (char)(file - 'A' + 'a');
    if (file < 'a' || file > 'h') return false;
    if (rank < '1' || rank > '8') return false;
    
    *col = file - 'a';
    *row = 8 - (rank - '0');
    return true;
}

void index_to_square(int row, int col, char *square) {
    if (!square || row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        if (square) strcpy(square, "??");
        return;
    }
    square[0] = (char)('a' + col);
    square[1] = (char)('8' - row);
    square[2] = '\0';
}

char piece_to_char(piece_t piece) {
    if (piece.type == EMPTY) return '.';
    
    char ch;
    switch (piece.type) {
        case PAWN: ch = 'P'; break;
        case KNIGHT: ch = 'N'; break;
        case BISHOP: ch = 'B'; break;
        case ROOK: ch = 'R'; break;
        case QUEEN: ch = 'Q'; break;
        case KING: ch = 'K'; break;
        default: ch = '?'; break;
    }
    
    return (piece.color == BLACK) ? (char)tolower((unsigned char)ch) : ch;
}

void chess_state_to_fen(const chess_state_t *chess, char *fen_buffer, size_t buffer_size) {
    if (!chess || !fen_buffer) return;
    
    char fen[256] = "";
    
    // Board position
    for (int r = 0; r < BOARD_SIZE; r++) {
        int empty_count = 0;
        for (int c = 0; c < BOARD_SIZE; c++) {
            piece_t p = chess->board[r][c];
            if (p.type == EMPTY) {
                empty_count++;
            } else {
                if (empty_count > 0) {
                    char count_str[2] = {'0' + empty_count, '\0'};
                    strcat(fen, count_str);
                    empty_count = 0;
                }
                char piece_char = piece_to_char(p);
                char piece_str[2] = {piece_char, '\0'};
                strcat(fen, piece_str);
            }
        }
        if (empty_count > 0) {
            char count_str[2] = {'0' + empty_count, '\0'};
            strcat(fen, count_str);
        }
        if (r < 7) strcat(fen, "/");
    }
    
    strcat(fen, " ");
    strcat(fen, (chess->turn == WHITE) ? "w" : "b");
    strcat(fen, " ");
    
    // Castling rights
    char castling[5] = "";
    if (chess->white_can_castle_kingside) strcat(castling, "K");
    if (chess->white_can_castle_queenside) strcat(castling, "Q");
    if (chess->black_can_castle_kingside) strcat(castling, "k");
    if (chess->black_can_castle_queenside) strcat(castling, "q");
    if (strlen(castling) == 0) strcat(castling, "-");
    strcat(fen, castling);
    
    strcat(fen, " ");
    strcat(fen, chess->en_passant_target);
    
    char counters[20];
    snprintf(counters, sizeof(counters), " %d %d", chess->halfmove_clock, chess->fullmove_number);
    strcat(fen, counters);
    
    strncpy(fen_buffer, fen, buffer_size - 1);
    fen_buffer[buffer_size - 1] = '\0';
}