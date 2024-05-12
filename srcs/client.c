#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "bomberwoman.h"
#include "format.c"

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

   u_int16_t reponse_serveur[4];
         
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
/*
    u_int16_t codereq = ntohs(reponse_serveur[0]) & 0x1FFF;
    u_int16_t id = (ntohs(reponse_serveur[0]) >> 13) & 0x3;
    u_int16_t eq = (ntohs(reponse_serveur[0]) >> 15) & 0x1;
    u_int16_t portUDP = ntohs(reponse_serveur[1]); numéro de port sur lequel le serveur attend les actions en UDP des joueurs 
*/    
    u_int16_t portMDIFF = ntohs(reponse_serveur[2]); /* numéro de port sur lequel le serveur multidiffusera ses messages aux joueurs */
   
    abonnementMultidiff(portMDIFF,reponse_serveur);
   
    close(sock);
    return 0;
}

void abonnementMultidiff (u_int16_t portMDIFF, u_int16_t reponse_serveur[]){
    
    /* le client doit s'abonner à l'adresseMultiDiff de multidiffusion */
    int sockUDP = socket(PF_INET6, SOCK_DGRAM,0);
    if (sockUDP < 0){ 
        perror("Socket UDP");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in6 adresseMultiDiff;
    memset(&adresseMultiDiff, 0, sizeof(adresseMultiDiff));
    adresseMultiDiff.sin6_family = AF_INET6;
    adresseMultiDiff.sin6_addr = in6addr_any;
    adresseMultiDiff.sin6_port = htons(portMDIFF);
    memcpy(adresseMultiDiff.sin6_addr.s6_addr,reponse_serveur+3,sizeof(adresseMultiDiff.sin6_addr.s6_addr));
   
    char adresse_str[INET6_ADDRSTRLEN];
    if (inet_ntop(AF_INET6, &adresseMultiDiff.sin6_addr, adresse_str, INET6_ADDRSTRLEN) == NULL) {
        perror("inet_ntop");
        exit(EXIT_FAILURE);
    }
    printf("Adresse de multidiffusion : %s\n", adresse_str);
    
    /* liaison de la socket au port pour permettre la réception des paquets */
    if(bind(sockUDP, (struct sockaddr*)&adresseMultiDiff, sizeof(adresseMultiDiff))) {
        perror("bind");
        close(sockUDP);
        exit(EXIT_FAILURE);
    }   

    /* abonnement de l'entité au groupe multicast */
    struct ipv6_mreq group;
    memcpy(&group.ipv6mr_multiaddr, &adresseMultiDiff.sin6_addr, sizeof(struct in6_addr));
    group.ipv6mr_interface = if_nametoindex("en0"); /* interface réseau multicast sur ma machine */
    
    if(setsockopt(sockUDP, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof(group)) < 0){
        perror("setsockopt");
        close(sockUDP);
        exit(EXIT_FAILURE);
    }

    printf("abonnement OK\n");
    close(sockUDP);
    
    /* lecture des messages multicast diffusé par le serveur *
    while (1){    
        ssize_t paquet_recu = read(sockUDP, buf, MAX_BUF);
        if (paquet_recu < 0){
            perror("erreur recvfrom");
            close(sockUDP);
            exit(EXIT_FAILURE);
        }

        printf("Message reçu du serveur: %.*s\n", (int)paquet_recu, buf);
    }*/
}
/*
void messageTchatClient (u_int16_t buf[], int tabulation, char data[], int len) {
    
    u_int16_t codereq = ntohs(buf[0]) & 0x1FFF;
    u_int16_t id = (ntohs(buf[0]) >> 13) & 0x3;
    u_int16_t eq = (ntohs(buf[0]) >> 15) & 0x1;
   
    if (codereq == 9) { // si on est en mode 4 joueurs tabulation est forcément égal à 7
        tabulation = 7;
    }
    
    formatage du message 
    u_int16_t* message = tchat_format(tabulation, id, eq, len, data);
    
    int paquets_envoyes = 0; 
    int res_send;
    size_t taille_message = (2 + (len / 2)) * sizeof(u_int16_t);

     envoie du message 
    while ((size_t)paquets_envoyes < sizeof(taille_message)) {
        
        res_send = send(sock, message, sizeof(taille_message - paquets_envoyes), 0);
       
       if (res_send == -1) {
            perror("Erreur lors de l'envoi du message");
            exit(EXIT_FAILURE);
        }
        if (res_send == 0) break;
        paquets_envoyes += res_send;  
    }
   
    free(message);
}
*/
/* méthode formatage des messages du tchat */
//header de chaque message
u_int16_t header(int codereq, int id, int eq) {
    u_int16_t res;

    res = 0;
    res |= (u_int16_t)(codereq & 0x1FFF); // 1FF est un masque hexa pour 111111111111 (12 bits)
    res |= (u_int16_t)((id & 0x3) << 13); // l'id est placé sur le bit 13
    res |= (u_int16_t)((eq & 0x1) << 15); // puis eq sur le bit 15
    return (htons(res)); // le tout est ensuite mis au format big endian
}

u_int16_t* tchat_format(int codereq, int id, int eq, int len, char * data) {
    
    u_int16_t *tchat = malloc((2 + (len / 2)) * sizeof(u_int16_t));

  //  u_int16_t tchat[2 + (len / 2)]; // comme chaque caractère est sur un octet, on divise par deux
                                    //le nombre de lignes de 16 bits à remplir
    u_int16_t tchat_1;
    u_int16_t tchat_i;
    int j = 1;

    tchat[0] = header(codereq, id, eq); // on place le header sur la première ligne

    tchat_1 = 0;
    tchat_1 |= (u_int16_t)(len & 0xFF); // le champ data est codé sur 8 bits (FF est le masque hexa pour 11111111)
    tchat_1 |= (u_int16_t)((data[0]) << 8); // le premier caractère est placé sur le bit 8
    tchat[1] = htons(tchat_1); // les deux octets sont au format big endiant
    for (int i = 2; i < 2 + (len / 2); i++) { // on boucle sur le message
        tchat_i = 0;
        tchat_i |= (u_int16_t)(data[j] & 0xFF); // chaque caractère est codé sur un octet
        if (i != len - 1 || len % 2 == 1)
            tchat_i |= (u_int16_t)((data[j + 1] & 0xFF) << 8); // le prochain caractère rempli le second octet
        j += 2;
        tchat[i] = tchat_i; // puis on place les deux octets sur la ligne d'indice i
    }
    return tchat;
   
}