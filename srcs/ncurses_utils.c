#include <unistd.h>
#include <stdlib.h>
#include "bomberwoman.h"
#include "ncurse.h"


void explode_bomb(board *b, int x, int y) 
{
    sleep(3);
    for (int i = x - 1 ; i < x + 2 ; i++)
    {
        for (int j = y - 1 ; j < y + 2 ; j++)
        {
            if (get_grid(b, i, j) == 4) {
                set_grid(b, x, y, 0);
            }
            if (get_grid(b, i, j) == 1) { 
                //un joueur est mort, contacter les secours (le serveur pour le virer)
                set_grid(b, x, y, 0);
            }

        }
    }

}