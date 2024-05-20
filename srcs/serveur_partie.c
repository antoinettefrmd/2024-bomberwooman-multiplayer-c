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
#include <sys/select.h>
#include "bomberwoman.h"

#define SIZE_MESS 1024
#define PORT 12121 /* quel port utiliser ?*/

static int PORT_UDP = 1234;
int PORT_MDIF = 4321;

int rajoute_joueur_partie(liste_parties_t *lp, int type_4, int sock_client) // rajouter un gros lock sur la fonction
{
    joueur_t *j = malloc(sizeof(joueur_t));    
    j -> sock_client = sock_client;
    
    liste_parties_t *courante = lp;

    if (courante->partie == NULL)
    {
        int sock_serv_UDP = socket(PF_INET6, SOCK_DGRAM, 0);
        if (sock_serv_UDP < 0) return -1;

        struct sockaddr_in6 servadr;
        memset(&servadr, 0, sizeof(servadr));
        servadr.sin6_family = AF_INET6;
        if (inet_pton(AF_INET6, "::1", &servadr.sin6_addr) == -1) {
            printf("inet_pton non réussi\n");
            return -1;
        }
        servadr.sin6_port = htons(PORT_UDP);

        bind(sock_serv_UDP, (struct sockaddr*)&servadr, sizeof(servadr));

        partie_t *p = malloc(sizeof(partie_t));
        j->id = 0;
        j->id_equipe = 0;
        p->joueurs[0] = j;
        p->nb_joueurs_courant = 1;
        p->port = PORT_UDP;
        p->adresse_serv_UDP = servadr;
        p->sock_serv_UDP = sock_serv_UDP;
        lp->partie = p;
        PORT_UDP++;   
        create_sockaddr_mdif(p, PORT_MDIF);
        PORT_MDIF++;
    }
    else 
    {
        while (courante->suivant != NULL)
        {
            courante = courante->suivant;
        }
        partie_t *p_courante = courante->partie;
        if (p_courante->nb_joueurs_courant < 4)
        {
            j->id = p_courante->nb_joueurs_courant;
            if (!type_4) 
            {
                if (p_courante->nb_joueurs_courant < 2) {j->id_equipe = 0;}
                else {j->id_equipe = 1;}
            }
            p_courante->joueurs[p_courante->nb_joueurs_courant] = j;
            p_courante->nb_joueurs_courant+=1;
        }
        else
        {

            int sock_serv_UDP = socket(PF_INET6, SOCK_DGRAM, 0);
            if (sock_serv_UDP < 0) return -1;
            struct sockaddr_in6 servadr;
            memset(&servadr, 0, sizeof(servadr));
            servadr.sin6_family = AF_INET6;
            inet_pton(AF_INET6, "::1", &servadr.sin6_addr);
            servadr.sin6_port = htons(PORT_UDP);
            if (bind(sock_serv_UDP, (struct sockaddr *)&servadr, sizeof(servadr)) < 0) return -1;

            partie_t *p = malloc(sizeof(partie_t));
            j->id=0;
            j->id_equipe = 0;
            p->joueurs[0] = j;
            p->nb_joueurs_courant = 1;
            p->port = PORT_UDP;
            liste_parties_t *lp2 = malloc(sizeof(liste_parties_t));
            lp2->partie = p;
            p->adresse_serv_UDP = servadr;
            p->sock_serv_UDP = sock_serv_UDP;
            courante->suivant = lp2;
            PORT_UDP++; 
            create_sockaddr_mdif(p, PORT_MDIF);
            PORT_MDIF++;           
        }
    }
    return 0;
}


void create_sockaddr_mdif(partie_t *p, int port_MDIF)
{
    int sock = socket(PF_INET6, SOCK_DGRAM,0); 
    if (sock < 0){
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in6 adresseMultiDiff;
    memset(&adresseMultiDiff, 0, sizeof(adresseMultiDiff));
    adresseMultiDiff.sin6_family = AF_INET6;
    inet_pton(AF_INET6,"ff12::1:2:3", &adresseMultiDiff.sin6_addr);
    adresseMultiDiff.sin6_port = htons(port_MDIF);
    printf("portMDIFF serveur = %u\n", port_MDIF);
    p->adresse_serv_MDIF = adresseMultiDiff;
    p->port_MDIF = port_MDIF;


    /* Liaison de la socket à une interface réseau spécifique */
    int ifindex = if_nametoindex("wlp0s20f3");
    if (ifindex < 0) {perror("erreur interface"); close(sock); exit(EXIT_FAILURE);}
    adresseMultiDiff.sin6_scope_id = ifindex;

    int ok = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0) {
        perror("echec de SO_REUSEADDR");
        close(sock);
    }

    p->sock_serv_MDIF = sock;
}


int client_thread(arg_thread_t *args) 
{
    int sock_client = args->socket_client;

    liste_parties_t *p_2v2 = args->liste_2v2;
    liste_parties_t *p_4_adv = args->liste_4adv;

    int messageRecu;
    int res_recv = 0;
    
    u_int16_t req[1];
    memset(&req, 0, sizeof(req));

    while ((size_t)res_recv < sizeof(req))
    {
        messageRecu = recv(sock_client, req, 1, 0);
        if (messageRecu == -1) 
        {
            perror("Erreur lors de la reception du message");
            return 0;
            // exit(EXIT_FAILURE);
        }
        if (messageRecu == 0) 
        {
            break;
        }
        res_recv += messageRecu;
    }
    printf("premier recv serveur fonctionnel\n");

    if (messageRecu == -1) {
        perror("Erreur lors de la réception");
        return 0;
        // exit(EXIT_FAILURE);
    }

    u_int16_t rep_0 = 0;
    char adrMdif[16] = {0};
    memcpy(adrMdif, "ff12::1:2:3", strlen("ff12::1:2:3"));
    u_int16_t rep[6];
    memset(&rep, 0, sizeof(rep));


    liste_parties_t *courante;
    pthread_mutex_lock(args->verrou);
    if (req[0] == 2) 
    { 
        rajoute_joueur_partie (p_2v2, 0, sock_client);
        rep_0 |= 10 << 3;
        courante = p_2v2;
    }
    else 
    {
        rajoute_joueur_partie(p_4_adv, 1, sock_client);
        rep_0 |= 9 << 3;
        courante = p_4_adv;
    }
    pthread_mutex_unlock(args->verrou);
    printf("post mutex\n");

    while(courante->suivant != NULL && courante->suivant->partie != NULL)
    {
        courante = courante->suivant;
    }
    
    int nb_joueurs = courante->partie->nb_joueurs_courant;
    u_int16_t id = ( nb_joueurs - 1);
    rep_0 |= id << 1;
    rep_0 |= courante->partie->joueurs[nb_joueurs-1]->id_equipe;
    rep[0] = htons(rep_0);

    rep[1] = htons(courante->partie->port);
    rep[2] = htons(courante->partie->port_MDIF);
    

    int reponse = 0;
    ssize_t envoi;
    while((size_t)reponse < sizeof(rep)) 
    {
        envoi = send(sock_client, rep + reponse, sizeof(rep) - reponse, 0);
        if (envoi == -1) 
        {
            perror("Erreur lors de l'envoi du message");
            exit(EXIT_FAILURE);
        }
        if (envoi == 0) 
        {
            break;
        }
        reponse += envoi;
    }
    
    reponse = 0;
    envoi = 0;
    while((size_t)reponse < sizeof(adrMdif)) 
    {
        envoi = send(sock_client, adrMdif + reponse, sizeof(adrMdif)-reponse, 0);
        if (envoi == -1) 
        {
            perror("Erreur lors de l'envoi du message");
            exit(EXIT_FAILURE);
        }
        if (envoi == 0) 
        {
            break;
        }
        reponse += envoi;
    }
    printf("post second send serveur\n");
    
    messageRecu = 0;
    res_recv = 0;
    char buf[6];
    memset(buf,0,sizeof(buf));

    while ((size_t)res_recv < sizeof(buf)) {
        messageRecu = recv(sock_client, buf+res_recv, sizeof(buf), 0);
        if (messageRecu < 0) {
            perror("Erreur lors de la réception");
            exit(EXIT_FAILURE);
        }
        res_recv += messageRecu;
    }

    if (strcmp("ready", buf) == 0) {
        printf("%s ! Le joueur est prêt à jouer\n", buf);
        serveur(courante->partie);
    } 
    
    handle_tchat_serveur(args->socket_client,courante->partie);

    //char buf[25];
    uint16_t client_move[2];
    socklen_t addr_len = sizeof(courante->partie->adresse_serv_UDP);

    int sock_serv_UDP = courante->partie->sock_serv_UDP;

    while (1) {     
        fd_set rset;
        FD_ZERO(&rset);
        
        FD_SET(sock_serv_UDP, &rset);
        select(sock_serv_UDP + 1, &rset, NULL, 0, NULL);
        if (FD_ISSET(sock_serv_UDP, &rset)) {
            if (recvfrom(sock_serv_UDP, client_move, sizeof(client_move), 0, (struct sockaddr *)&courante->partie->adresse_serv_UDP, &addr_len)<0){ printf("arrrrr\n"); return -1;}
        }

        // serveur();
        uint16_t action = (ntohs(client_move[1]) >> 13) & 0x7;
        printf("action : %u\n", action);
    }
    close(sock_client);
    return 1;
}

int serveur(partie_t *p) {

    printf("diffusion OK\n");
    if (p->nb_joueurs_courant < 4) 
    {
        char message[1024] = {0};
        sprintf(message, "Il manque encore %d joueur.s", 4 - p->nb_joueurs_courant);
        int envoyes = 0;
        while ((size_t)envoyes < strlen(message)) {
            ssize_t tailleEnvoie = sendto(p->sock_serv_MDIF, message + envoyes, strlen(message) - envoyes, 0, (struct sockaddr *)&p->adresse_serv_MDIF, sizeof(p->adresse_serv_MDIF));
            if (tailleEnvoie < 0) {
                perror("Erreur lors de l'envoi du message");
                close(p->sock_serv_MDIF);
                exit(EXIT_FAILURE);
            }
            envoyes += tailleEnvoie;
        }
    }
    else
    {
        char message2[1024] = {0};
        sprintf(message2, "La partie peut commencer !!");
        int envoyes = 0;
        while ((size_t)envoyes < strlen(message2)) {
            ssize_t tailleEnvoie = sendto(p->sock_serv_MDIF, message2 + envoyes, strlen(message2) - envoyes, 0, (struct sockaddr *)&p->adresse_serv_MDIF, sizeof(p->adresse_serv_MDIF));
            if (tailleEnvoie < 0) {
                perror("Erreur lors de l'envoi du message");
                close(p->sock_serv_MDIF);
                exit(EXIT_FAILURE);
            }
            envoyes += tailleEnvoie;
        }

    }    

    printf("Envoie OK\n");
    return 0;
}

void handle_tchat_serveur (int sock_TCP, partie_t *p){

    int len_recu;
    int recu = 0;
    int octet_recu = 0;

    while ((size_t)octet_recu < sizeof(5))
    {
        recu = recv(sock_TCP, &len_recu, sizeof(len_recu), 0);
        if (recu < 0){
            perror("recv len failed");
            exit(EXIT_FAILURE);
        }
        octet_recu += recu;
    }

    printf("Longueur du message reçue : %d\n", len_recu);

    u_int16_t taille_message = ((len_recu / 2) + 2) * sizeof(u_int16_t);
    u_int16_t *message = malloc(taille_message);
    if (!message) {
        perror("malloc failed");
        return;
    }
    
    printf("taille : %u\n",taille_message);
    int paquets_recu = 0;
    int res_recv;
    while (paquets_recu < taille_message) {

        res_recv = recv(sock_TCP, message + paquets_recu, taille_message - paquets_recu, 0);

        if (res_recv == -1){
            perror("Erreur lors de la reception du message TCP");
            free(message);
            exit(EXIT_FAILURE);
        }
        if (res_recv == 0) break;

        paquets_recu += res_recv;
    }

    printf("Reception tchat %c\n", (char)(message[1] & 0xFF));
   
    u_int16_t codereq = ntohs(message[0]) & 0x1FFF;
   
    printf("codereq %u\n",codereq);
   // u_int16_t len = (ntohs(message[1] & 0xFF));
   // printf("len = %u\n", len);
    printf("message = %c", message[1] & 0xFF);
    
    for (int i = 2; i < len_recu / 2 + 2; i++) {
             printf("%c", (char)((message[i])>> 8));
             printf("%c", (char)(message[i] & 0xFF));

   }
    printf("\n");
    
    if (codereq == 7) { // envoyé à tout le monde

      int res_send;
      int paquets_envoyes;
      printf("nb joueurs courant = %d\n", p->nb_joueurs_courant);
    
        for (int i = 0; i < p-> nb_joueurs_courant; i++){
            
             if (send(p->joueurs[i]->sock_client, &len_recu, sizeof(len_recu), 0) < 0) {
                perror("send length");
                free(message);
                exit(EXIT_FAILURE);
            }
             paquets_envoyes = 0; 
            while (paquets_envoyes < taille_message){
                res_send = send(p->joueurs[i]->sock_client, message + paquets_envoyes, taille_message - paquets_envoyes, 0);

                if (res_send == -1) {
                    perror("Erreur lors de l'envoi du message");
                    exit(EXIT_FAILURE);
                }
                if (res_send == 0) break;
                paquets_envoyes += res_send;
            }
                printf("je boucle\n");
                printf("paquets envoyes : %d\n", paquets_envoyes);
        }
        printf("post for\n");
    } else { // envoie uniquement à l'équipier
        int equipe = (ntohs(message[0]) >> 15) & 0x1;
        printf("equipe %d\n", equipe);
        

        int paquets_envoyes = 0; 
        int res_send;

        for (int i = 0; i < p->nb_joueurs_courant; i++){
            if( p->joueurs[i]->id_equipe == equipe){

                while (paquets_envoyes < taille_message){
                    res_send = send(p->joueurs[i]->sock_client, message + paquets_envoyes, taille_message - paquets_envoyes, 0);

                    if (res_send == -1) {
                        perror("Erreur lors de l'envoi du message");
                        exit(EXIT_FAILURE);
                    }
                    if (res_send == 0) break;
                    paquets_envoyes += res_send;

                }
            }
            
        }
    
    }
    printf("Tchat serveur vers joueurs OK\n");

    free(message);
}