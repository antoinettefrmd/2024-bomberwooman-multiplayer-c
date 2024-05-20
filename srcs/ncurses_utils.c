#include <unistd.h>
#include <stdlib.h>
#include "bomberwoman.h"
#include "ncurse.h"


void *explode_bomb(arg_thread_bomb *arg) 
{
    sleep(3);
    arg_thread_bomb *args = (arg_thread_bomb *)arg;
    int x = args->x;
    int y = args->y;
    board *b = args->b;
    line *l = args->l;
    // propagation
    if ((get_grid(b, x, y+1) != 3) && (get_grid(b, x, y+1) != 4)){ // ni D ni I
        set_grid(b, x, y+2, 0);
    }
    if ((get_grid(b, x, y-1) != 3) && (get_grid(b, x, y-1) != 4)){
        set_grid(b, x, y-2, 0);
    }
    if ((get_grid(b, x-1, y) != 3) && (get_grid(b, x-1, y) != 4)){
        set_grid(b, x-2, y, 0);
    }
    if ((get_grid(b, x+1, y) != 3) && (get_grid(b, x+1, y) != 4)){
        set_grid(b, x+2, y, 0);
    }
    // boucle première rangée
    for (int i = x - 1; i < x + 2; i++) {
        for (int j = y - 1; j < y + 2; j++) {
            if ((get_grid(b, i, j) == 4) || (i == x && j == y)) {
                set_grid(b, i, j, 0);
            }
            // if (get_grid(b, i, j) == 1) { 
            //     //un joueur est mort, contacter les secours (le serveur pour le virer)
            //     set_grid(b, args->x, y, 0);
            // }
        }
    }

    refresh_game(b,l);
    free(arg);  // Libérer la mémoire allouée
    return NULL;
}