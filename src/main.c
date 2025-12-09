#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "options.h"
#include "network.h"
#include "process.h"
#include "ui.h"


int main(int argc, char **argv) {
    program_options opts;
    initialiser_options(&opts);
    traiter_options(argc, argv, &opts);

    printf("\nOptions traitées avec succès.\n");
    printf("\n======= Options =======\n");
    printf("help: %s\n", opts.help ? "true" : "false");
    printf("dry Run: %s\n", opts.dry_run ? "true" : "false");
    printf("remote-c onfig: %s\n", opts.remote_config ? opts.remote_config : "NULL");
    printf("connection-type: %s\n", opts.connection_type ? opts.connection_type : "NULL");
    printf("port: %d\n", opts.port);
    printf("login: %s\n", opts.login ? opts.login : "NULL");
    printf("remote-server: %s\n", opts.remote_server ? opts.remote_server : "NULL");
    printf("username: %s\n", opts.username ? opts.username : "NULL");
    printf("password: %s\n", opts.password ? opts.password : "NULL");
    printf("all: %s\n", opts.all ? "true" : "false");
    printf("=======================\n");

    
    //==========================création de la liste des machines distantes============================//
    int nb_machines = 0;
    remote_machine *liste_machines = NULL;
    if (opts.remote_config) {
        lire_config(opts.remote_config, &liste_machines, &nb_machines);

        printf("\n========Liste des machines=======\n\n");
    } 
    if(opts.remote_server) {
        ajouter_machine_utilisateur(&liste_machines, &nb_machines, opts.remote_server, opts.username, opts.password, opts.port, opts.connection_type);
    }
    //Liste de machine à fini d'être crée
    if(nb_machines==0){
        printf("aucune machine distantes. Lancée le programme en local");
    } else {
        //==========================création de la liste des machines distantes============================//
        for (int i = 0; i < nb_machines; i++)
            {
                printf("Machine %d \n", i);
                printf("nom: \t\t %s \n", liste_machines[i].name);
                printf("adresse: \t %s \n", liste_machines[i].address);
                printf("username: \t %s \n", liste_machines[i].username);
                printf("password: \t %s \n", liste_machines[i].password);
                printf("type connection: \t %s \n", liste_machines[i].conn_type);
                printf("port: \t %d \n\n", liste_machines[i].port);
            }

        // N'oublie pas de libérer la mémoire après
        free_machine_list(liste_machines, nb_machines);
    }

    sleep(1);

    ui_loop_local();

    return 0;
}
