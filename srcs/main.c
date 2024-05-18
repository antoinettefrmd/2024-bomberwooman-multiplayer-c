#include "bomberwoman.h"

int main() {

    int nb_thread = 0;
    pthread_t tpthread[10];
    pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;

    arg_thread_t args;
    memset(&args, 0, sizeof(args));
    args.verrou = &verrou;
    
    liste_parties_t *parties_2v2;
    liste_parties_t *parties_4_adv;

    args.socket_client = -1;

    parties_2v2 = malloc(sizeof(liste_parties_t));
    parties_4_adv = malloc(sizeof(liste_parties_t));
    
    if (parties_2v2 == NULL || parties_4_adv == NULL) {
        perror("malloc failed");
        exit(1);
    }

    memset(parties_2v2, 0, sizeof(liste_parties_t));
    memset(parties_4_adv, 0, sizeof(liste_parties_t));

    args.liste_2v2 = parties_2v2;
    args.liste_4adv = parties_4_adv;

    /* creation de la socket serveur */
    int sock = socket(PF_INET6, SOCK_STREAM, 0);
    if(sock < 0){ perror("creation socket"); exit(1);}
    
    /* creation de l’adresse du destinataire (serveur) */
    struct sockaddr_in6 adrsock;
    memset(&adrsock, 0, sizeof(adrsock));
    adrsock.sin6_family = AF_INET6;
    adrsock.sin6_port = htons(1124); 
    adrsock.sin6_addr = in6addr_any; // une variable de type struct in6_addr qui contient l’adresse locale au format IPv6 avec octets déjà dans l’ordre réseau

    int ok = 1;
    int ra = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok));
    if (ra < 0) {
        perror("Erreur sockopt");
        exit(EXIT_FAILURE);
    }
    /* lier la socket à un numéro de port */
    int r = bind(sock, (struct sockaddr *) &adrsock, sizeof(adrsock));

    if (r < 0) { perror("bind failed"); exit(EXIT_FAILURE);}

    /* Le serveur se déclare prêt à écouter les connexions sur le port */
    int r2 = listen(sock, 0);
    if (r2 == -1) 
    {
        perror("Le serveur n'a pas réussi à se mettre sur écoute");
        exit(-1);
    }

    /* on récupère l'adresse du client */
    struct sockaddr_in adrclient;
    socklen_t size = sizeof(adrclient);  
    while(1) {    
        /* pour accepter la demande de connexion d'un client */
        int sockclient = accept(sock, (struct sockaddr *) &adrclient, &size);
        if(sockclient  >= 0) {
            char addr_buf[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &(adrclient.sin_addr), addr_buf, sizeof(addr_buf));
        }
        printf("[*] Connexion établie avec %s:%d\n", inet_ntoa(adrclient.sin_addr), ntohs(adrclient.sin_port));

        args.socket_client = sockclient;

        // Chaque client va s'éxecuter dans un thread
        if (pthread_create(&tpthread[nb_thread], NULL, (void *)client_thread,(void *)&args) < 0) {perror("Création thread"); exit(1);}
        nb_thread++;
    }

    for(int i=0; i<15; i++)
        pthread_join(tpthread[i], NULL);
    
    close(sock);
    return 0;
}


// diviser les deux fonctions en sous-fonctions, par exemple pour recevoir ou envoyer un message