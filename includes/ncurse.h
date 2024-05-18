#ifndef NCURSE_H

#define NCURSE_H
#define TEXT_SIZE 255

typedef enum ACTION { NONE, UP, DOWN, LEFT, RIGHT, BOMB, QUIT, ENTREE } ACTION;

typedef struct board {
    char* grid;
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

void explode_bomb(board *b, int x, int y) ;
void set_grid(board* b, int x, int y, int v);
int get_grid(board* b, int x, int y) ;

#endif