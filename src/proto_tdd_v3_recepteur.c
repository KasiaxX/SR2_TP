/*************************************************************
* proto_tdd_v0 -  récepteur                                  *
* TRANSFERT DE DONNEES  v0                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* Université de Toulouse / FSI / Dpt d'informatique          *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"
#define N 4
#define CAPACITE 16

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    // unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t pdata, pack;                  /* paquet utilisé par le protocole */
    int fin = 0;                     /* condition d'arrêt */
    int paquet_attendu = 0;

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {

        // attendre(); /* optionnel ici car de_reseau() fct bloquante */
        de_reseau(&pdata);

        printf("[TRP] j'ai recu le paquet %d.\n", pdata.num_seq);


        // /* extraction des donnees du paquet recu */
        // for (int i=0; i<pdata.lg_info; i++) {
        //     message[i] = pdata.info[i];
        // }
        
        
        if(verifier_controle(pdata)){
            printf("[TRP] verifier controle. \n");
            if(pdata.num_seq == paquet_attendu){
                fin = vers_application(pdata.info, pdata.lg_info);

                printf("[TRP] le paquet recu %d est le paquet attendu %d. \n", pdata.num_seq, paquet_attendu);

                //on met a jour le num de sequence pour envoyer l'ack
                pack.num_seq = paquet_attendu;

                //on increment le paquet attendu
                paquet_attendu = inc(paquet_attendu, CAPACITE);

            }else{ //paquet hors-sequence : on renvoie l'ack du dernier paquet recu valide, paquet avant paquet_attendu
                printf("[TRP] le paquet recu %d n'est pas le paquet attendu %d .\n", pdata.num_seq, paquet_attendu);
                pack.num_seq = (paquet_attendu - 1 + CAPACITE) % CAPACITE; 
            }
            
        }

        pack.type = ACK;
        pack.lg_info = 0;
        pack.somme_ctrl = generer_controle(pack);

        printf("[TRP] on envoye le ack de paquet %d.\n", pdata.num_seq);
        vers_reseau(&pack);

    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
