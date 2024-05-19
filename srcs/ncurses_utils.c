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
    for (int i = x - 1; i < x + 2; i++) {
        for (int j = y - 1; j < y + 2; j++) {
            if (get_grid(b, i, y) == 4) {
                printf("je pense que ça a déjà segfault\n");
                set_grid(b, x, y, 0);
            }
            // if (get_grid(b, i, j) == 1) { 
            //     //un joueur est mort, contacter les secours (le serveur pour le virer)
            //     set_grid(b, args->x, y, 0);
            // }
        }
    }
    free(arg);  // Libérer la mémoire allouée
    return NULL;
}