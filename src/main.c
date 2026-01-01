#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include "options.h"
#include "network.h"
#include "process.h"
#include "ui.h"
#include "global.h"
#include <signal.h>

// Définition des variables globales
int nb_machines = 0;
remote_machine *liste_machines = NULL;
int current_page = 0;
int current_telnet_session = -1;
ssh_session current_ssh_session = NULL;

int main(int argc, char **argv) {
    program_options opts;
    initialiser_options(&opts);
    traiter_options(argc, argv, &opts);

    //==========================création de la liste des machines distantes============================//
    nb_machines = 0;
    if(opts.all || (!opts.remote_config && !opts.remote_server)){
        ajouter_machine_local();
    }

    if (opts.remote_config) {
        lire_config(opts.remote_config);
    } 
    if(opts.remote_server) {
        ajouter_machine_utilisateur(opts.remote_server, opts.username, opts.password, opts.port, opts.connection_type);
    }

    if(liste_machines[0].port == 22){
        current_ssh_session = connection_ssh(&liste_machines[0]);
    } else {
        current_telnet_session = connection_telnet(&liste_machines[0]);
    }

    //==========================Début de la boucle d'affichage============================//
    ui_init();

    signal(SIGWINCH, handle_resize);
    int running = 1;
    int selected = 0;

    if (opts.help) {
        affiche_aide();
    }

    while (running) {
        Process *listproc = NULL;
        int nbproc = 0;
        remote_machine current_machine = liste_machines[current_page];
        /* Lecture des processus selon la page */

        //on initialise comme si la première page n'étais pas une machine distante
        if (current_page == 0 && strcmp(liste_machines[0].name,"Localhost") == 0) {
            listproc = read_proc(-1, NULL);
        } else if (current_ssh_session) {
            listproc = read_proc(-1, current_ssh_session);
        } else if (current_telnet_session != -1) {
            listproc = read_proc(current_telnet_session, NULL);
        }

        /* Si liste vide ou erreur, affiche page vide */
        if (!listproc && current_page != 0 && strcmp(liste_machines[0].name,"LocalHost")!=0) {
            //erreur de connection
            affichePrinc(NULL, selected, current_machine);
        } else if (!listproc) {
            //bug dans la récupération locale
            break;
        } else {
            update_mem_percentage(listproc);
            listproc = sort_by_mem(listproc);

            for (Process *p = listproc; p != NULL; p = p->next) {
                nbproc++;
            }

            affichePrinc(listproc, selected, current_machine);
        }

        int ch = getch();
        ui_traite_event(ch, &selected, nbproc, &running, listproc);

        free_processes(listproc);

        usleep(150000);
    }

    ui_shutdown();
    free_machine_list();
    return 0;
}