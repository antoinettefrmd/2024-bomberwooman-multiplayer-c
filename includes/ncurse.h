#ifndef NCURSE_H

#define NCURSE_H
#define TEXT_SIZE 255

#include <ncurses.h>

typedef enum ACTION { NONE, UP, DOWN, LEFT, RIGHT, BOMB, QUIT, ENTREE } ACTION;

typedef struct board {
    int* grid;
    int w;
    int h;
} board;

typedef struct line {
    char data[TEXT_SIZE];
    int cursor;
} line;

typedef struct pos {
    int x;
    int y;
} pos;

typedef struct arg_thread_bomb {
    int x;
    int y;
    struct board *b;
    struct line *l;
} arg_thread_bomb;

void refresh_game(board* b, line* l);
void *explode_bomb(arg_thread_bomb *args);
void set_grid(board* b, int x, int y, int v);
int get_grid(board* b, int x, int y) ;
void setup_board(board* board);
void print_board(board* b);

#endif