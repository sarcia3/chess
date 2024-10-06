#ifndef BOARD_H
#define BOARD_H

#include "bitboard.h"
#include "board_utils.h"
#include <stack>
#include <map>
#include <vector>
#include <functional>
#include <array>
#include <bit>
#include <iostream>
#include <cmath> 
#include <set>

using namespace std;


class board {
    private:
        array<bitboard, 12> is_piece;
        bool white_short_castle;
        bool white_long_castle;
        bool black_short_castle;
        bool black_long_castle;
        int ply_100;
        int ply;
        bool turn;
        pair<int, int> en_pessant;
        
        bitboard &white_pawn;
        bitboard &white_knight;
        bitboard &white_bishop;
        bitboard &white_rook;
        bitboard &white_queen;
        bitboard &white_king;
        bitboard &black_pawn;
        bitboard &black_knight;
        bitboard &black_bishop;
        bitboard &black_rook;
        bitboard &black_queen;
        bitboard &black_king;

        bitboard is_anything;
        array<bitboard, 2> is_color;

        bitboard &is_white;
        bitboard &is_black;

        board();
        board(const board& to_copy);

        void update_is_anything_color();

        bitboard gen_attacked(int gen_turn);
        stack<pair<int, int>> gen_moves();

        bool is_legal();

        void make_move(const pair<int, int> &move);
        void make_move(const pair<int, int> &start, const pair<int, int> &end);


    public: 
        enum game_state {undecided, white_won, draw_3_fold, draw_50_rule, draw_stalemate, black_won };
        
        game_state current_state;

        void update_state();

        set<string> print_moves();

        void user_move(set<string> legal);

        board (const string &fen);

        void print_board();
};
#endif
