#ifndef BOMBERWOMAN_H

#define BOMBERWOMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
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
} arg_thread_t;

int client(const char *argv[]);
int ncurses(u_int16_t *buf);
void actions(int a, u_int16_t *buf);
u_int16_t header(int codereq, int id, int eq);
//void c_actions(int a);
void abonnementMultidiff (u_int16_t portMDIFF, u_int16_t reponse_serveur[]);
int ncurses();

int client_thread(arg_thread_t *args);
int serveur(struct sockaddr_in6 addr_server);
#endif