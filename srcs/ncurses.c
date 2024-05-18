// Build with -lncurses option

#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bomberwoman.h"
#include "ncurse.h"

// static int tabulation = 7; // destiné à tous les joueurs


void setup_board(board* board) {
    srand(time(NULL));
    int lines; int columns;
    getmaxyx(stdscr,lines,columns);
    board->h = lines - 2 - 1; // 2 rows reserved for border, 1 row for chat
    board->w = columns - 2; // 2 columns reserved for border
    board->grid = calloc((board->w)*(board->h),sizeof(char));

    int x, y; 

    for (x = 0; x < board->w; x++) {    
        for (y = 0; y < board->h; y++) {
            if ((x%5 == 4) && (y%2 == 1) ) set_grid(board, x, y, 3);
        }    
    }
    for (x = 0 ; x < board->w; x++) {   
        if (x > 5 && (rand()*3 == 2)) set_grid(board, x, 0, 4);
        if (x > 5 && (rand()*3 == 2)) set_grid(board, board->h, 0, 4);
        for (y = 0 ; y < board->h; y++) {
            if (rand()%5 == 1) set_grid(board, x, y, 4);
        }    
    }
    for (y = 5 ; y < board->h; y++) {
        if (rand()*3 == 2) set_grid(board, 0, y, 4);
        if (rand()*3 == 2) set_grid(board, board->w, y, 4);
    }

}

void free_board(board* board) {
    free(board->grid);
}

int get_grid(board* b, int x, int y) {
    return b->grid[y*b->w + x];
}

void set_grid(board* b, int x, int y, int v) {
    b->grid[y*b->w + x] = v;
}

void refresh_game(board* b, line* l) {
    // Update grid
    int x,y;
    for (y = 0; y < b->h+2; y++) {
        for (x = 0; x < b->w+2; x++) {
            char c;
            switch (get_grid(b,x,y)) {
                case 0:
                    c = ' ';
                    break;
                case 1:
                    c = 'O';
                    break;
                case 2:
                    c = 'B';
                    break;
                case 3 :
                    c = 'I';
                    break;
                case 4 : 
                    c = 'D';
                    break;
                default:
                    c = '?';
                    break;
            }
            mvaddch(y+1,x+1,c);
        }
    }
    for (x = 0; x < b->w+2; x++) {
        mvaddch(0, x, '-'); // mur en haut
        mvaddch(b->h+1, x, '-'); // mur en bas
    }
    for (y = 0; y < b->h+2; y++) {
        mvaddch(y, 0, '|'); // mur à gauche
        mvaddch(y, b->w+1, '|'); // mur à droite
    }
    
    // Update chat text
    attron(COLOR_PAIR(1)); // Enable custom color 1
    attron(A_BOLD); // Enable bold
    for (x = 0; x < b->w+2; x++) {
        if (x >= TEXT_SIZE || x >= l->cursor)
            mvaddch(b->h+2, x, ' ');
        else
            mvaddch(b->h+2, x, l->data[x]);
    }
    attroff(A_BOLD); // Disable bold
    attroff(COLOR_PAIR(1)); // Disable custom color 1
    refresh(); // Apply the changes to the terminal
}

ACTION control(line* l) {
    int c;
    int prev_c = ERR;
    // We consume all similar consecutive key presses
    while ((c = getch()) != ERR) { // getch returns the first key press in the queue
        if (prev_c != ERR && prev_c != c) {
            ungetch(c); // put 'c' back in the queue
            break;
        }
        prev_c = c;
    }
    ACTION a = NONE;
    //printf("prev_c = %d\n", prev_c);
    switch (prev_c) {
        case ERR: break;
        case KEY_LEFT:
            a = LEFT; break;
        case KEY_RIGHT:
            a = RIGHT; break;
        case KEY_UP:
            a = UP; break;
        case KEY_DOWN:
            a = DOWN; break;
        case ')':
            a = BOMB; break;
        case '~':
            a = QUIT; break;
        case KEY_BACKSPACE:
            if (l->cursor > 0) l->cursor--;
            break;
        case 10: // correspond au bouton entrée
           // messageTchatClient(buf,tabulation,l->data);
            //memset(l->data, 0, sizeof(l->data)); // on vide data 
        case 9: // correspond à tabulation
        //    tabulation = tabulation == 8 ? 7 : 8;
        default:
            if (prev_c >= ' ' && prev_c <= '~' && l->cursor < TEXT_SIZE)
                l->data[(l->cursor)++] = prev_c;
            break;

    }
    return a;
}

bool perform_action(board* b, pos* p, ACTION a, u_int16_t *buf, int sock_UDP, struct sockaddr_in6 serv_dest) {
    int xd = 0;
    int yd = 0;
    switch (a) {
        case LEFT:
            xd = -1; yd = 0; actions(3, buf, sock_UDP, serv_dest); break;
        case RIGHT:
            xd = 1; yd = 0; actions(1, buf, sock_UDP, serv_dest); break;
        case UP:
            xd = 0; yd = -1; actions(0, buf, sock_UDP, serv_dest);  break;
        case DOWN:
            xd = 0; yd = 1; actions(2, buf, sock_UDP, serv_dest);  break; 
        case BOMB :
            explode_bomb(b, p->x, p->y); actions(4, buf, sock_UDP,serv_dest); set_grid(b,p->x,p->y,2) ; return false;
        case QUIT:
            return true;
        default: break;
    }
    if ((get_grid(b, p->x + xd, p->y + yd) == 3) 
    || (get_grid(b, p->x + xd, p->y + yd) == 4) 
    || p->x + xd < 0 || p->y + yd < 0 
    || p->x + xd >= b->w || p->y + yd >= b->h) {
        actions(5, buf, sock_UDP,serv_dest); set_grid(b,p->x,p->y,5) ; return false;
    }
    else {
        set_grid(b, p->x,p->y,0);
        p->x += xd; p->y += yd;
    }

    if (get_grid(b,p->x,p->y) != 2) set_grid(b,p->x,p->y, 1);
    return false;
}

int ncurses(uint16_t *rep_serv, int sock_UDP, struct sockaddr_in6 serv_dest)
{
    board* b = malloc(sizeof(board));;
    line* l = malloc(sizeof(line));
    l->cursor = 0;
    pos* p = malloc(sizeof(pos));
    p->x = 0; p->y = 0;

    // NOTE: All ncurses operations (getch, mvaddch, refresh, etc.) must be done on the same thread.
    initscr(); /* Start curses mode */
    raw(); /* Disable line buffering */
    intrflush(stdscr, FALSE); /* No need to flush when intr key is pressed */
    keypad(stdscr, TRUE); /* Required in order to get events from keyboard */
    nodelay(stdscr, TRUE); /* Make getch non-blocking */
    noecho(); /* Don't echo() while we do getch (we will manually print characters when relevant) */
    curs_set(0); // Set the cursor to invisible
    start_color(); // Enable colors
    init_pair(1, COLOR_YELLOW, COLOR_BLACK); // Define a new color style (text is yellow, background is black)

    setup_board(b);
    while (true) {
        ACTION a = control(l);
        if (perform_action(b, p, a, rep_serv, sock_UDP, serv_dest)) break;
        refresh_game(b,l);
        usleep(30*1000);
    }
    free_board(b);

    curs_set(1); // Set the cursor to visible again
    endwin(); /* End curses mode */

    free(p); free(l); free(b);

    return 0;
}
