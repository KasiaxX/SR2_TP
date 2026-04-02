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

        
        if(verifier_controle(pdata)){

            printf("[TRP] verifier controle. \n");

            pack.type = ACK;
            pack.lg_info = 0;
            pack.num_seq = pdata.num_seq; // SR: On acquitte le paquet reçu, même hors-séquence
            pack.somme_ctrl = generer_controle(pack);

            // On envoie l'ACK immédiatement
            vers_reseau(&pack); 
            printf("[TRP] Envoi ACK %d\n", pack.num_seq);

            

            if(dans_fenetre(paquet_attendu, pdata.num_seq, taille_fenetre)){
                printf("[TRP] dans la fenetre \n");

                if(pdata.num_seq != paquet_attendu){  //Not first ; paquet hors sequence


                    // Paquet hors séquence : on le stocke dans le buffer
                    if(paquet_hors[pdata.num_seq] == 0) {

                        buffer[pdata.num_seq] = pdata; // On stocke le paquet ENTIER à son index
                        paquet_hors[pdata.num_seq] = 1;
                        printf("[TRP] Paquet %d hors sequence, mis en buffer.\n", pdata.num_seq);
                    }

                }else{ //c'est le first ; 1er paquet; paqet attendu
                    // C'est le paquet attendu ! On le remonte à l'application.

                    fin = vers_application(&pdata.info, pdata.lg_info);
                    // borneInf = inc(borneInf, CAPACITE); //on decale la fenetre
                    // //on met a jour le num de sequence pour envoyer l'ack
                    // pack.num_seq = paquet_attendu;

                    //on increment le paquet attendu
                    paquet_attendu = inc(paquet_attendu, CAPACITE);

                    while( paquet_hors[paquet_attendu] == 1){
                        printf("[TRP] Paquet %d sorti du buffer et remonte.\n", paquet_attendu);

                        fin = vers_application(buffer[paquet_attendu].info, buffer[paquet_attendu].lg_info);

                        //on vide la case
                        paquet_hors[paquet_attendu] = 0;
                        // borneInf = inc(borneInf, CAPACITE); //on decale la fenetre

                        paquet_attendu = inc(paquet_attendu, CAPACITE);
                    }
                }
            } 
            
        }else {
            printf("[TRP] Paquet corrompu reçu.\n");
        }

    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
