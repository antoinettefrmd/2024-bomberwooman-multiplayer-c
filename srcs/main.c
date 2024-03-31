#include "bomberwoman.h"

// #define BUFFER_SIZE 1024

int main(int argc, const char *argv[]) {
    
    int portUDP = 1111;
    int portMDIFF = 4321;
   
    int messageRecu;
    int res_recv = 0;

    u_int16_t req[1];
    memset(&req, 0, sizeof(req));

    liste_parties_t *partie_2v2 = malloc(sizeof(liste_parties_t));
    liste_parties_t *partie_4_adv = malloc(sizeof(liste_parties_t));
    
    if (partie_2v2 == NULL || partie_4_adv == NULL) {
        perror("malloc failed");
        exit(1);
    }

    memset(partie_2v2, 0, sizeof(liste_parties_t));
    memset(partie_4_adv, 0, sizeof(liste_parties_t));

    if(argc != 2) 
    {
        perror("Commence par rentrer les bons arguments");
        exit(1);
    }

    /* creation de la socket serveur */
    int sock = socket(PF_INET6, SOCK_STREAM, 0);
    if(sock < 0){ perror("creation socket"); exit(1);}
    
    /* creation de l’adresse du destinataire (serveur) */
    struct sockaddr_in6 adrsock;
    memset(&adrsock, 0, sizeof(adrsock));
    adrsock.sin6_family = AF_INET6;
    adrsock.sin6_port = htons(1124); // le port du service echo
    adrsock.sin6_addr = in6addr_any; // une variable de type struct in6_addr qui contient l’adresse locale au format IPv6 avec octets déjà dans l’ordre réseau

    /* lier la socket à un numéro de port */
    int r = bind(sock, (struct sockaddr *) &adrsock, sizeof(adrsock));

    if (r < 0) { perror("bind failed"); exit(EXIT_FAILURE);}

    /* Le serveur se déclare prêt à écouter les connexions sur le port */
    int r2 = listen(sock, 0);
    if (r2 == -1) 
    {
        perror("bon bah là c'est pas de t faute (je crois)");
        exit(-1);
    }

    /* on récupère l'adresse du client */
    struct sockaddr_in adrclient;
    socklen_t size = sizeof(adrclient);  

    client(argv);

    while(1) {
        /* pour accepter la demande de connexion d'un client */
        int sockclient = accept(sock, (struct sockaddr *) &adrclient, &size);
        if(sockclient  >= 0) {
            char addr_buf[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &(adrclient.sin_addr), addr_buf, sizeof(addr_buf));
        }
        printf("[*] Connexion établie avec %s:%d\n", inet_ntoa(adrclient.sin_addr), ntohs(adrclient.sin_port));
        joueur_t j;
        j.id = 1;
        while ((size_t)res_recv < sizeof(req))
        {
            messageRecu = recv(sockclient, req, 1, 0);
            if (messageRecu == -1) 
            {
                perror("Erreur lors de l'envoi du message");
                exit(EXIT_FAILURE);
            }
            if (messageRecu == 0) 
            {
               break;
            }
            res_recv += messageRecu;
        } 

        if (messageRecu == -1) {
            perror("Erreur lors de la réception");
            exit(EXIT_FAILURE);
        }

        partie_t p;
        p.joueurs[0] = j;
       
        u_int16_t rep[4];
        memset(&rep, 0, sizeof(u_int16_t)*4);


        if (req[0] == 2) 
        {
            partie_2v2->partie = p;
            rep[0] = htons(10);
            if (p.nb_joueurs_courant < 2){
                rep[0] |= 0;
            } else {
                rep[0] |= 1;
            }     
        }
        else 
        {
            partie_4_adv->partie = p;
            rep[0] = htons(9);
        }

        u_int16_t id = j.id;
        rep[0] |= id << 1; 
        rep[1] = htons(portUDP);
        rep[2] = htons(portMDIFF);
        
        struct sockaddr_in6 adresseMultiDiff;
        memset(&adresseMultiDiff, 0, sizeof(adresseMultiDiff));
        adresseMultiDiff.sin6_family = AF_INET6;
        inet_pton(AF_INET6,"ff12::1:2:3", &adresseMultiDiff);
        adresseMultiDiff.sin6_port = htons(portMDIFF);
        
     //  int sock = socket(AF_INET6,SOCK_DGRAM,0);

       // rep[3] = adresseMultiDiff; // adresse à laquelle les joueurs s'abonnent

        close(sockclient);
        for(int i = 0 ; i < 1 ; i++) {
            printf("%hx\n",req[i]);
        } 
    }
    
    close(sock);
    return 0;
}