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
#include "bomberwoman.h"

#define PORT 12121 /* quel port utiliser ?*/
static int PORT_MDIFF = 4321;

int rajoute_joueur_partie(liste_parties_t *lp, int type) 
{
    joueur_t *j = malloc(sizeof(joueur_t));    

    liste_parties_t *courante = lp;

    if (courante->partie == NULL)
    {
        partie_t *p = malloc(sizeof(partie_t));
        j->id = 0;
        if (!type) { j->id_equipe = 0; }
        p->joueurs[0] = j;
        p->nb_joueurs_courant = 1;
        p->port = 1903;
        lp->partie = p;
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
            j->id = p_courante->nb_joueurs_courant - 1;
            if (!type) 
            {
                if (p_courante->nb_joueurs_courant < 2) {j->id_equipe = 0;}
                else {j->id_equipe = 1;}
            }
            p_courante->joueurs[p_courante->nb_joueurs_courant - 1] = j;
            p_courante->nb_joueurs_courant++;
        }
        else
        {
            partie_t *p = malloc(sizeof(partie_t));
            j->id=0;
            if (!type) { j->id_equipe = 0; }
            p->joueurs[0] = j;
            p->nb_joueurs_courant = 1;
            p->port++;
            liste_parties_t *lp2 = malloc(sizeof(liste_parties_t));
            lp2->partie = p;
            courante->suivant = lp2;
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

    u_int16_t rep[4];
    memset(&rep, 0, sizeof(u_int16_t)*4);


    liste_parties_t *courante;
    printf("%d\n",ntohs(req[0]));
    if (req[0] == 2) 
    { 
        rajoute_joueur_partie (p_2v2, 0);
        rep_0 |= (u_int16_t)(10 & 0x1FFF);
        courante = p_2v2;
    }
    else 
    {
        rajoute_joueur_partie(p_4_adv, 1);
        rep_0 |= (u_int16_t)(9 & 0x1FFF);
        courante = p_4_adv;
    }

    while(courante->suivant != NULL && courante->suivant->partie != NULL)
    {
        courante = courante->suivant;
    }

    u_int16_t id = (courante->partie->nb_joueurs_courant - 1) << 1;
    rep_0 |= (u_int16_t)((id & 0x3) << 13);
    rep_0 |= (u_int16_t)((1 & 0x1) << 15);
    rep[0] = htons(rep_0);

    rep[1] = htons(1234);
    rep[2] = htons(4321);
    
    struct sockaddr_in6 adresseMultiDiff;
    memset(&adresseMultiDiff, 0, sizeof(adresseMultiDiff));
    adresseMultiDiff.sin6_family = AF_INET6;
    inet_pton(AF_INET6,"ff12::1:2:3", &adresseMultiDiff.sin6_addr);
    adresseMultiDiff.sin6_port = htons(PORT_MDIFF);
    memcpy(&rep[3], adresseMultiDiff.sin6_addr.s6_addr, sizeof(rep[3]));
   
    serveur(adresseMultiDiff);

    int reponse = 0;
    ssize_t envoi;
    while((size_t)reponse < sizeof(rep)) 
    {
        envoi = send(sock_client, rep + reponse, sizeof(rep), 0);
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

    close(sock_client);
    return 1;
}

int serveur(struct sockaddr_in6 addr_server) {
    
    /* déclaration d'une socket UDP IPv6 */
    int sock = socket(AF_INET6, SOCK_DGRAM,0); 
    if (sock < 0){
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }

    /* Liaison de la socket à une interface réseau spécifique */
    int ifindex = if_nametoindex("en0"); /* interface réseau multicast sur ma machine */
    if (ifindex == 0) {
        perror("Erreur lors de la récupération de l'index de l'interface");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if(setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifindex, sizeof(ifindex)) == -1) {
        perror("erreur initialisation de l’interface locale");
        exit(EXIT_FAILURE);
    }

    /* Liaison de la socket au port */
    if (bind(sock, (struct sockaddr *)&addr_server, sizeof(addr_server)) < 0) {
        perror("Erreur lors de la liaison de la socket au port");
        exit(EXIT_FAILURE);
    }
    printf("diffusion OK\n");

 
    return 0;
}