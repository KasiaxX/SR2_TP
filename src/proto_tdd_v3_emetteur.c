/*************************************************************
* proto_tdd_v3 -  émetteur                                   *
* TRANSFERT DE DONNEES  v3                                   *
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

    // if (argc > 1) {
    //     taille_fenetre = atoi(argv[1]);
    // }

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);

    printf("[TRP] j'ai recu le paquet.\n");

    /* tant que l'émetteur a des données à envoyer OU qu'il attend des ACK */
    while ( taille_msg != 0 || borneInf != curseur) {

        //1er cas : On a de la place dans la fenêtre ET on a des données à envoyer
        if(dans_fenetre(borneInf, curseur, taille_fenetre) && taille_msg != 0 ){
            printf("[TRP] dans la fenetre \n");
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

            printf("[TRP] j'envoye le paquet\n");

            //temporisateur que pour le 1er paquet de window
            if(borneInf == curseur){  //le 1er paquet de la fenetre
                depart_temporisateur(100);  
                printf("[TRP] depart temporisateur\n");     
            }

             //on prend le paquet suivant
            curseur = inc(curseur,CAPACITE);

            /* lecture des donnees suivantes de la couche application */
            de_application(message, &taille_msg);

        }else{ 
            // 2ème cas : La fenêtre est pleine OU on a fini de lire le fichier (taille_msg == 0)
            // On se met en attente d'un événement (ACK ou Timeout)
            int event = attendre();
            printf("[TRP] non dans la fenetre\n"); 

            if(event == PAQUET_RECU){
                printf("[TRP] paquet recu\n"); 

                de_reseau(&pack);
                //dans_fenetre -> verifier le num seq valide
                if(verifier_controle(pack)){
                    printf("[TRP] verifier controle\n"); 

                    if(dans_fenetre(borneInf, pack.num_seq, taille_fenetre)){
                        printf("[TRP] dans la fenetre 2\n"); 
                        
                        borneInf = inc(pack.num_seq, CAPACITE); //decalage de fenetre

                        arret_temporisateur(); // On arrête le timer actuel
                        printf("[TRP] arret temporisateur\n"); 

                        // S'il reste des paquets non acquittés, on relance le timer
                        if(borneInf != curseur){  
                            depart_temporisateur(100);
                        }
                    }
                    // Si l'ACK est corrompu, on ne fait rien, on attendra le timeout.
                    
                }else {
                    // Paquet corrompu reçu : on ne fait rien, on attendra le timeout.
                    printf("[TRP] Paquet corrompu ignoré.\n"); 
                }
            }else{  
                //paquet n'est pas recu - il n'y a pas d' ack
                // TIMEOUT (event != PAQUET_RECU)
                printf("[EMETTEUR] Erreur -> retransmission\n");
                int i = borneInf;

                //on relance le timer pour la retransmission
                depart_temporisateur(100);

                // On retransmet TOUTE la fenêtre (de borneInf à curseur)
                while(i != curseur){
                    printf("[TRP] Renvoie de paquet");
                    vers_reseau(&tab[i]);
                    i = inc(i, CAPACITE);
                }
            }
        }      
    }
    //TRP - mess pour indiquer sur la couche de transport
    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
