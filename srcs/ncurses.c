// Build with -lncurses option

#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bomberwoman.h"
#include "ncurse.h"

static int tabulation = 7; // destiné à tous les joueurs

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
                    c = 'I';
                    break;
                case 2:
                    c = 'D';
                    break;
                case 3 :
                    c = 'B';
                    break;
                case 4 : 
                    c = 'E';
                    break;
                case 5 : 
                    c = '0';
                    break;
                case 6 : 
                    c = '1';
                    break;
                case 7 : 
                    c = '2';
                    break;
                case 8 : 
                    c = '3';
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

ACTION control(line* l, int sock_TCP, uint16_t *rep_serv) {
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
        case 10:  
            printf("entrée cliquée\n");
            messageTchatClient(sock_TCP, rep_serv, tabulation, l->data, strlen(l->data)); 
            memset(l->data, 0, sizeof(l->data)); break;
        case 9:
           tabulation = tabulation == 8 ? 7 : 8; break;
        default:
            if (prev_c >= ' ' && prev_c <= '~' && l->cursor < TEXT_SIZE)
                l->data[(l->cursor)++] = prev_c;
            break;

    }
    return a;
}

bool perform_action(board* b, pos* p, ACTION a, u_int16_t *buf, int sock_UDP, struct sockaddr_in6 serv_dest, line *l) {
    pthread_t thread_bomb;
    arg_thread_bomb *args;

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
            args = malloc(sizeof(arg_thread_bomb));  // Allocation dynamique
            if (!args) {
                perror("Allocation dynamique échouée");
                exit(EXIT_FAILURE);
            }
            memset(args, 0, sizeof(arg_thread_bomb));
            args->x = p->x;
            args->y = p->y;
            args->b = b;
            args->l = l;
            // printf("x : %d ; y : %d\n", args->x, args->y);
            if (pthread_create(&thread_bomb, NULL,(void *)explode_bomb, (void *)args) < 0) {
                perror("Création thread");
                free(args->b);
                free(args);
                exit(EXIT_FAILURE);
            }
            actions(4, buf, sock_UDP,serv_dest); 
            set_grid(b,p->x,p->y,4); 
            return false;
        case QUIT:
            return true;
        default: break;
    }
    if ((get_grid(b, p->x + xd, p->y + yd) == 1) // si le joueur avance sur un mur ou en dehors du jeu
    || (get_grid(b, p->x + xd, p->y + yd) == 2) 
    || p->x + xd < 0 || p->y + yd < 0 
    || p->x + xd >= b->w || p->y + yd >= b->h) {
        actions(5, buf, sock_UDP,serv_dest); set_grid(b,p->x,p->y,9) ; return false;
    }
    else {
        if(get_grid(b,p->x,p->y) != 3) set_grid(b, p->x,p->y,0);
        p->x += xd; p->y += yd;
    }

    if (get_grid(b,p->x,p->y) != 3) set_grid(b,p->x,p->y, 5); // joueur courant
    return false;
}

int ncurses(uint16_t *rep_serv, int sock_UDP, int sock_TCP, struct sockaddr_in6 serv_dest)
{
    board* b = malloc(sizeof(board));;
    line* l = malloc(sizeof(line));
    l->cursor = 0;
    pos* p = malloc(sizeof(pos));
    uint16_t id = (ntohs(rep_serv[0]) >> 13) & 0x3;
    //printf("%u\n", id);
    p->x = id % 2 ; p->y = id/2;
    //printf("x = %d, y = %d\n", p->x, p->y);

    // NOTE: All ncurses operations (getch, mvaddch, refresh, etc.) must be done on the same thread.
    raw(); /* Disable line buffering */
    intrflush(stdscr, FALSE); /* No need to flush when intr key is pressed */
    keypad(stdscr, TRUE); /* Required in order to get events from keyboard */
    nodelay(stdscr, TRUE); /* Make getch non-blocking */
    noecho(); /* Don't echo() while we do getch (we will manually print characters when relevant) */
    curs_set(0); // Set the cursor to invisible
    start_color(); // Enable colors
    init_pair(1, COLOR_YELLOW, COLOR_BLACK); // Define a new color style (text is yellow, background is black)

    // setup_board(b);
    // p->x *= (b->w - 1); //configure les coordonnées du joueur 
    // p->y *= (b->h - 1);
    //printf("x = %d, y = %d\n", p->x, p->y);
    //exit(0);
    while (true) {
        ACTION a = control(l, sock_TCP, rep_serv);
        if (perform_action(b, p, a, rep_serv, sock_UDP, serv_dest, l)) break;
        refresh_game(b,l);
        usleep(30*1000);
    }
    free_board(b);

    curs_set(1); // Set the cursor to visible again

    free(p); free(l); free(b);

    return 0;
}
