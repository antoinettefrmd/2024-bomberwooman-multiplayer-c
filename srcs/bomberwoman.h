#ifndef BOMBERWOMAN_H

#define BOMBERWOMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Struct joueur

typedef struct joueur {
    int id;
    int id_equipe;// à voir si on garde le -1 dans le point h
    struct sockaddr_in6 adresse; // adresse du joueur 
} joueur_t;

// Struct partie

typedef struct partie {
    int nb_joueurs_courant;
    joueur_t joueurs[4];
    struct sockaddr_in6 adresse; // adresse de la partie
} partie_t;


// Struct serv

typedef struct liste_parties {
    struct partie partie;
    struct liste_parties *suivant;
} liste_parties_t;





int client(const char *argv[]);

#endif