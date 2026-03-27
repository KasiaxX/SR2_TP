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
#define W 8
#define CAPACITE 16

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    //unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t pdata, pack;                  /* paquet utilisé par le protocole */
    int fin = 0;                     /* condition d'arrêt */
    int paquet_attendu = 0;
    int taille_fenetre = W;

    int borneInf = 0;
    int curseur = 0;
    

    paquet_t buffer[CAPACITE]; //tableau de paquets de taille 16 ; 1er paquet - 0, l'indice de tab - 0 etc ...
    int paquet_hors[CAPACITE] = {0}; // 1 si le paquet a été acquitté, 0 sinon

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

        pack.type = ACK;
        pack.lg_info = 0;
        pack.num_seq = (paquet_attendu - 1 + CAPACITE) % CAPACITE;
        
        
        if(verifier_controle(pdata)){

            printf("[TRP] verifier controle. \n");

            if(dans_fenetre(borneInf, curseur, taille_fenetre)){
                printf("[TRP] dans la fenetre \n");

                if(pdata.num_seq != paquet_attendu){  //Not first ; paquet hors sequence

                    int cpt = 0;
                    /* construction paquet */
                    for (int i=0; pdata.info[i] != '\0'; i++) {
                       buffer[curseur].info[i] = pdata.info[i];
                       cpt++;
                    }
                    buffer[curseur].lg_info = cpt;
                    buffer[curseur].type = DATA;
                    buffer[curseur].num_seq = curseur;
                    buffer[curseur].somme_ctrl = generer_controle(buffer[curseur]);

                    printf("[TRP] le paquet recu %d est le paquet attendu %d. \n", pdata.num_seq, paquet_attendu);

                    paquet_hors[pdata.num_seq] = 1; 
                }
                else{ //c'est le first ; 1er paquet; paqet attendu

                    fin = vers_application(&pdata, pdata.lg_info);
                    borneInf = inc(borneInf, CAPACITE); //on decale la fenetre
                    //on met a jour le num de sequence pour envoyer l'ack
                    pack.num_seq = paquet_attendu;

                    //on increment le paquet attendu
                    paquet_attendu = inc(paquet_attendu, CAPACITE);

                    while( paquet_hors[borneInf] == 1){
                        fin = vers_application(buffer[borneInf].info, buffer[borneInf].lg_info);
                        paquet_hors[borneInf] = 0;
                        borneInf = inc(borneInf, CAPACITE); //on decale la fenetre

                        pack.num_seq = paquet_attendu;
                        paquet_attendu = inc(paquet_attendu, CAPACITE);
                    }

                }

            }
            vers_reseau(&pack);
            
        }else {
            printf("[TRP] Paquet corrompu reçu.\n");
        }

        pack.somme_ctrl = generer_controle(pack);

        printf("[TRP] on envoye le ack de paquet %d.\n", pdata.num_seq);
        // vers_reseau(&pack);

    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
