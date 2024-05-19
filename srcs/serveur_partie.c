//#include "bomberwoman.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/select.h>
#include "bomberwoman.h"
#include "ncurse.h"

#define PORT 12121 /* quel port utiliser ?*/

static int PORT_UDP = 1234;
int PORT_MDIF = 4321;
int NUM_MSG = 0;

int rajoute_joueur_partie(liste_parties_t *lp, int type_4) // rajouter un gros lock sur la fonction
{
    joueur_t *j = malloc(sizeof(joueur_t));    

    liste_parties_t *courante = lp;

    if (courante->partie == NULL)
    {
        int sock_serv_UDP = socket(PF_INET6, SOCK_DGRAM, 0);
        if (sock_serv_UDP < 0) return -1;

        struct sockaddr_in6 servadr;
        memset(&servadr, 0, sizeof(servadr));
        servadr.sin6_family = AF_INET6;
        if (inet_pton(AF_INET6, "::1", &servadr.sin6_addr) == -1) {
            printf("inet_pton non réussi\n");
            return -1;
        }
        servadr.sin6_port = htons(PORT_UDP);

        bind(sock_serv_UDP, (struct sockaddr*)&servadr, sizeof(servadr));

        partie_t *p = malloc(sizeof(partie_t));
        j->id = 0;
        j->id_equipe = 0;
        p->joueurs[0] = j;
        p->nb_joueurs_courant = 1;
        p->port = PORT_UDP;
        p->adresse_serv_UDP = servadr;
        p->sock_serv_UDP = sock_serv_UDP;
        p->type = (type_4)? 1: 0;
        lp->partie = p;
        PORT_UDP++;   
        create_sockaddr_mdif(p, PORT_MDIF);
        PORT_MDIF++;
    }
    else 
    {
        while (courante->suivant != NULL)
        {
            courante = courante->suivant;
        }
        partie_t *p_courante = courante->partie;
        if (p_courante->nb_joueurs_courant < 4)
        {
            j->id = p_courante->nb_joueurs_courant;
            if (!type_4) 
            {
                p_courante->type = 0;
                if (p_courante->nb_joueurs_courant < 2) {j->id_equipe = 0;}
                else {j->id_equipe = 1;}
            }
            else { p_courante->type = 1;}
            p_courante->joueurs[p_courante->nb_joueurs_courant] = j;
            p_courante->nb_joueurs_courant+=1;
        }
        else
        {

            int sock_serv_UDP = socket(PF_INET6, SOCK_DGRAM, 0);
            if (sock_serv_UDP < 0) return -1;
            struct sockaddr_in6 servadr;
            memset(&servadr, 0, sizeof(servadr));
            servadr.sin6_family = AF_INET6;
            inet_pton(AF_INET6, "::1", &servadr.sin6_addr);
            servadr.sin6_port = htons(PORT_UDP);
            if (bind(sock_serv_UDP, (struct sockaddr *)&servadr, sizeof(servadr)) < 0) return -1;

            partie_t *p = malloc(sizeof(partie_t));
            j->id=0;
            j->id_equipe = 0;
            p->joueurs[0] = j;
            p->nb_joueurs_courant = 1;
            p->port = PORT_UDP;
            liste_parties_t *lp2 = malloc(sizeof(liste_parties_t));
            lp2->partie = p;
            p->adresse_serv_UDP = servadr;
            p->sock_serv_UDP = sock_serv_UDP;
            p->type = (type_4)? 1: 0;
            courante->suivant = lp2;
            PORT_UDP++; 
            create_sockaddr_mdif(p, PORT_MDIF);
            PORT_MDIF++;           
        }
    }
    return 0;
}

void create_sockaddr_mdif(partie_t *p, int port_MDIF)
{
    int sock = socket(PF_INET6, SOCK_DGRAM,0); 
    if (sock < 0){
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in6 adresseMultiDiff;
    memset(&adresseMultiDiff, 0, sizeof(adresseMultiDiff));
    adresseMultiDiff.sin6_family = AF_INET6;
    inet_pton(AF_INET6,"ff12::1:2:3", &adresseMultiDiff.sin6_addr);
    adresseMultiDiff.sin6_port = htons(port_MDIF);
    printf("portMDIFF serveur = %u\n", port_MDIF);
    p->adresse_serv_MDIF = adresseMultiDiff;
    p->port_MDIF = port_MDIF;


    /* Liaison de la socket à une interface réseau spécifique */
    int ifindex = if_nametoindex("wlp0s20f3");
    if (ifindex < 0) {perror("erreur interface"); close(sock); exit(EXIT_FAILURE);}
    adresseMultiDiff.sin6_scope_id = 0;

    int ok = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0) {
        perror("echec de SO_REUSEADDR");
        close(sock);
    }

    p->sock_serv_MDIF = sock;
}


int client_thread(arg_thread_t *args) 
{
    int sock_client = args->socket_client;

    liste_parties_t *p_2v2 = args->liste_2v2;
    liste_parties_t *p_4_adv = args->liste_4adv;

    int messageRecu;
    int res_recv = 0;
    
    u_int16_t req[1];
    memset(&req, 0, sizeof(req));

    while ((size_t)res_recv < sizeof(req))
    {
        messageRecu = recv(sock_client, req, 1, 0);
        if (messageRecu == -1) 
        {
            perror("Erreur lors de la reception du message");
            return 0;
            // exit(EXIT_FAILURE);
        }
        if (messageRecu == 0) 
        {
            break;
        }
        res_recv += messageRecu;
    }
    printf("premier recv serveur fonctionnel\n");

    if (messageRecu == -1) {
        perror("Erreur lors de la réception");
        return 0;
        // exit(EXIT_FAILURE);
    }

    u_int16_t rep_0 = 0;
    char adrMdif[16] = {0};
    memcpy(adrMdif, "ff12::1:2:3", strlen("ff12::1:2:3"));
    u_int16_t rep[6];
    memset(&rep, 0, sizeof(rep));


    liste_parties_t *courante;
    pthread_mutex_lock(args->verrou);
    if (req[0] == 2) 
    { 
        rajoute_joueur_partie (p_2v2, 0);
        rep_0 |= 10 << 3;
        courante = p_2v2;
    }
    else 
    {
        rajoute_joueur_partie(p_4_adv, 1);
        rep_0 |= 9 << 3;
        courante = p_4_adv;
    }
    pthread_mutex_unlock(args->verrou);
    printf("post mutex\n");

    while(courante->suivant != NULL && courante->suivant->partie != NULL)
    {
        courante = courante->suivant;
    }
    
    int nb_joueurs = courante->partie->nb_joueurs_courant;
    u_int16_t id = ( nb_joueurs - 1);
    rep_0 |= id << 1;
    rep_0 |= courante->partie->joueurs[nb_joueurs-1]->id_equipe;
    rep[0] = htons(rep_0);

    rep[1] = htons(courante->partie->port);
    rep[2] = htons(courante->partie->port_MDIF);
    

    int reponse = 0;
    ssize_t envoi;
    while((size_t)reponse < sizeof(rep)) 
    {
        envoi = send(sock_client, rep + reponse, sizeof(rep) - reponse, 0);
        if (envoi == -1) 
        {
            perror("Erreur lors de l'envoi du message");
            exit(EXIT_FAILURE);
        }
        if (envoi == 0) 
        {
            break;
        }
        reponse += envoi;
    }
    
    reponse = 0;
    envoi = 0;
    while((size_t)reponse < sizeof(adrMdif)) 
    {
        envoi = send(sock_client, adrMdif + reponse, sizeof(adrMdif)-reponse, 0);
        if (envoi == -1) 
        {
            perror("Erreur lors de l'envoi du message");
            exit(EXIT_FAILURE);
        }
        if (envoi == 0) 
        {
            break;
        }
        reponse += envoi;
    }
    printf("post second send serveur\n");

    serveur(courante->partie);
    

    //char buf[25];
    uint16_t client_move[2];
    socklen_t addr_len = sizeof(courante->partie->adresse_serv_UDP);

    int sock_serv_UDP = courante->partie->sock_serv_UDP;

    while (1) {     
        fd_set rset = args->rset;
        //FD_ZERO(&rset);
        
        FD_SET(sock_serv_UDP, &rset);
        select(sock_serv_UDP + 1, &rset, NULL, 0, NULL);
        if (FD_ISSET(sock_serv_UDP, &rset)) {
            if (recvfrom(sock_serv_UDP, client_move, sizeof(client_move), 0, (struct sockaddr *)&courante->partie->adresse_serv_UDP, &addr_len)<0){ printf("arrrrr\n"); return -1;}
        }

        // serveur();
        uint16_t action = (ntohs(client_move[1]) >> 13) & 0x7;
        printf("action : %u\n", action);
    }
    close(sock_client);
    return 1;
}

int serveur(partie_t *p) {

    printf("diffusion OK\n");
    sleep(1);
    if (p->nb_joueurs_courant < 4) 
    {
        char message[1024] = {0};
        sprintf(message, "Il manque encore %d joueur.se.s", 4 - p->nb_joueurs_courant);
        int envoyes = 0;
        while ((size_t)envoyes < strlen(message)) {
            ssize_t tailleEnvoie = sendto(p->sock_serv_MDIF, message + envoyes, strlen(message) - envoyes, 0, (struct sockaddr *)&p->adresse_serv_MDIF, sizeof(p->adresse_serv_MDIF));
            if (tailleEnvoie < 0) {
                perror("Erreur lors de l'envoi du message");
                close(p->sock_serv_MDIF);
                exit(EXIT_FAILURE);
            }
            envoyes += tailleEnvoie;
        }
    }
    else
    {
        char message[1024] = {0};
        sprintf(message, "La partie peut commencer !");
        int envoyes = 0;
        while ((size_t)envoyes < strlen(message)) {
            ssize_t tailleEnvoie = sendto(p->sock_serv_MDIF, message + envoyes, strlen(message) - envoyes, 0, (struct sockaddr *)&p->adresse_serv_MDIF, sizeof(p->adresse_serv_MDIF));
            if (tailleEnvoie < 0) {
                perror("Erreur lors de l'envoi du message");
                close(p->sock_serv_MDIF);
                exit(EXIT_FAILURE);
            }
            envoyes += tailleEnvoie;
        }
    }    
    //initialisation du board
    board *b = malloc(sizeof(board));
    setup_board(b);

    //envoie du board //metrre un mutex pour accéder et modifier num
    u_int16_t *format_msg_grille = grille_format(NUM_MSG, b->h, b->w, b->grid);
    int envoyes = 0;
    printf("largeur : %d; hauteur : %d\n", b->w, b->h);
    printf("largeur : %d; hauteur : %d;\n",(format_msg_grille[2] & 0xFF), ((format_msg_grille[2] & 0xFF) >> 8));

    while ((size_t)envoyes < 6) {
        ssize_t tailleEnvoie = sendto(p->sock_serv_MDIF, format_msg_grille + envoyes, sizeof(format_msg_grille) - envoyes, 0, (struct sockaddr *)&p->adresse_serv_MDIF, sizeof(p->adresse_serv_MDIF));
        if (tailleEnvoie < 0) {
            perror("Erreur lors de l'envoi du message");
            close(p->sock_serv_MDIF);
            exit(EXIT_FAILURE);
        }
        envoyes += tailleEnvoie;
    }

    while ((size_t)envoyes < sizeof(format_msg_grille)) {
        ssize_t tailleEnvoie = sendto(p->sock_serv_MDIF, format_msg_grille + envoyes, sizeof(format_msg_grille) - envoyes, 0, (struct sockaddr *)&p->adresse_serv_MDIF, sizeof(p->adresse_serv_MDIF));
        if (tailleEnvoie < 0) {
            perror("Erreur lors de l'envoi du message");
            close(p->sock_serv_MDIF);
            exit(EXIT_FAILURE);
        }
        envoyes += tailleEnvoie;
    }


    printf("Envoie OK\n");
    return 0;
}

int get_grid(board* b, int x, int y) {
    return b->grid[y*b->w + x];
}

void set_grid(board* b, int x, int y, int v) {
    b->grid[y*b->w + x] = v;
}

void setup_board(board* board) {
    srand(time(NULL));
    int lines; int columns;
    getmaxyx(stdscr,lines,columns);
    board->h = lines - 2 - 1; // 2 rows reserved for border, 1 row for chat
    board->w = columns - 2; // 2 columns reserved for border
    printf("largeur : %d; hauteur : %d;\n", board->w, board->h);
    board->grid = calloc((board->w)*(board->h),sizeof(char));

    int x, y; 

    // Position des murs indestructibles
    for (x = 0; x < board->w; x++) {    
        for (y = 0; y < board->h; y++) {
            if ((x%5 == 4) && (y%2 == 1) ) set_grid(board, x, y, 1);
        }    
    }
    // Position des murs destructibles
    for (x = 0 ; x < board->w; x++) {   
        if (x > 5 && (x < board->w-5) && (rand()*3 == 2)) set_grid(board, x, 0, 2);
        if (x > 5 && (x < board->w-5) && (rand()*3 == 2)) set_grid(board, x, board->h, 2);
        for (y = 0 ; y < board->h; y++) {
            if (rand()%5 == 1) set_grid(board, x, y, 2);
        }    
    }
    for (y = 5 ; y < board->h; y++) {
        if (rand()*3 == 2) set_grid(board, 0, y, 2);
        if (rand()*3 == 2) set_grid(board, board->w, y, 2);
    }
    
    // Position des joueurs dans la grille
    set_grid(board, 0, 0, 0); 
    set_grid(board, board->w-1, board->h - 1, 1);
    set_grid(board, 0, board->h - 1, 2);
    set_grid(board, board->w - 1, 0, 3);
}

u_int16_t header(int codereq, int id, int eq) {
    u_int16_t res;

    res = 0;
    res |= (u_int16_t)(codereq & 0x1FFF); // 1FF est un masque hexa pour 111111111111 (12 bits)
    res |= (u_int16_t)((id & 0x3) << 13); // l'id est placé sur le bit 13
    res |= (u_int16_t)((eq & 0x1) << 15); // puis eq sur le bit 15
    return (htons(res)); // le tout est ensuite mis au format big endian
}

u_int16_t *grille_format(int num, int hauteur, int largeur, char *plateau) {
    u_int16_t *grille = malloc((3 + (hauteur * largeur) / 2)*sizeof(uint16_t));
    u_int16_t grille_2;
    u_int16_t grille_i;
    int i = 2;

    grille[0] = header(11, 0, 0); // le header est placé sur la première ligne
    grille[1] = htons(num % (int) pow(2, 16)); // le numéro du message modulo 2^16 occupe la deuxième au format big endian

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
                grille_i |= (u_int16_t)(plateau[x*largeur+y] & 0xFF);
            }
            else { // sinon on ajoute la case du plateau sur le second octet de la ligne
                grille_i |= (u_int16_t)((plateau[x*largeur + y] & 0xFF) << 8);
                grille[i] = grille_i;
            }
        }
    }
    return grille;
}
