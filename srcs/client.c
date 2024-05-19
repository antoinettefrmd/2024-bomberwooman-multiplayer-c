#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <math.h>
#include "bomberwoman.h"
#include "format.c"

#define SIZE_MESS 1024
static int n_move = 0;

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
    printf("premier send client fonctionnel\n");

    u_int16_t reponse_serveur[6];
    char adrmdif[16] = {0};

         
    /* Attente de la réponse  */
    while ((size_t)octets_recu < sizeof(reponse_serveur)) {
        recu = recv(sock, reponse_serveur + octets_recu, sizeof(reponse_serveur), 0);
        if (recu == -1) 
        {
            perror("Erreur lors de la réception");
            exit(EXIT_FAILURE);
        }
        octets_recu += recu;
    }
    printf("entre deux rev\n");

    octets_recu = 0;
    while ((size_t)octets_recu < sizeof(adrmdif)) {
        recu = recv(sock, adrmdif + octets_recu, sizeof(adrmdif), 0);
        if (recu == -1) 
        {
            perror("Erreur lors de la réception");
            exit(EXIT_FAILURE);
        }
        octets_recu += recu;
        printf("octets recus = %d\n", octets_recu);
    }
    //printf("Réponse du service : %u\n", ntohs(buf2[0] & 0xFFF));

    u_int16_t codereq = (ntohs(reponse_serveur[0]) >> 3) & 0x1FFF;
    u_int16_t id = (ntohs(reponse_serveur[0]) >> 1) & 0x3;
    u_int16_t eq = (ntohs(reponse_serveur[0])) & 0x1;
    u_int16_t portUDP = ntohs(reponse_serveur[1]); /* numéro de port sur lequel le serveur attend les actions en UDP des joueurs */    
    u_int16_t portMDIFF = ntohs(reponse_serveur[2]); /* numéro de port sur lequel le serveur multidiffusera ses messages aux joueurs */
    printf("codereq: %u\n id: %u\n eq: %u\n portUDP: %u\n portMDIFF: %u\n adresseMultiDif : %s\n", codereq, id, eq, portUDP, portMDIFF, adrmdif); 
   
    abonnementMultidiff(portMDIFF,adrmdif);
   
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

   // ncurses(reponse_serveur, sock_UDP, servadr_dest);
    //char buf[25];
    //sprintf(buf, "coucou ça fonctionne !");
    //if (sendto(sock_UDP, buf , strlen(buf), 0, (struct sockaddr *)&servadr_dest, sizeof(servadr_dest))< 0) { printf("sendto failed\n");return -1; }

   //close(sock_UDP);
    //close(sock);
    //return 0;
}

u_int16_t header(int codereq, int id, int eq) {
    u_int16_t res;

    res = 0;
    res |= (u_int16_t)(codereq & 0x1FFF); // 1FF est un masque hexa pour 111111111111 (12 bits)
    res |= (u_int16_t)((id & 0x3) << 13); // l'id est placé sur le bit 13
    res |= (u_int16_t)((eq & 0x1) << 15); // puis eq sur le bit 15
    return (htons(res)); // le tout est ensuite mis au format big endian
}

void actions(int a, u_int16_t *buf, int sock_UDP, struct sockaddr_in6 servadr_dest) {
    //printf("action = %d et codereq = %u\n",a, ntohs(buf[0] & 0xFF00));
     u_int16_t move[2];
    u_int16_t move_1;

    if (ntohs(buf[0] & 0xFF00) == 9)
        move[0] = header(5, (buf[0] >> 13) & 0x3, (buf[0] >> 15) & 0x1);
    else
        move[0] =  header(5, (buf[0] >> 13) & 0x3, 0);        

    move_1 = 0;
    move_1 |= (u_int16_t)((n_move % (int)pow(2, 13)) & 0x1FFF); // le numéro est également codé sur 12 bits
    n_move++;
    move_1 |= (u_int16_t)((a & 0x3) << 13); // action est placé sur le bit 13
    move[1] = htons(move_1); // les deux octets sont mis au format big endian
    if (sendto(sock_UDP, move, sizeof(move), 0, (struct sockaddr *)&servadr_dest, sizeof(servadr_dest)) < 0) {exit (0);}

}

void abonnementMultidiff (u_int16_t portMDIFF, char * adrMdif){

    /* le client doit s'abonner à l'adresseMultiDiff de multidiffusion */
    int sockMdifClient = socket(AF_INET6, SOCK_DGRAM,0);
    if (sockMdifClient < 0){ 
        perror("Socket UDP");
        exit(EXIT_FAILURE);
    }

    // Reconstituer l'adresse IPv6 à partir de rep
    struct sockaddr_in6 adresseMultiDiff;
    memset(&adresseMultiDiff, 0, sizeof(adresseMultiDiff));
    adresseMultiDiff.sin6_family = AF_INET6;
    adresseMultiDiff.sin6_addr = in6addr_any;
    adresseMultiDiff.sin6_port = htons(portMDIFF);
   
    if (bind(sockMdifClient, (struct sockaddr*)&adresseMultiDiff, sizeof(adresseMultiDiff)) < 0) {
        perror("Erreur lors de la liaison de la socket");
        close(sockMdifClient);
    }
    printf("portMDIFF abonnement = %u\n", portMDIFF);
    
    /* abonnement de l'entité au groupe multicast */
    struct ipv6_mreq group;
    inet_pton(AF_INET6, adrMdif, &group.ipv6mr_multiaddr);
  //  memcpy(&group.ipv6mr_multiaddr, &adresseMultiDiff.sin6_addr, sizeof(struct in6_addr));
    group.ipv6mr_interface = if_nametoindex("wlo1"); /* interface réseau multicast par défaut */
    
    if (setsockopt(sockMdifClient, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof(group)) < 0){
        perror("setsockopt");
        close(sockMdifClient);
        exit(EXIT_FAILURE);
    }
    printf("abonnement OK\n");
    
   char buf[SIZE_MESS];
    ssize_t paquet_recu;
    size_t octets_recus = 0;

    memset(buf, 0, SIZE_MESS);

    /* Lecture des messages multicast diffusés par le serveur */
    while (octets_recus < SIZE_MESS) {    
        paquet_recu = read(sockMdifClient, buf + octets_recus, SIZE_MESS - octets_recus);
        if (paquet_recu < 0) {
            perror("Erreur lors de la réception du message");
            close(sockMdifClient);
            exit(EXIT_FAILURE); // Sortie en cas d'erreur de réception.
        }
        //printf("octets recus multidiff = %ld\n", octets_recus);
        // if (paquet_recu == 0) {
        //     printf("La connexion a été fermée par le serveur.\n");
        //     close(sockMdifClient);
        //     break; // Sortie de la boucle si le serveur ferme la connexion.
        // }
        printf("Message reçu du serveur: %.*s\n", (int)paquet_recu, buf + octets_recus);
        octets_recus += paquet_recu;
    }
    close(sockMdifClient);
}
