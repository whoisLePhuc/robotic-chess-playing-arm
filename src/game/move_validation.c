#include "game/move_validation.h"
#include "game/chess_state.h"

static bool is_path_clear(const chess_state_t *chess, int from_row, int from_col, int to_row, int to_col) {
    int row_step = (to_row > from_row) ? 1 : (to_row < from_row) ? -1 : 0;
    int col_step = (to_col > from_col) ? 1 : (to_col < from_col) ? -1 : 0;
    
    int current_row = from_row + row_step;
    int current_col = from_col + col_step;
    
    while (current_row != to_row || current_col != to_col) {
        if (chess->board[current_row][current_col].type != EMPTY) {
            return false;
        }
        current_row += row_step;
        current_col += col_step;
    }
    return true;
}

static bool is_valid_pawn_move(const chess_state_t *chess, int from_row, int from_col, int to_row, int to_col) {
    piece_t pawn = chess->board[from_row][from_col];
    if (pawn.type != PAWN) return false;
    
    int row_diff = to_row - from_row;
    int col_diff = abs(to_col - from_col);
    int direction = (pawn.color == WHITE) ? -1 : 1;
    
    // Forward move
    if (col_diff == 0) {
        if (row_diff == direction) {
            return chess->board[to_row][to_col].type == EMPTY;
        }
        if (row_diff == 2 * direction) {
            int start_row = (pawn.color == WHITE) ? 6 : 1;
            return (from_row == start_row && 
                    chess->board[to_row][to_col].type == EMPTY &&
                    chess->board[from_row + direction][from_col].type == EMPTY);
        }
    }
    // Diagonal capture
    else if (col_diff == 1 && row_diff == direction) {
        piece_t target = chess->board[to_row][to_col];
        if (target.type != EMPTY && target.color != pawn.color) {
            return true;
        }
        
        // En passant
        char target_square[3];
        index_to_square(to_row, to_col, target_square);
        return strcmp(chess->en_passant_target, target_square) == 0;
    }
    
    return false;
}

static bool is_valid_knight_move(int from_row, int from_col, int to_row, int to_col) {
    int row_diff = abs(to_row - from_row);
    int col_diff = abs(to_col - from_col);
    return (row_diff == 2 && col_diff == 1) || (row_diff == 1 && col_diff == 2);
}

static bool is_valid_bishop_move(const chess_state_t *chess, int from_row, int from_col, int to_row, int to_col) {
    int row_diff = abs(to_row - from_row);
    int col_diff = abs(to_col - from_col);
    return (row_diff == col_diff) && is_path_clear(chess, from_row, from_col, to_row, to_col);
}

static bool is_valid_rook_move(const chess_state_t *chess, int from_row, int from_col, int to_row, int to_col) {
    return (from_row == to_row || from_col == to_col) && 
           is_path_clear(chess, from_row, from_col, to_row, to_col);
}

static bool is_valid_queen_move(const chess_state_t *chess, int from_row, int from_col, int to_row, int to_col) {
    return is_valid_rook_move(chess, from_row, from_col, to_row, to_col) ||
           is_valid_bishop_move(chess, from_row, from_col, to_row, to_col);
}

static bool is_valid_king_move(const chess_state_t *chess, int from_row, int from_col, int to_row, int to_col) {
    int row_diff = abs(to_row - from_row);
    int col_diff = abs(to_col - from_col);
    
    // Normal king move
    if (row_diff <= 1 && col_diff <= 1) return true;
    
    // Castling
    if (row_diff == 0 && col_diff == 2) {
        piece_t king = chess->board[from_row][from_col];
        
        if (king.color == WHITE && from_row == 7) {
            if (to_col == 6) { // Kingside
                return chess->white_can_castle_kingside &&
                       chess->board[7][5].type == EMPTY &&
                       chess->board[7][6].type == EMPTY &&
                       chess->board[7][7].type == ROOK &&
                       !is_square_attacked(chess, 7, 4, BLACK) &&
                       !is_square_attacked(chess, 7, 5, BLACK) &&
                       !is_square_attacked(chess, 7, 6, BLACK);
            } else if (to_col == 2) { // Queenside
                return chess->white_can_castle_queenside &&
                       chess->board[7][1].type == EMPTY &&
                       chess->board[7][2].type == EMPTY &&
                       chess->board[7][3].type == EMPTY &&
                       chess->board[7][0].type == ROOK &&
                       !is_square_attacked(chess, 7, 4, BLACK) &&
                       !is_square_attacked(chess, 7, 3, BLACK) &&
                       !is_square_attacked(chess, 7, 2, BLACK);
            }
        } else if (king.color == BLACK && from_row == 0) {
            if (to_col == 6) { // Kingside
                return chess->black_can_castle_kingside &&
                       chess->board[0][5].type == EMPTY &&
                       chess->board[0][6].type == EMPTY &&
                       chess->board[0][7].type == ROOK &&
                       !is_square_attacked(chess, 0, 4, WHITE) &&
                       !is_square_attacked(chess, 0, 5, WHITE) &&
                       !is_square_attacked(chess, 0, 6, WHITE);
            } else if (to_col == 2) { // Queenside
                return chess->black_can_castle_queenside &&
                       chess->board[0][1].type == EMPTY &&
                       chess->board[0][2].type == EMPTY &&
                       chess->board[0][3].type == EMPTY &&
                       chess->board[0][0].type == ROOK &&
                       !is_square_attacked(chess, 0, 4, WHITE) &&
                       !is_square_attacked(chess, 0, 3, WHITE) &&
                       !is_square_attacked(chess, 0, 2, WHITE);
            }
        }
    }
    
    return false;
}

bool is_square_attacked(const chess_state_t *chess, int row, int col, color_t by_color) {
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            piece_t p = chess->board[r][c];
            if (p.type == EMPTY || p.color != by_color) continue;
            
            bool can_attack = false;
            switch (p.type) {
                case PAWN: {
                    int direction = (p.color == WHITE) ? -1 : 1;
                    can_attack = (row == r + direction && (col == c - 1 || col == c + 1));
                    break;
                }
                case KNIGHT:
                    can_attack = is_valid_knight_move(r, c, row, col);
                    break;
                case BISHOP:
                    can_attack = is_valid_bishop_move(chess, r, c, row, col);
                    break;
                case ROOK:
                    can_attack = is_valid_rook_move(chess, r, c, row, col);
                    break;
                case QUEEN:
                    can_attack = is_valid_queen_move(chess, r, c, row, col);
                    break;
                case KING: {
                    int row_diff = abs(row - r);
                    int col_diff = abs(col - c);
                    can_attack = (row_diff <= 1 && col_diff <= 1 && (row_diff || col_diff));
                    break;
                }
                default:
                    break;
            }
            
            if (can_attack) return true;
        }
    }
    return false;
}

bool is_king_in_check(const chess_state_t *chess, color_t king_color) {
    // Find king position
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            piece_t p = chess->board[r][c];
            if (p.type == KING && p.color == king_color) {
                color_t opponent = (king_color == WHITE) ? BLACK : WHITE;
                return is_square_attacked(chess, r, c, opponent);
            }
        }
    }
    return false;
}

static bool would_move_leave_king_in_check(chess_state_t *chess, const char *uci_move) {
    // Make temporary move
    chess_state_t temp = *chess;
    
    int from_row, from_col, to_row, to_col;
    square_to_index(uci_move, &from_row, &from_col);
    square_to_index(uci_move + 2, &to_row, &to_col);
    
    piece_t moving = temp.board[from_row][from_col];
    temp.board[to_row][to_col] = moving;
    temp.board[from_row][from_col] = (piece_t){EMPTY, COLOR_NONE};
    
    return is_king_in_check(&temp, chess->turn);
}

bool is_legal_move(const chess_state_t *chess, const char *uci_move) {
    if (!chess || !uci_move || strlen(uci_move) < 4) return false;
    
    int from_row, from_col, to_row, to_col;
    if (!square_to_index(uci_move, &from_row, &from_col)) return false;
    if (!square_to_index(uci_move + 2, &to_row, &to_col)) return false;
    
    piece_t moving = chess->board[from_row][from_col];
    if (moving.type == EMPTY || moving.color != chess->turn) return false;
    
    piece_t target = chess->board[to_row][to_col];
    if (target.type != EMPTY && target.color == moving.color) return false;
    
    // Piece-specific validation
    bool valid = false;
    switch (moving.type) {
        case PAWN:
            valid = is_valid_pawn_move(chess, from_row, from_col, to_row, to_col);
            break;
        case KNIGHT:
            valid = is_valid_knight_move(from_row, from_col, to_row, to_col);
            break;
        case BISHOP:
            valid = is_valid_bishop_move(chess, from_row, from_col, to_row, to_col);
            break;
        case ROOK:
            valid = is_valid_rook_move(chess, from_row, from_col, to_row, to_col);
            break;
        case QUEEN:
            valid = is_valid_queen_move(chess, from_row, from_col, to_row, to_col);
            break;
        case KING:
            valid = is_valid_king_move(chess, from_row, from_col, to_row, to_col);
            break;
        default:
            return false;
    }
    
    if (!valid) return false;
    
    // Check if move would leave king in check
    return !would_move_leave_king_in_check((chess_state_t*)chess, uci_move);
}

static bool has_legal_moves(const chess_state_t *chess, color_t color) {
    char move[6];
    
    for (int fr = 0; fr < BOARD_SIZE; fr++) {
        for (int fc = 0; fc < BOARD_SIZE; fc++) {
            piece_t piece = chess->board[fr][fc];
            if (piece.type == EMPTY || piece.color != color) continue;
            
            for (int tr = 0; tr < BOARD_SIZE; tr++) {
                for (int tc = 0; tc < BOARD_SIZE; tc++) {
                    char from_sq[3], to_sq[3];
                    index_to_square(fr, fc, from_sq);
                    index_to_square(tr, tc, to_sq);
                    snprintf(move, sizeof(move), "%s%s", from_sq, to_sq);
                    
                    if (is_legal_move(chess, move)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool is_checkmate(const chess_state_t *chess) {
    return is_king_in_check(chess, chess->turn) && !has_legal_moves(chess, chess->turn);
}

bool is_stalemate(const chess_state_t *chess) {
    return !is_king_in_check(chess, chess->turn) && !has_legal_moves(chess, chess->turn);
}

move_result_t make_move(chess_state_t *chess, const char *uci_move) {
    if (!chess || !uci_move) return MOVE_INVALID_FORMAT;
    if (strlen(uci_move) < 4) return MOVE_INVALID_FORMAT;
    
    if (!is_legal_move(chess, uci_move)) {
        return MOVE_ILLEGAL;
    }
    
    int from_row, from_col, to_row, to_col;
    square_to_index(uci_move, &from_row, &from_col);
    square_to_index(uci_move + 2, &to_row, &to_col);
    
    piece_t moving = chess->board[from_row][from_col];
    piece_t captured = chess->board[to_row][to_col];
    
    // Record move in history
    move_t *move = &chess->move_history[chess->move_count];
    strcpy(move->notation, uci_move);
    move->moved_piece = moving;
    move->captured_piece = captured;
    move->from_row = from_row;
    move->from_col = from_col;
    move->to_row = to_row;
    move->to_col = to_col;
    move->is_castle = false;
    move->is_en_passant = false;
    move->is_promotion = false;
    
    // Execute move
    chess->board[to_row][to_col] = moving;
    chess->board[from_row][from_col] = (piece_t){EMPTY, COLOR_NONE};
    
    // Handle special moves
    // Castling
    if (moving.type == KING && abs(to_col - from_col) == 2) {
        move->is_castle = true;
        if (to_col == 6) { // Kingside
            chess->board[from_row][5] = chess->board[from_row][7];
            chess->board[from_row][7] = (piece_t){EMPTY, COLOR_NONE};
        } else { // Queenside
            chess->board[from_row][3] = chess->board[from_row][0];
            chess->board[from_row][0] = (piece_t){EMPTY, COLOR_NONE};
        }
    }
    
    // En passant
    char target_square[3];
    index_to_square(to_row, to_col, target_square);
    if (moving.type == PAWN && strcmp(chess->en_passant_target, target_square) == 0) {
        move->is_en_passant = true;
        int captured_row = (moving.color == WHITE) ? to_row + 1 : to_row - 1;
        chess->board[captured_row][to_col] = (piece_t){EMPTY, COLOR_NONE};
    }
    
    // Promotion
    if (strlen(uci_move) >= 5 && moving.type == PAWN) {
        move->is_promotion = true;
        move->promotion_piece = uci_move[4];
        
        piece_type_t promo_type = QUEEN;
        switch (tolower((unsigned char)uci_move[4])) {
            case 'q': promo_type = QUEEN; break;
            case 'r': promo_type = ROOK; break;
            case 'b': promo_type = BISHOP; break;
            case 'n': promo_type = KNIGHT; break;
        }
        chess->board[to_row][to_col].type = promo_type;
    }
    
    // Update en passant target
    strcpy(chess->en_passant_target, "-");
    if (moving.type == PAWN && abs(to_row - from_row) == 2) {
        index_to_square((from_row + to_row) / 2, from_col, chess->en_passant_target);
    }
    
    // Update castling rights
    if (moving.type == KING) {
        if (moving.color == WHITE) {
            chess->white_can_castle_kingside = false;
            chess->white_can_castle_queenside = false;
        } else {
            chess->black_can_castle_kingside = false;
            chess->black_can_castle_queenside = false;
        }
    } else if (moving.type == ROOK) {
        if (moving.color == WHITE) {
            if (from_row == 7 && from_col == 0) chess->white_can_castle_queenside = false;
            if (from_row == 7 && from_col == 7) chess->white_can_castle_kingside = false;
        } else {
            if (from_row == 0 && from_col == 0) chess->black_can_castle_queenside = false;
            if (from_row == 0 && from_col == 7) chess->black_can_castle_kingside = false;
        }
    }
    
    // Update halfmove clock
    if (moving.type == PAWN || captured.type != EMPTY) {
        chess->halfmove_clock = 0;
    } else {
        chess->halfmove_clock++;
    }
    
    // Update turn and move count
    chess->turn = (chess->turn == WHITE) ? BLACK : WHITE;
    if (chess->turn == WHITE) chess->fullmove_number++;
    chess->move_count++;
    
    return MOVE_SUCCESS;
}