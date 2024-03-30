#ifndef BOMBERWOMAN_H

#define BOMBERWOMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// ça ne fonctionne pas encore
// // Struct serv

// struct liste_parties {
//     struct partie partie;
//     struct liste_parties *suivant;
// } liste_parties_t;

// // Struct partie

// struct partie {
//     int nb_joueurs_courant;
//     struct sockaddr_in6 adresse;
// } partie_t;



int client(const char *argv[]);

#endif