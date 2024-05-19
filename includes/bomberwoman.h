#ifndef BOMBERWOMAN_H

#define BOMBERWOMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT_TCP 4444

// Struct joueur

typedef struct joueur {
    int id;
    int id_equipe; 
    struct sockaddr_in6 adresse; // adresse du client quand il se connecte
} joueur_t;

// Struct partie

typedef struct partie {
    int nb_joueurs_courant;
    joueur_t *joueurs[4];
    int port;
    struct sockaddr_in6 adresse_serv_UDP;
    int sock_serv_UDP;
    struct sockaddr_in6 adresse_serv_MDIF;
    int port_MDIF;
    int sock_serv_MDIF;
} partie_t;


// Struct serv

typedef struct liste_parties {
    struct partie *partie;
    struct liste_parties *suivant;
} liste_parties_t;

// Struct client 

typedef struct arg_thread {
    int socket_client;
    // joueur_t *joueur;
    liste_parties_t *liste_2v2;
    liste_parties_t *liste_4adv;
    pthread_mutex_t *verrou;
    fd_set rset;
} arg_thread_t;

int client(const char *argv[]);
void actions(int a, u_int16_t *buf, int sock_UDP, struct sockaddr_in6 servadr_dest);
void abonnementMultidiff (u_int16_t portMDIFF, char *adrMdif);
u_int16_t header(int codereq, int id, int eq);
u_int16_t* tchat_format(int codereq, int id, int eq, int len, char * data);
void messageTchatClient (int sock_TCP, u_int16_t buf[], int tabulation, char data[], int len);

int ncurses(uint16_t *rep_serv, int sock_UDP, int sock_TCP, struct sockaddr_in6 serv_dest);

int client_thread(arg_thread_t *args);

void handle_tchat_serveur (int sock_TCP, fd_set rset);

int serveur(partie_t *p);
void create_sockaddr_mdif(partie_t *p, int port_MDIF);

#endif
