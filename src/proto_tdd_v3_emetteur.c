/*************************************************************
* proto_tdd_v0 -  émetteur                                   *
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
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg;                  /* taille du message */
    paquet_t  pack;                  /* paquet utilisé par le protocole */
    int taille_fenetre = N;
    int borneInf = 0;
    int curseur = 0;
    paquet_t tab[CAPACITE]; //tableau de paquets de taille 8 ; 1er paquet - 0, l'indice de tab - 0 etc ...


    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);

    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 || borneInf != curseur) {

        //1er cas
        if(dans_fenetre(borneInf, curseur, taille_fenetre)){
            /* construction paquet */
            for (int i=0; i<taille_msg; i++) {
                tab[curseur].info[i] = message[i];
            }
            tab[curseur].lg_info = taille_msg;
            tab[curseur].type = DATA;
            tab[curseur].num_seq = curseur;
            tab[curseur].somme_ctrl = generer_controle(tab[curseur]);

            /* remise à la couche reseau */
            vers_reseau(&tab[curseur]);

            if(borneInf == curseur){  //le 1er paquet de la fenetre
                depart_temporisateur(100);       
            }

             //on prend le paquet suivant
            curseur = inc(curseur,CAPACITE);

        }else{ //prochaine paquet n'est plus dans la fenetre
            int event = attendre();

            if(event == PAQUET_RECU){
                de_reseau(&pack);
                //dans_fenetre -> verifier le num seq valide
                if(verifier_controle(pack)){
                    if(dans_fenetre(borneInf, pack.num_seq, taille_fenetre)){
                        borneInf = inc(pack.num_seq, CAPACITE); //decalage de fenetre

                        if(borneInf == curseur){ // tout a ete acquitte donc on arrete le timeur 
                            arret_temporisateur();
                        }
                    }
                    
                }else{  //paquet n'est pas recu - il n'y a pas d' ack
                    printf("[EMETTEUR] Erreur -> retransmission\n");
                    int i = borneInf;
                    depart_temporisateur(100);

                    while(i != curseur){
                        printf("[TRP] Renvoie de paquet");
                        vers_reseau(&tab[i]);
                        i = inc(i, CAPACITE);
                    }
                }
            }

           

            /* lecture des donnees suivantes de la couche application */
            de_application(message, &taille_msg);
        }

        //TRP - mess pour indiquer sur la couche de transport
        printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
        return 0;    
    }
}
