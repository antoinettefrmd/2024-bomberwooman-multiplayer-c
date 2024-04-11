#include "bomberwoman.h"

u_int16_t header(int codereq, int id, int eq) {
    u_int16_t res;

    res = 0;
    res |= (u_int16_t)(codereq & 0x1FF);
    res |= (u_int16_t)((id & 0x3) << 13);
    res |= (u_int16_t)((eq & 0x1) << 15);
    return (htons(res));
}

void move_format(int codereq, int id, int eq, int num, int action) {
    u_int16_t move[2];
    u_int16_t move_1;

    move[0] = header(codereq, id, eq);

    move_1 = 0;
    move_1 |= (u_int16_t)(num & 0x1FFF);
    move_1 |= (u_int16_t)((action & 0x3) << 13);
    move[1] = htons(move_1);
}

void tchat_format(int codereq, int id, int eq, int len, char * data) {
     u_int16_t tchat[2 + (len / 2)];
    u_int16_t tchat_1;
    u_int16_t tchat_i;
    int j = 1;

    tchat[0] = header(codereq, id, eq);

    tchat_1 = 0;
    tchat_1 |= (u_int16_t)(len & 0xFF);
    tchat_1 |= (u_int16_t)((data[0]) << 8);
    tchat[1] = htons(tchat_1);
    for (int i = 2; i < 2 + (len / 2); i++) {
        tchat_i = 0;
        tchat_i |= (u_int16_t)(data[j] & 0xFF);
        if (i != len - 1 || len % 2 == 1)
            tchat_i |= (u_int16_t)((data[j + 1] & 0xFF) << 8);
        j += 2;
        tchat[i] = tchat_i;
    }
}

void grille_format(int num, int hauteur, int largeur, int **plateau) {
    u_int16_t grille[3 + (hauteur * largeur) / 2];
    u_int16_t grille_2;
    u_int16_t grille_i;
    int i = 2;

    grille[0] = header(11, 0, 0);
    grille[1] = htons(num % pow(2, 16));

    grille_2 = 0;
    grille_2 |= (u_int16_t)(hauteur & 0xFF);
    grille_2 |= (u_int16_t)((largeur & 0xFF) << 8);
    grille[2] = grille_2;

    for (int x = 0; x < hauteur; x++) {
        for (int y = 0; y < largeur; y++) {
            if ((x * largeur + y) % 2 == 0) {
                i++;
                grille_i = 0;
                grille_i |= (u_int16_t)(plateau[x][y] & 0xFF);
            }
            else {
                grille_i |= (u_int16_t)((plateau[x][y] & 0xFF) << 8);
                grille[i] = grille_i;
            }
        }
    }

}

void modif_format(int num, int nb, int **cases) {
    u_int16_t modif[3 + nb / 2];
    u_int16_t modif_i;
    int x = 2;
    int c = 1;

    modif[0] = header(12, 0, 0);
    modif[1] = htons(num % pow(2, 16));
    modif_i = 0;
    modif_i |= (u_int16_t)(nb & 0xFF);
    for (int i = 0; i < nb; i++) {
        for (int j = 0; j < 3; j++) {
            if (c % 2 == 1) {
                modif_i |= (u_int16_t)((cases[i][j]) << 8);
                modif[x] = modif_i;
                x++;
            }
            else {
                modif_i = 0;
                modif_i |= (u_int16_t)(cases[i][j]);
            }
            c++;
                
        }
    }
}