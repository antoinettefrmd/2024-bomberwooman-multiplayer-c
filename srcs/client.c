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
#include "ncurse.h"
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

    // ncurses(reponse_serveur, sock_UDP, sock, servadr_dest);
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
        move[0] = header(5, (buf[0] >> 13) & 0x7, (buf[0] >> 15) & 0x1);
    else
        move[0] =  header(5, (buf[0] >> 13) & 0x7, 0);        

    move_1 = 0;
    move_1 |= (u_int16_t)((n_move % (int)pow(2, 13)) & 0x1FFF); // le numéro est également codé sur 12 bits
    n_move++;
    move_1 |= (u_int16_t)((a & 0x7) << 13); // action est placé sur le bit 13
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

    int ok = 1;
    if (setsockopt(sockMdifClient, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0){
        perror("setsockopt");
        close(sockMdifClient);
        exit(EXIT_FAILURE);
    }

    if (bind(sockMdifClient, (struct sockaddr*)&adresseMultiDiff, sizeof(adresseMultiDiff)) < 0) {
        perror("Erreur lors de la liaison de la socket");
        close(sockMdifClient);
    }
    printf("portMDIFF abonnement = %u\n", portMDIFF);
    
    /* abonnement de l'entité au groupe multicast */
    struct ipv6_mreq group;
    inet_pton(AF_INET6, adrMdif, &group.ipv6mr_multiaddr);
  //  memcpy(&group.ipv6mr_multiaddr, &adresseMultiDiff.sin6_addr, sizeof(struct in6_addr));
    int ifindex = if_nametoindex("wlp0s20f3");
    if (ifindex < 0) {perror("erreur interface"); close(sockMdifClient); exit(EXIT_FAILURE);}
    group.ipv6mr_interface = 0; /* interface réseau multicast par défaut */
  
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
    paquet_recu = read(sockMdifClient, buf + octets_recus, SIZE_MESS - octets_recus);
    if (paquet_recu < 0) {
        perror("Erreur lors de la réception du message");
        close(sockMdifClient);
        exit(EXIT_FAILURE); // Sortie en cas d'erreur de réception.
    }
    // octets_recus+=paquet_recu;
    printf("Message reçu du serveur: %.*s\n", (int)paquet_recu, buf);

    // --------------------------------------------------------------------------------------------------
    uint16_t code[3];

    paquet_recu = read(sockMdifClient, code, sizeof(code));
    if (paquet_recu < 0) {
        perror("Erreur lors de la réception du message");
        close(sockMdifClient);
        exit(EXIT_FAILURE); // Sortie en cas d'erreur de réception.
    }

    int hauteur = code[2] >> 8;
    int largeur = code[2] & 0xFF;
    // printf("largeur : %d ; hauteur : %d \n", largeur, hauteur);

    u_int16_t grille[largeur*hauteur];
    octets_recus = 0;

    paquet_recu = read(sockMdifClient, grille, largeur*hauteur);
    if (paquet_recu < 0) {
        perror("Erreur lors de la réception du message");
        close(sockMdifClient);
        exit(EXIT_FAILURE); // Sortie en cas d'erreur de réception.
    }

    char * grid = malloc(largeur *hauteur);

    for (int i = 0 ; i < hauteur ; i++)
    {
        for(int j = 0 ; j < largeur ; j++)
        {
            if (j%2 == 0)  grid[i*largeur+j] = (grille[i] >> 8);
            else grid[i*largeur+j] = (grille[i] & 0xFF);
        }
    }
    board *b = malloc(sizeof(board));
    memset(b,0,sizeof(board));
    b->h = hauteur;
    b->w = largeur;
    b->grid = grid;
    

    printf("largeur : %d ; hauteur : %d ; grille : %s\n", largeur, hauteur, grid);


    close(sockMdifClient);
}

/* méthode formatage d'un message du tchat*/
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

void messageTchatClient (int sock_TCP, u_int16_t buf[], int tabulation, char data[], int len) {
    (void)sock_TCP;
    (void)buf;
    (void)tabulation;
    (void)data;
    (void)len;
    
    // u_int16_t codereq = ntohs(buf[0]) & 0x1FFF;
    // u_int16_t id = (ntohs(buf[0]) >> 13) & 0x3;
    // u_int16_t eq = (ntohs(buf[0]) >> 15) & 0x1;
    // printf("message: %s\n",data);

    // if (codereq == 9) { // si on est en mode 4 joueurs tabulation est forcément égal à 7
    //    tabulation = 7;
    // }
    
    // u_int16_t *message = tchat_format(tabulation, id, eq, len, data);
    
    // int paquets_envoyes = 0; 
    // int res_send;
    // size_t taille_message = (2 + (len / 2)) * sizeof(u_int16_t);

    // int mess_len = len;
    // if (send(sock_TCP, &mess_len, sizeof(int), MSG_NOSIGNAL | MSG_DONTWAIT) < 0){
    //     perror("send1");
    //     exit(EXIT_FAILURE);
    // }
    // while ((size_t)paquets_envoyes < sizeof(taille_message)) {

    //     res_send = send(sock_TCP, message, sizeof(taille_message - paquets_envoyes), 0);
       
    //    if (res_send == -1) {
    //         perror("Erreur lors de l'envoi du message");
    //         exit(EXIT_FAILURE);
    //     }
    //     if (res_send == 0) break;
    //     paquets_envoyes += res_send;  
    // }

    // printf("envoie tchat OK\n");
   
    // free(message);
    //     //printf("octets recus multidiff = %ld\n", octets_recus);
    //     // if (paquet_recu == 0) {
    //     //     printf("La connexion a été fermée par le serveur.\n");
    //     //     close(sockMdifClient);
    //     //     break; // Sortie de la boucle si le serveur ferme la connexion.
    //     // }
    //     printf("Message reçu du serveur: %.*s\n", (int)paquet_recu, buf + octets_recus);
    //     octets_recus += paquet_recu;
    // }
}
