#include "board.h"
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
using namespace board_utils;

board::board() 
    : is_piece{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      white_pawn(is_piece[0]),
      white_knight(is_piece[1]), 
      white_bishop(is_piece[2]), 
      white_rook(is_piece[3]), 
      white_queen(is_piece[4]), 
      white_king(is_piece[5]), 
      black_pawn(is_piece[6]), 
      black_knight(is_piece[7]), 
      black_bishop(is_piece[8]), 
      black_rook(is_piece[9]), 
      black_queen(is_piece[10]), 
      black_king(is_piece[11]),
      is_anything(0), 
      is_color{0, 0},
      is_white(is_color[0]), 
      is_black(is_color[1]),
      current_state(undecided)
{
    white_short_castle = true;
    white_long_castle  = true;
    black_short_castle = true;
    black_long_castle  = true;
    ply_100 = 0;
    ply = 0;
    turn = 0;
    en_pessant = {-1, -1};
}

board::board(const board& to_copy) 
    : is_piece(to_copy.is_piece),
      white_pawn(is_piece[0]),
      white_knight(is_piece[1]), 
      white_bishop(is_piece[2]), 
      white_rook(is_piece[3]), 
      white_queen(is_piece[4]), 
      white_king(is_piece[5]), 
      black_pawn(is_piece[6]), 
      black_knight(is_piece[7]), 
      black_bishop(is_piece[8]), 
      black_rook(is_piece[9]), 
      black_queen(is_piece[10]), 
      black_king(is_piece[11]),
      is_anything(to_copy.is_anything),
      is_color(to_copy.is_color),
      is_white(is_color[0]),
      is_black(is_color[1]),
      current_state(to_copy.current_state)
{
    white_short_castle = to_copy.white_short_castle;
    white_long_castle = to_copy.white_long_castle;
    black_short_castle = to_copy.black_short_castle;
    black_long_castle = to_copy.black_long_castle;
    ply_100 = to_copy.ply_100;
    ply = to_copy.ply;
    turn = to_copy.turn;
    en_pessant = to_copy.en_pessant;
}

void board::update_is_anything_color() {
    is_white = 0;

    for(int i=0; i<6; i++)
        is_white |= is_piece[i];

    is_black = 0;

    for(int i=6; i<12; i++)
        is_black |= is_piece[i];

    is_anything = is_white | is_black;
}

bitboard board::gen_attacked(int gen_turn) {
    stack<pair<int, int>> S;

    if(gen_turn == 0) {
        for(int i=0; i<64; i++)
            if(white_pawn[i]) {
                auto [row, column] = gen_coordinate(i);

                S.push({row+1, column-1});
                S.push({row+1, column+1});
            }
    } else {
        for(int i=0; i<64; i++)
            if(black_pawn[i]) {
                auto [row, column] = gen_coordinate(i);

                S.push({row-1, column-1});
                S.push({row-1, column+1});
            }
    }

    bitboard &turn_knight = is_piece[1 + 6 * gen_turn];
    bitboard &turn_bishop = is_piece[2 + 6 * gen_turn];
    bitboard &turn_rook   = is_piece[3 + 6 * gen_turn];
    bitboard &turn_queen  = is_piece[4 + 6 * gen_turn];
    bitboard &turn_king   = is_piece[5 + 6 * gen_turn];

    for(int i=0; i<64; i++) 
        if(turn_knight[i]) {
                auto [row, column] = gen_coordinate(i);

                for(int dir1=-1; dir1<2; dir1+=2)
                    for(int dir2=-1; dir2<2; dir2+=2) {
                        S.push({row + 2 * dir1, column + 1 * dir2});
                        S.push({row + 1 * dir1, column + 2 * dir2});
                    }
        }

    pair<int, int> direction;
    function<void(pair<int, int>)> go_into = [&](pair<int, int> coordinate) {
        coordinate.first += direction.first;
        coordinate.second += direction.second;
        if(coordinate_is_legal(coordinate)){
            S.push(coordinate);
            if(!((is_anything >> ind_from_coordinate(coordinate))&1))
                go_into(coordinate);
        }
    };

    for(int i=0; i<64; i++) {

        if(turn_bishop[i]) {
            for(direction.first = -1; direction.first<2; direction.first+=2)
                for(direction.second = -1; direction.second<2; direction.second+=2)
                    go_into(gen_coordinate(i));
        }

        if(turn_rook[i]) {
            for(direction.first = -1; direction.first<2; direction.first++)
                for(direction.second = -1; direction.second<2; direction.second++)
                    if(((bool)direction.first) ^ ((bool)direction.second))
                        go_into(gen_coordinate(i));
        }

        if(turn_queen[i]) {
            for(direction.first = -1; direction.first<2; direction.first++)
                for(direction.second = -1; direction.second<2; direction.second++)
                    if(direction.first != 0 || direction.second != 0)
                        go_into(gen_coordinate(i));
        }
    }

    bitboard res = 0;

    while(S.size()) {
        if(coordinate_is_legal(S.top())) {
            int i = ind_from_coordinate(S.top());
            res.set_val(true, i);
        }
        S.pop();
    }

    return res;
}

bool board::is_legal(){
    //1st check - is every square occupied by exactly zero or one piece
    bitboard current = 0;
    for(auto &elem : is_piece){
        if(current & elem)
            return false;
        else current |= elem;
    }

    //2nd check - pawns on first and last ranks
    if(0xFF000000000000FF & (white_pawn | black_pawn))
        return false;

    
    //3rd check - exactly one king of each color exists 
    if(popcount(white_king) != 1 || popcount(black_king) != 1)
        return false;


    //4th check - king not in check
    if(gen_attacked(turn) & is_piece[11 - 6*turn])
        return false;

    return true;
};

stack<pair<int, int>> board::gen_moves() {
    stack<pair<int, int>> S;

    auto pawn_push = [&](int start, int end){
        if(end/8 == 0 || end/8 == 7){
            end<<=2;
            S.push({-start, end});
            S.push({-start, end+1});
            S.push({-start, end+2});
            S.push({-start, end+3});
            //negative start to signify pawn promotion, last two bits of end representing
            //the new piece 0 - knight, 1 - bishop, 2 - rook, 3 - queen
        } else {
            S.push({start, end});
        }
    };

    if(turn == 0) {
        for(int i=0; i<64; i++)
            if(white_pawn[i]) {
                auto [row, column] = gen_coordinate(i);
                if(coordinate_is_legal({row+1, column-1}) && 
                    (is_black[ind_from_coordinate({row+1, column-1})]) || (en_pessant.first == row+1 && en_pessant.second == column-1))
                        pawn_push(i, ind_from_coordinate({row+1, column-1}));
                if(coordinate_is_legal({row+1, column+1}) && 
                    (is_black[ind_from_coordinate({row+1, column+1})] || (en_pessant.first == row+1 && en_pessant.second == column+1)))
                        pawn_push(i, ind_from_coordinate({row+1, column+1}));
                
                if(!is_anything[i+8]) {
                    pawn_push(i, i+8);
                    if(row==1 && !is_anything[i+16])
                        S.push({i, i+16}); // pawn push useless, thus omited
                }

                
            }
    } else {
        for(int i=0; i<64; i++)
            if(black_pawn[i]) {
                auto [row, column] = gen_coordinate(i);
                if(coordinate_is_legal({row-1, column-1}) && 
                    (is_white[ind_from_coordinate({row-1, column-1})] || (en_pessant.first == row-1 && en_pessant.second == column-1)))
                        pawn_push(i, ind_from_coordinate({row-1, column-1}));
                if(coordinate_is_legal({row-1, column+1}) && 
                    (is_white[ind_from_coordinate({row-1, column+1})] || (en_pessant.first == row-1 && en_pessant.second == column+1)))
                        pawn_push(i, ind_from_coordinate({row-1, column+1}));
                
                if(!is_anything[i-8]) {
                    pawn_push(i, ind_from_coordinate({row-1, column}));
                    if(row==6 && !is_anything[i-16])
                        S.push({i, i-16}); // pawn push useless, thus omited
                }
            }
    }

    bitboard &turn_knight = is_piece[1 + 6 * turn];
    bitboard &turn_bishop = is_piece[2 + 6 * turn];
    bitboard &turn_rook   = is_piece[3 + 6 * turn];
    bitboard &turn_queen  = is_piece[4 + 6 * turn];
    bitboard &turn_king   = is_piece[5 + 6 * turn];

    for(int i=0; i<64; i++) 
        if(turn_knight[i]) {
                auto [row, column] = gen_coordinate(i);

                for(int ska1=-1; ska1<2; ska1+=2)
                    for(int ska2=-1; ska2<2; ska2+=2) {
                        if(coordinate_is_legal({row + 2 * ska1, column + 1 * ska2}) && 
                            !is_color[turn][ind_from_coordinate({row + 2 * ska1, column + 1 * ska2})])
                                S.push({i, ind_from_coordinate({row + 2 * ska1, column + 1 * ska2})});
                        if(coordinate_is_legal({row + 1 * ska1, column + 2 * ska2}) && 
                            !is_color[turn][ind_from_coordinate({row + 1 * ska1, column + 2 * ska2})])
                                S.push({i, ind_from_coordinate({row + 1 * ska1, column + 2 * ska2})});
                    }
        }

    for(int i=0; i<64; i++){
        if(turn_king[i]){
            auto [row, column] = gen_coordinate(i);
            for(int dirx=-1; dirx<2; dirx++)
                for(int diry=-1; diry<2; diry++)
                    if(coordinate_is_legal({row + diry, column + dirx}) && 
                        !is_color[turn][ind_from_coordinate({row + diry, column + dirx})])
                            S.push({i, ind_from_coordinate({row + diry, column + dirx})});

            break;
        }
    }
    int i;
    pair<int, int> direction;
    function<void(pair<int, int>)> go_into = [&](pair<int, int> coordinate) {
        coordinate.first += direction.first;
        coordinate.second += direction.second;
        if(coordinate_is_legal(coordinate) && !is_color[turn][ind_from_coordinate(coordinate)]){
            S.push({i, ind_from_coordinate(coordinate)});
            if(!is_anything[ind_from_coordinate(coordinate)])
                go_into(coordinate);
        }
    };

    for(i=0; i<64; i++) {

        if(turn_bishop[i]) {
            for(direction.first = -1; direction.first<2; direction.first+=2)
                for(direction.second = -1; direction.second<2; direction.second+=2)
                    go_into(gen_coordinate(i));
        }

        if(turn_rook[i]) {
            for(direction.first = -1; direction.first<2; direction.first++)
                for(direction.second = -1; direction.second<2; direction.second++)
                    if(((bool) direction.first) ^ ((bool)direction.second))
                        go_into(gen_coordinate(i));
        }

        if(turn_queen[i]) {
            for(direction.first = -1; direction.first<2; direction.first++)
                for(direction.second = -1; direction.second<2; direction.second++)
                    if(direction.first != 0 || direction.second != 0)
                        go_into(gen_coordinate(i));
        }
    }

    stack<pair<int, int>> res;

    if(turn == 0) {
        if(white_short_castle && !((is_anything | gen_attacked(!turn)) & (96ULL))){
            res.push({0, 0});
        }
        if(white_long_castle && !((is_anything | gen_attacked(!turn)) & (14ULL))){
            res.push({1, 1});
        }
    } else {
        if(black_short_castle && !((is_anything | gen_attacked(!turn)) & (0x6000000000000000ULL))){
            res.push({2, 2});
        }
        if(black_long_castle && !((is_anything | gen_attacked(!turn)) & (0xE00000000000000ULL))){
            res.push({3, 3});
        }
    }

    while(S.size()){
        auto top = S.top(); S.pop();
        board copy(*this);
        copy.make_move(top);
        if(copy.is_legal()) res.push(top);
    }

    if(ply_100 == 100) {current_state = draw_50_rule; return {};}
    if(turn == 0 && res.empty()) {if(white_king & gen_attacked(!turn)) {current_state = black_won; return {};}}
    if(turn == 1 && res.empty()) {if(black_king & gen_attacked(!turn)) {current_state = white_won; return {};}}
    if(res.empty()) {current_state == draw_stalemate; return {};}

    return res;
}

void board::make_move(const pair<int, int> &move){
    auto [start, end] = move;
    if(start == 0 && end == 0) {
        white_king.set_val(false, ind_from_coordinate({0, 4}));
        white_king.set_val(true, ind_from_coordinate({0, 6}));
        white_rook.set_val(false, ind_from_coordinate({0, 7}));
        white_rook.set_val(true, ind_from_coordinate({0, 5}));
        white_short_castle = false;
    }
    if(start == 1 && end == 1) {
        white_king.set_val(false, ind_from_coordinate({0, 4}));
        white_king.set_val(true, ind_from_coordinate({0, 2}));
        white_rook.set_val(false, ind_from_coordinate({0, 0}));
        white_rook.set_val(true, ind_from_coordinate({0, 3}));
        white_long_castle = false;
    }
    if(start == 2 && end == 2) {
        black_king.set_val(false, ind_from_coordinate({7, 4}));
        black_king.set_val(true, ind_from_coordinate({7, 6}));
        black_rook.set_val(false, ind_from_coordinate({7, 7}));
        black_rook.set_val(true, ind_from_coordinate({7, 5}));
        black_short_castle = false;
    }
    if(start == 3 && end == 3) {
        black_king.set_val(false, ind_from_coordinate({7, 4}));
        black_king.set_val(true, ind_from_coordinate({7, 2}));
        black_rook.set_val(false, ind_from_coordinate({7, 0}));
        black_rook.set_val(true, ind_from_coordinate({7, 3}));
        black_long_castle = false;
    }
    if(start == end) {
        ply_100 = 0;
        ply++;
        turn ^= 1;
        en_pessant = {-1, -1};
        update_is_anything_color();
        return;
    }


    if(start < 0) { //promotion AHHHH
        start *= -1;
        int prom_type = end%4;
        end>>=2;

        auto [start_row, start_col] = gen_coordinate(start);
        auto [end_row, end_col] = gen_coordinate(end);

        if(end == 4) white_short_castle = white_long_castle = false;
        if(end == 60) black_short_castle = black_long_castle = false;
        if(end == 0) white_short_castle = false;
        if(end == 7) white_long_castle = false;
        if(end == 56) black_short_castle = false;
        if(end == 63) black_long_castle = false;

        en_pessant = {-1, -1};

        is_piece[turn*6].set_val(false, start);
        for(auto &elem : is_piece) if(elem[end]) { elem.set_val(false, end); ply_100 = -1; }
        is_piece[turn*6 + 1 + prom_type].set_val(1, end);
        ply_100 = 0;
        ply++;
        turn^=1;
        update_is_anything_color();
        return;
    }

    bitboard tmp1(1ULL << start);
    bitboard tmp2(1ULL << end);
    int start_pos=0, end_pos=0;
    for(int &i=start_pos; i<12; i++) if((tmp1 & is_piece[i]) != 0) break;
    for(int &j=end_pos; j<12; j++) if((tmp2 & is_piece[j]) != 0) break;

    auto [start_row, start_col] = gen_coordinate(start);
    auto [end_row, end_col] = gen_coordinate(end);

    if(end_pos == 12 && start_col != end_col && is_piece[6*turn][start]) { // en pessant
        pair<int, int> sec_end_pos = {start_row, end_col};
        for(auto &elem : is_piece) 
            elem.set_val(false, ind_from_coordinate(sec_end_pos));
        
        is_piece[start_pos].set_val(false, start);
        is_piece[start_pos].set_val(true, end);
        ply_100 = 0;
        ply++;
        turn ^= 1;
        en_pessant = {-1, -1};
    } else {
        end_pos = 0;
        if(start == 4 || end==4) white_short_castle = white_long_castle = false;
        if(start == 60 ||end==4) black_short_castle = black_long_castle = false;
        if(start == 0 || end==4) white_short_castle = false;
        if(start == 7 || end==4) white_long_castle = false;
        if(start == 56 || end==4) black_short_castle = false;
        if(start == 63 || end==4) black_long_castle = false;

        if(is_piece[6*turn][start] && abs(start-end) == 16) en_pessant == gen_coordinate((start+end)/2);
        else en_pessant = {-1, -1};

        is_piece[start_pos].set_val(false, start);
        for(auto &elem : is_piece) if(elem[end]) { elem.set_val(false, end); ply_100 = -1; }
        is_piece[start_pos].set_val(true, end);
        ply_100++;
        ply++;
        turn^=1;
    }
    update_is_anything_color();
}

void board::make_move(const pair<int, int> &start, const pair<int, int> &end){
    make_move({ind_from_coordinate(start), ind_from_coordinate(end)});
}

void board::update_state(){
    if(ply_100 == 100) {current_state = draw_50_rule; return;}
    if(turn == 0) {if(black_king & gen_attacked(turn)) {current_state = white_won; return;}}
    if(turn == 1) {if(white_king & gen_attacked(turn)) {current_state = black_won; return;}}
    if(gen_moves().size() == 0) {current_state == draw_stalemate; return;}
}

set<string> board::print_moves(){
    cout << "Avalaible moves";
    auto tmp = gen_moves();
    set<string> res;
    if(tmp.size() == 0) {
        cout << ": none!\n";
        switch (current_state) {
            case white_won : cout << "White won!\n"; break;
            case draw_3_fold : cout << "Draw! (a 3-fold repetition)\n"; break;
            case draw_50_rule : cout << "Draw! (50-move rule)\n"; break;
            case draw_stalemate : cout << "Draw by stalemate!\n"; break;
            case black_won : cout << "Black won!\n"; break;
        }
        return {};
    }

    cout << " (" << tmp.size() << ")\n";
    while(tmp.size()) {
        auto top = tmp.top(); tmp.pop();
        if(top.first == top.second){
            cout << (top.first % 2 ? "o-o-o\n" : "o-o\n");
            (top.first % 2 ? res.insert("o-o-o") : res.insert("o-o"));
            continue;
        }
        if(top.first<0) {
            string without_prom = (std::string(1, char(gen_coordinate(-top.first).second + 'a')) + 
                      std::to_string(gen_coordinate(-top.first).first + 1) + '-' +
                      std::string(1, char(gen_coordinate(top.second >> 2).second + 'a')) + 
                      std::to_string(gen_coordinate(top.second >> 2).first + 1) + '=');
                      
            if(top.second%4 == 0){
                without_prom+=("N");
                cout << without_prom << '\n';
                res.insert(without_prom);
                continue;
            }
            if(top.second%4 == 1){
                without_prom+=("B");
                cout << without_prom << '\n';
                res.insert(without_prom);
                continue;
            }
            if(top.second%4 == 2){
                without_prom+=("R");
                cout << without_prom << '\n';
                res.insert(without_prom);
                continue;
            }
            if(top.second%4 == 3){
                without_prom+=("Q");
                cout << without_prom << '\n';
                res.insert(without_prom);
                continue;
            }
        }
        string move = (std::string(1, char(gen_coordinate(top.first).second + 'a')) + 
                      std::to_string(gen_coordinate(top.first).first + 1) + '-' +
                      std::string(1, char(gen_coordinate(top.second).second + 'a')) + 
                      std::to_string(gen_coordinate(top.second).first + 1));

        cout << move << '\n';
        res.insert(move);
    }
    return res;
}

void board::user_move(set<string> legal){
    cout << "input move: ";
    string s; cin >> s;
    while(legal.count(s) == 0) {
        cout << "Illegal move! Try again.\ninput move: ";
        cin >> s;
    }

    if(s == "o-o") {make_move({0 + 2*turn, 0 + 2*turn}); return;}
    if(s == "o-o-o") {make_move({1 + 2*turn, 1 + 2*turn}); return;}
    if(s.size()==7) {
        int piece;
        if(s.back() == 'N') piece = 0;
        if(s.back() == 'B') piece = 1;
        if(s.back() == 'R') piece = 2;
        if(s.back() == 'Q') piece = 3;
        make_move({-ind_from_coordinate({s[1]-'1', s[0]-'a'}), (ind_from_coordinate({s[4]-'1', s[3]-'a'})<<2) + piece});
        return;
    } 
    make_move({s[1]-'1', s[0]-'a'}, {s[4]-'1', s[3]-'a'});
}

board::board (const string &fen) 
    : is_piece{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      white_pawn(is_piece[0]),
      white_knight(is_piece[1]), 
      white_bishop(is_piece[2]), 
      white_rook(is_piece[3]), 
      white_queen(is_piece[4]), 
      white_king(is_piece[5]), 
      black_pawn(is_piece[6]), 
      black_knight(is_piece[7]), 
      black_bishop(is_piece[8]), 
      black_rook(is_piece[9]), 
      black_queen(is_piece[10]), 
      black_king(is_piece[11]),
      is_anything(0), 
      is_color{0, 0},
      is_white(is_color[0]), 
      is_black(is_color[1]),
      current_state(undecided)
{
    constexpr array<int, 128> parse = []() {
        array<int, 128> map{};
        
        map['P'] = 0;
        map['N'] = 1;
        map['B'] = 2;
        map['R'] = 3;
        map['Q'] = 4;
        map['K'] = 5;
        map['p'] = 6;
        map['n'] = 7;
        map['b'] = 8;
        map['r'] = 9;
        map['q'] = 10;
        map['k'] = 11;
        
        return map;
    }();

    int fen_pos=0;
    for(int j = 7; j >= 0; j--) {
        int i = 0;
        for(; fen[fen_pos] != '/' && fen[fen_pos] != ' '; fen_pos++) {
            if(fen[fen_pos] >= '0' && fen[fen_pos] <= '9')
                i += fen[fen_pos] - '0';
            else 
                is_piece[parse[fen[fen_pos]]].set_val(true, ind_from_coordinate({j, i++}));
        }
        fen_pos++;
    }
    
    turn = (fen[fen_pos] != 'w');

    fen_pos++;
    fen_pos++;
    if(fen[fen_pos] != '-'){
        while(fen[fen_pos] != ' '){
            white_short_castle |= (fen[fen_pos] == 'K');
            white_long_castle  |= (fen[fen_pos] == 'Q');
            black_short_castle |= (fen[fen_pos] == 'k');
            black_long_castle  |= (fen[fen_pos] == 'q');

            fen_pos++;
        }
    } else fen_pos++;

    fen_pos++;
    if(fen[fen_pos] != '-') 
        en_pessant = {fen[fen_pos+1]-'1', fen[fen_pos++]-'a'};
    
    fen_pos++;
    fen_pos++;

    while(fen[fen_pos] != ' '){
        ply_100 *= 10;
        ply_100 += (fen[fen_pos++] - '0');
    }
    
    fen_pos++;

    while(fen.size() > fen_pos){
        ply *= 10;
        ply += (fen[fen_pos++] - '0');
    }

    white_pawn    = is_piece[0];
    white_knight  = is_piece[1];
    white_bishop  = is_piece[2];
    white_rook    = is_piece[3];
    white_queen   = is_piece[4];
    white_king    = is_piece[5];
    black_pawn    = is_piece[6];
    black_knight  = is_piece[7];
    black_bishop  = is_piece[8];
    black_rook    = is_piece[9];
    black_queen  = is_piece[10];
    black_king   = is_piece[11];
    is_white      = is_color[0];
    is_black      = is_color[1];

    update_is_anything_color();
}

void board::print_board() {
    constexpr std::array<char, 13> parse = {
        'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k', ';'
    };

    for(int row = 7; row >= 0; row--){
        cout << row + 1 << ' ';
        for(int column = 0; column < 8; column++){
            int i;
            for(i=0; i<12; i++){
                if(is_piece[i][ind_from_coordinate({row, column})])
                    break;
            }
            cout << parse[i];
        }
        cout << '\n';
    }
    cout << "  abcdefgh\n";

}
