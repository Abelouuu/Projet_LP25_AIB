#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

// Structure pour contenir les options du programme
typedef struct{
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
} program_options;

//Initialiser les options par défaut
void initialiser_options(program_options *options);

// Traiter les options de la ligne de commande
void traiter_options(int argc, char **argv, program_options *options);

void valider_options(program_options *options);

void free_options(program_options *opt);

void dry_run(program_options *options);

void free_options(program_options *opt);

void dry_run(program_options *options);

<<<<<<< HEAD
#endif
=======
#endif
>>>>>>> 332736e43b24e3bf92547d652e94fa2b9d82d5e5
