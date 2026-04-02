/*************************************************************
* proto_tdd_v2 -  émetteur                                   *
* TRANSFERT DE DONNEES  v2                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* Université de Toulouse / FSI / Dpt d'informatique          *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg;                  /* taille du message */
    paquet_t pdata, pack;                  /* paquet utilisé par le protocole */
    int prochain_paquet = 0;

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);

    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 ) {

        /* construction paquet */
        for (int i=0; i<taille_msg; i++) {
            pdata.info[i] = message[i];
        }
        pdata.lg_info = taille_msg;
        pdata.type = DATA;
        pdata.num_seq = prochain_paquet;
        pdata.somme_ctrl = generer_controle(pdata);

        /* remise à la couche reseau */
        vers_reseau(&pdata);
        depart_temporisateur(100);
        int event = attendre();

        while(event != PAQUET_RECU ){
            //retransmission
            printf("[EMETTEUR] Erreur -> retransmission\n");
            vers_reseau(&pdata);
            depart_temporisateur(100);
            event = attendre();
        }

        //ACK 
        de_reseau(&pack);
        printf("%d", pdata.type);

        arret_temporisateur();

        //on prend le paquet suivant
        prochain_paquet = inc(prochain_paquet,2);

        /* lecture des donnees suivantes de la couche application */
        de_application(message, &taille_msg);
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
