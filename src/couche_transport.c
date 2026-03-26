#include <stdio.h>
#include "couche_transport.h"
#include "services_reseau.h"
#include "application.h"

/* ************************************************************************** */
/* *************** Fonctions utilitaires couche transport ******************* */
/* ************************************************************************** */

// RAJOUTER VOS FONCTIONS DANS CE FICHIER...



/*--------------------------------------*/
/* Fonction d'inclusion dans la fenetre */
/*--------------------------------------*/
int dans_fenetre(unsigned int inf, unsigned int pointeur, int taille) {

    unsigned int sup = (inf+taille-1) % SEQ_NUM_SIZE;

    return
        /* inf <= pointeur <= sup */
        ( inf <= sup && pointeur >= inf && pointeur <= sup ) ||
        /* sup < inf <= pointeur */
        ( sup < inf && pointeur >= inf) ||
        /* pointeur <= sup < inf */
        ( sup < inf && pointeur <= sup);
}

uint8_t generer_controle(paquet_t paquet){
    uint8_t somme = 0;

    //^ XOR

    somme = paquet.type ^ somme;
    somme = paquet.num_seq ^ somme;
    somme = paquet.lg_info ^ somme;
    
    for(int i=0; i<paquet.lg_info; i++){
        somme = paquet.info[i] ^ somme;
    }

    return somme;
}

/* verifier si il n'y a pas d'erreur */
uint8_t verifier_controle(paquet_t paquet){
    uint8_t somme_recu = paquet.somme_ctrl;

    uint8_t somme = generer_controle(paquet);

    return somme_recu == somme;
}

/* pour incrementer le numero de paquet avec le modulo donne*/
int inc(int num_paquet, int mod){
    num_paquet = (num_paquet + 1) %mod;

    return num_paquet;
}





