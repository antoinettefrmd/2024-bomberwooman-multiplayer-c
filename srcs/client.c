#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// #define SIZE_MESS 100

int client (const char *argv[]) {

    int sock = socket(PF_INET6,SOCK_STREAM,0);
    if (sock < 0){
        perror("socket failure");
    }

    struct sockaddr_in6 adresse; /* adresse IP */
    memset(&adresse,0,sizeof(adresse));
   
    adresse.sin6_family = AF_INET6;
    adresse.sin6_port = htons(1124); /* numéro de port du service echo dans cat /etc/services */

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

    // char buf[SIZE_MESS];
    // memset(buf, 0, SIZE_MESS);
    
    // char buf2[SIZE_MESS];
    // memset(buf2, 0, SIZE_MESS);
    
    u_int16_t req[16];
    req[0] = htons(atoi(argv[1]));
    req[1] = 0; 
    req[1]<<= 12;
    // req[2] = 0;
    
    int paquets_envoyes = 0; 
    int res_send;
    // Envoi du message
    while ((size_t)paquets_envoyes < 16) 
    {
        res_send = send(sock, req + paquets_envoyes, 16, 0);
        if (res_send == -1) 
        {
            perror("Erreur lors de l'envoi du message");
            exit(EXIT_FAILURE);
        }
        if (res_send == 0) break;
        paquets_envoyes += res_send;  
    }
         
    // // Attente de la réponse
    // if (recv(sock, buf2, SIZE_MESS, 0) > 0) {
    //     printf("Réponse du service : %s\n", buf2);
    // } else {
    //     perror("Réception de la réponse échouée");
    //     exit(EXIT_FAILURE);
    // }
    close(sock);


    return 0;
}