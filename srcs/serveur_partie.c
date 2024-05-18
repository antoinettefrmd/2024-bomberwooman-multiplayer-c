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

#define PORT 12121 /* quel port utiliser ?*/

static int PORT_UDP = 1234;
int PORT_MDIF = 4321;

int rajoute_joueur_partie(liste_parties_t *lp, int type_4) // rajouter un gros lock sur la fonction
{
    joueur_t *j = malloc(sizeof(joueur_t));    

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
        }
    }
    return 0;
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
        rajoute_joueur_partie (p_2v2, 0);
        rep_0 |= 10 << 3;
        courante = p_2v2;
    }
    else 
    {
        rajoute_joueur_partie(p_4_adv, 1);
        rep_0 |= 9 << 3;
        courante = p_4_adv;
    }
    pthread_mutex_unlock(args->verrou);

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
    rep[2] = htons(PORT_MDIF);
    
    
    struct sockaddr_in6 adresseMultiDiff;
    memset(&adresseMultiDiff, 0, sizeof(adresseMultiDiff));
    adresseMultiDiff.sin6_family = AF_INET6;
    inet_pton(AF_INET6,"ff12::1:2:3", &adresseMultiDiff.sin6_addr);
    adresseMultiDiff.sin6_port = htons(PORT_MDIF);
    printf("portMDIFF serveur = %u\n", PORT_MDIF);
    PORT_MDIF++;

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

    serveur(adresseMultiDiff);
    

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
        uint16_t action = (ntohs(client_move[1]) >> 13) & 0x3;
        printf("action : %u\n", action);
    }
    close(sock_client);
    return 1;
}

int serveur(struct sockaddr_in6 adresseMultiDiff) {
    
    /* déclaration d'une socket UDP IPv6 */
    int sock = socket(PF_INET6, SOCK_DGRAM,0); 
    if (sock < 0){
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }

    /* Liaison de la socket à une interface réseau spécifique */
    int ifindex = if_nametoindex("wlo1");
    adresseMultiDiff.sin6_scope_id = ifindex;

    int ok = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0) {
        perror("echec de SO_REUSEADDR");
        close(sock);
        return 1;
    }
    /*
    if(setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifindex, sizeof(ifindex)) == -1) {
        perror("erreur initialisation de l’interface locale");
        exit(EXIT_FAILURE);
    }*/
    // adresseMultiDiff.sin6_scope_id = ifindex;

    printf("diffusion OK\n");

    char message[] = "Message de test";
    ssize_t tailleEnvoie = sendto(sock, message, strlen(message), 0, (struct sockaddr *)&adresseMultiDiff, sizeof(adresseMultiDiff));
    if (tailleEnvoie < 0) {
        perror("Erreur lors de l'envoi du message");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Envoie OK\n");
    return 0;
}
