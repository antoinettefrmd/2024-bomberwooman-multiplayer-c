#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "bomberwoman.h"

#define SIZE_MESS 1024

int main (int argc, const char *argv[]) {

    if(argc != 2) {
        perror("Erreur : Veuillez ajouter en argument 2 pour une partie 2v2 ou 4 pour une partie chacun pour soi.");
        exit(1);
    }

    int octets_recu = 0;
    ssize_t recu = 0;

    int sock = socket(PF_INET6,SOCK_STREAM,0);
    if (sock < 0){
        perror("socket failure");
    }

    struct sockaddr_in6 adresse; /* adresse IP */
    memset(&adresse,0,sizeof(adresse));
   
    adresse.sin6_family = AF_INET6;
    adresse.sin6_port = htons(1124);

    // Utilisation de l'adresse IPv6 locale ::1
    if (inet_pton(AF_INET6, "::1", &adresse.sin6_addr) <= 0) {
        perror("inet_pton failure");
        exit(EXIT_FAILURE);
    }    
    int r = connect (sock, (struct sockaddr *) &adresse, sizeof(adresse));
    
    /* 0 en cas de succès et -1 sinon */
    if (r < 0){
        perror("connection failed");
        exit(EXIT_FAILURE);
    }
   
    u_int16_t req[1];
    memset(&req, 0, sizeof(req));
    req[0] = htons(atoi(argv[1]));
    
    int paquets_envoyes = 0; 
    int res_send;
   
    // Envoi du message
    while ((size_t)paquets_envoyes < sizeof(req)) 
    {
        res_send = send(sock, req, sizeof(req), 0);
        if (res_send == -1) {
            perror("Erreur lors de l'envoi du message");
            exit(EXIT_FAILURE);
        }
        if (res_send == 0) break;
        paquets_envoyes += res_send;  
    }

    u_int16_t reponse_serveur[11];
         
    /* Attente de la réponse  */
    while ((size_t)octets_recu < sizeof(reponse_serveur)) {
        recu = recv(sock, reponse_serveur + octets_recu, SIZE_MESS, 0);
        if (recu == -1) 
        {
            perror("Erreur lors de la réception");
            exit(EXIT_FAILURE);
        }
        octets_recu += recu;
    }

    u_int16_t codereq = ntohs(reponse_serveur[0]) & 0x1FFF;
    u_int16_t id = (ntohs(reponse_serveur[0]) >> 13) & 0x3;
    u_int16_t eq = (ntohs(reponse_serveur[0]) >> 15) & 0x1;
    u_int16_t portUDP = ntohs(reponse_serveur[1]); /* numéro de port sur lequel le serveur attend les actions en UDP des joueurs */
    u_int16_t portMDIFF = ntohs(reponse_serveur[2]); /* numéro de port sur lequel le serveur multidiffusera ses messages aux joueurs */
    printf("codereq: %u\n id: %u\n eq: %u\n portUDP: %u\n portMDIFF: %u\n ", codereq, id, eq, portUDP, portMDIFF); 
   
    abonnementMultidiff(portMDIFF,reponse_serveur);
   
    int sock_UDP = socket(PF_INET6, SOCK_DGRAM, 0);
    if (sock_UDP < 0){ perror("socket failure"); }

    struct sockaddr_in6 servadr_dest;
    memset(&servadr_dest, 0, sizeof(servadr_dest));
    servadr_dest.sin6_family = AF_INET6;
    if (inet_pton(AF_INET6, "::1", &servadr_dest.sin6_addr) != 1) {
        perror("Erreur lors de la conversion de l'adresse IP");
        return -1;
    }
    servadr_dest.sin6_port = htons(portUDP);

    char buf[25];
    sprintf(buf, "coucou ça fonctionne !");
    if (sendto(sock_UDP, buf , strlen(buf), 0, (struct sockaddr *)&servadr_dest, sizeof(servadr_dest))< 0) { return -1; }

    close(sock_UDP);
    close(sock);
    return 0;
}

void abonnementMultidiff (u_int16_t portMDIFF, u_int16_t reponse_serveur[]){
    
    /* le client doit s'abonner à l'adresseMultiDiff de multidiffusion */
    int sockUDP = socket(AF_INET6, SOCK_DGRAM,0);
    if (sockUDP < 0){ 
        perror("Socket UDP");
        exit(EXIT_FAILURE);
    }

    // Reconstituer l'adresse IPv6 à partir de rep
    struct sockaddr_in6 adresseMultiDiff;
    memset(&adresseMultiDiff, 0, sizeof(adresseMultiDiff));
    adresseMultiDiff.sin6_family = AF_INET6;
    adresseMultiDiff.sin6_addr = in6addr_any;
    for (int i = 0; i < 11; ++i) {
        adresseMultiDiff.sin6_addr.s6_addr[i * 2] = (reponse_serveur[i+3] >> 8) & 0xFF;
        adresseMultiDiff.sin6_addr.s6_addr[i * 2 + 1] = reponse_serveur[i+3] & 0xFF;
    }
    adresseMultiDiff.sin6_port = htons(portMDIFF);

    // Affichage pour vérifier l'adresse de multidiffusion
    char adresse_str[INET6_ADDRSTRLEN];
    if (inet_ntop(AF_INET6, &adresseMultiDiff.sin6_addr, adresse_str, INET6_ADDRSTRLEN) == NULL) {
        perror("inet_ntop");
        close(sockUDP);
    }
    printf("Adresse de multidiffusion client : %s\n", adresse_str);
   
    /* Lier la socket à toute interface et au port de multidiffusion*/
    struct sockaddr_in6 localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin6_family = AF_INET6;
    localAddr.sin6_addr = in6addr_any;
    localAddr.sin6_port = htons(portMDIFF); 

    if (bind(sockUDP, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
        perror("Erreur lors de la liaison de la socket");
        close(sockUDP);
    }
    printf("portMDIFF abonnement = %u\n", portMDIFF);
    
    /* abonnement de l'entité au groupe multicast */
    struct ipv6_mreq group;
    memcpy(&group.ipv6mr_multiaddr, &adresseMultiDiff.sin6_addr, sizeof(struct in6_addr));
    group.ipv6mr_interface = if_nametoindex("eth0"); /* interface réseau multicast par défaut */
       
    if( setsockopt(sockUDP, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof(group)) < 0){
        perror("setsockopt");
        close(sockUDP);
        exit(EXIT_FAILURE);
    }
    printf("abonnement OK\n");
    
    char buf[1024];
    ssize_t paquet_recu;
    struct sockaddr_in6 expediteur;
    socklen_t addrlen = sizeof(expediteur);

    /* Lecture des messages multicast diffusés par le serveur */
    while (1){    
        paquet_recu = recvfrom(sockUDP, buf, sizeof(buf),0,(struct sockaddr *)&expediteur, &addrlen);
        if (paquet_recu < 0){
            perror("Erreur lors de la réception du message");
            close(sockUDP);
        }
        printf("Message reçu du serveur: %.*s\n", (int)paquet_recu, buf);
    }

    close(sockUDP);
}
