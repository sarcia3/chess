#include <iostream> 
#include "board.h"  

void clearConsole() {
#ifdef _WIN32 
    system("cls");
#else 
    system("clear");
#endif
}

int main() {
    clearConsole();
    board start("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    auto legal = start.print_moves();
    while(legal.size()) {
        start.print_board();
        start.user_move(legal);
        clearConsole();
        legal = start.print_moves();
    }
    start.print_board();
    return 0; 
}
