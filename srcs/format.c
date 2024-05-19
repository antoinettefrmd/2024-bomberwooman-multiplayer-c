#include "bomberwoman.h"
#include <math.h>

/*header de chaque message
u_int16_t header(int codereq, int id, int eq) {
    u_int16_t res;

    res = 0;
    res |= (u_int16_t)(codereq & 0x1FFF); // 1FF est un masque hexa pour 111111111111 (12 bits)
    res |= (u_int16_t)((id & 0x3) << 13); // l'id est placé sur le bit 13
    res |= (u_int16_t)((eq & 0x1) << 15); // puis eq sur le bit 15
    return (htons(res)); // le tout est ensuite mis au format big endian
}

void move_format(int codereq, int id, int eq, int num, int action) {
    u_int16_t move[2];
    u_int16_t move_1;

    move[0] = header(codereq, id, eq); // le header est placé sur les deux premiers octets

    move_1 = 0;
    move_1 |= (u_int16_t)(num & 0x1FFF); // le numéro est également codé sur 12 bits
    move_1 |= (u_int16_t)((action & 0x3) << 13); // action est placé sur le bit 13
    move[1] = htons(move_1); // les deux octets sont mis au format big endian
}


*/
/*
void grille_format(int num, int hauteur, int largeur, int **plateau) {
    u_int16_t grille[3 + (hauteur * largeur) / 2]; // pareil que le tchat, chaque case est codée sur un octet
    u_int16_t grille_2;
    u_int16_t grille_i;
    int i = 2;

    grille[0] = header(11, 0, 0); // le header est placé sur la première ligne
    grille[1] = htons(num % pow(2, 16)); // le numéro du message modulo 2^16 occupe la deuxième au format big endian

    grille_2 = 0;
    grille_2 |= (u_int16_t)(hauteur & 0xFF); // la hauteur est codé sur le premier octet
    grille_2 |= (u_int16_t)((largeur & 0xFF) << 8); // la largeur sur le second
    grille[2] = grille_2;

    //on parcourt la grille
    for (int x = 0; x < hauteur; x++) {
        for (int y = 0; y < largeur; y++) {
            if ((x * largeur + y) % 2 == 0) { // si on est sur un premier octet de ligne, on incrémente i puis on 
                                              // réinitalise les deux octets avant de placer la case du plateau sur le premeir
                i++;
                grille_i = 0;
                grille_i |= (u_int16_t)(plateau[x][y] & 0xFF);
            }
            else { // sinon on ajoute la case du plateau sur le second octet de la ligne
                grille_i |= (u_int16_t)((plateau[x][y] & 0xFF) << 8);
                grille[i] = grille_i;
            }
        }
    }

}

void modif_format(int num, int nb, int **cases) {
    u_int16_t modif[3 + nb / 2]; // idem
    u_int16_t modif_i;
    int x = 2; // indice de la ligne à remplir
    int c = 1; // compteur de cases modifiées

    modif[0] = header(12, 0, 0); // on place le header
    modif[1] = htons(num % pow(2, 16)); // la deuxième ligne contient le numéro de message modulo 2^16 au format big endian
    modif_i = 0;
    modif_i |= (u_int16_t)(nb & 0xFF); // nb est le nombre de cases modifées, placé sur le premier octet
    for (int i = 0; i < nb; i++) {
        for (int j = 0; j < 3; j++) { // chaque ligne du tableau cases contient : 
                                      // 0 : le numéro de ligne, 1 : le numéro de colonne, 2 : le contenu de la case modifiée
            if (c % 2 == 1) { // si l'on doit placer le second octet, on le met sur le bit 8 et on incrémente x
                modif_i |= (u_int16_t)((cases[i][j]) << 8); // la case est alors placée sur le bit 8
                modif[x] = modif_i;
                x++;
            }
            else { // sinon on réinitialise l'entier sur 16 bits et on rempli le premier octet avec la case correspondante
                modif_i = 0;
                modif_i |= (u_int16_t)(cases[i][j]);
            }
            c++; // on incrémente le compteur quoi qu'il arrive
                
        }
    }
}
*/