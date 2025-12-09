#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "network.h"


void lire_config(const char *chemin, remote_machine **liste, int *count){
    FILE *fichier = fopen(chemin, "r");
    if (!fichier) {
        perror("Erreur lors de l'ouverture du fichier de configuration");
        return;
    }
    char ligne[256];
    while (fgets(ligne, sizeof(ligne), fichier)) {
        remote_machine machine;

        machine.name = strdup(strtok(ligne, ":"));
        machine.address = strdup(strtok(NULL, ":"));
        machine.port = atoi(strtok(NULL, ":"));
        machine.username = strdup(strtok(NULL, ":"));
        machine.password = strdup(strtok(NULL, ":"));
        machine.conn_type = strdup(strtok(NULL, ":\n"));


        if(!machine_existe(*liste, *count, machine.address)) {
            ajouter_machine(liste, count, machine);
        } else {
            printf("La machine %s est en doublons. Ignorée.\n", machine.address);

            //libère l'espace alloué pour les champs de la machine ignorée
            free(machine.name);
            free(machine.address);
            free(machine.username);
            free(machine.password);
            free(machine.conn_type);
        }
    }
    fclose(fichier);
}

int machine_existe(remote_machine *liste, int count, char *adress){
    for(int i = 0; i<count; i++){
        if(strcmp(liste[i].address, adress) == 0){
            return 1; // La machine existe déjà
        }
    }
    return 0; // La machine n'existe pas
}

void ajouter_machine(remote_machine **liste, int *count, remote_machine machine){
    *liste = realloc(*liste, (*count +1) *sizeof(remote_machine));
    if (!*liste) {
        perror("Erreur de réallocation de mémoire");
        exit(EXIT_FAILURE);
    }
    (*liste)[*count] = machine;
    (*count)++;
}

void ajouter_machine_utilisateur(remote_machine **liste, int *count, char *address, char *username, char *password, int port, char *conn_type) {
    remote_machine machine;
    
    machine.name = strdup("Machine User");
    machine.address = strdup(address);
    machine.port = port;
    machine.username = strdup(username);
    machine.password = strdup(password);
    machine.conn_type = strdup(conn_type);

    //Ajouter la machine a la liste
    ajouter_machine(liste, count, machine);
}

void free_machine_list(remote_machine *liste, int count){
    for(int i = 0; i<count; i++){
        free(liste[i].name);
        free(liste[i].address);
        free(liste[i].username);
        free(liste[i].password);
        free(liste[i].conn_type);
    }
    free(liste);
}