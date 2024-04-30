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

#define MULTICAST_GROUP "ff12::1" /* fait partie de la plage d'adresses multicast réservées pour les liaisons de liaison locale */
#define PORT 12121 /* quel port utiliser ?*/

int serveur() {
    
    /* déclaration d'une socket UDP IPv6 */
    int sock = socket(AF_INET6, SOCK_DGRAM,0); 
    if (sock < 0){
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }

    /* initialisation de l'adresse multicast du groupe (IP + PORT) */
    struct sockaddr_in6 addr_server;
    memset(&addr_server, 0, sizeof(addr_server));
    addr_server.sin6_family = AF_INET6;
    addr_server.sin6_port = htons(PORT);
    addr_server.sin6_addr = in6addr_any; // Utilise toutes les interfaces disponibles
   
    /* Liaison de la socket à une interface réseau spécifique
    int ifindex = if_nametoindex("eth0");
    addr_server.sin6_scope_id = ifindex;
    */

    int ifindex = if_nametoindex("eth0");
   
    if(setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifindex, sizeof(ifindex)) == -1) {
        perror("erreur initialisation de l’interface locale");
        exit(EXIT_FAILURE);
    }

    /* Liaison de la socket au port */
    if (bind(sock, (struct sockaddr *)&addr_server, sizeof(addr_server)) < 0) {
        perror("Erreur lors de la liaison de la socket au port");
        exit(EXIT_FAILURE);
    }
    
   
    /*
    
    */

    close(sock);
    return 0;
}