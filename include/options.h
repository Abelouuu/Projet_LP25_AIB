#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

// Structure pour contenir les options du programme
struct program_options {
    bool help;
    bool dry_run;
    char *remote_config;
    char *connection_type;
    int port;
    char *login;
    char *remote_server;
    char *username;
    char *password;
    bool all;
};

//Initialiser les options par défaut
void initialiser_options(struct program_options *options);

// Traiter les options de la ligne de commande
void traiter_options(int argc, char **argv, struct program_options *options);

// Afficher l'aide du programme (appel de -h ou --help)
void affiche_aide(const char *nom_programme);

void valider_options(struct program_options *options);

#endif