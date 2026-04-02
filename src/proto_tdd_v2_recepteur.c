/*************************************************************
* proto_tdd_v2 -  récepteur                                  *
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
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message pour l'application */
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

        /* extraction des donnees du paquet recu */
        for (int i=0; i<pdata.lg_info; i++) {
            message[i] = pdata.info[i];
        }
        
        
        if(verifier_controle(pdata)){
            if(pdata.num_seq == paquet_attendu){
                pack.type = ACK;
                fin = vers_application(message, pdata.lg_info);
                paquet_attendu = inc(paquet_attendu, 2);
            }
            vers_reseau(&pack);
        }

    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
