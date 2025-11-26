#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "options.h"

void initialiser_options(struct program_options *options) {
    options->help = false;
    options->dry_run = false;
    options->remote_config = NULL;
    options->connection_type = NULL;
    options->port = 0;
    options->login = NULL;
    options->remote_server = NULL;
    options->username = NULL;
    options->password = NULL;
    options->all = false;
}

void traiter_options(int argc, char **argv, struct program_options *options){
    int opt = 0;
    struct option my_opts[] = {
        {"help", no_argument, 0, 'h'},
        {"dry-run", no_argument, 0, 1},
        {"remote-config", required_argument, 0, 'c'},
        {"connection-type", required_argument, 0, 't'},
        {"port", required_argument, 0, 'P'},
        {"login", required_argument, 0, 'l'},
        {"remote-server", required_argument, 0, 's'},
        {"username", required_argument, 0, 'u'},
        {"password", required_argument, 0, 'p'},
        {"all", no_argument, 0, 'a'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "hc:t:P:l:s:u:p:a", my_opts, NULL)) != -1) {
        switch (opt) {
            case 'h':
                options->help = true;
                affiche_aide(argv[0]);
                break;
            case 1:
                options->dry_run = true;
                break;
            case 'c':
                options->remote_config = strdup(optarg);
                break;
            case 't':
                options->connection_type = strdup(optarg);
                break;
            case 'P':
                options->port = atoi(optarg);
                break;
            case 'l':
                options->login = strdup(optarg);
                break;
            case 's':
                options->remote_server = strdup(optarg);
                break;
            case 'u':
                options->username = strdup(optarg);
                break;
            case 'p':
                options->password = strdup(optarg);
                break;
            case 'a':
                options->all = true;
                break;
            default:
                // Gérer les options inconnues
                fprintf(stderr, "Option inconnue: %c\n", opt);
                exit(EXIT_FAILURE);
        }
    }
    valider_options(options);
}

void affiche_aide(const char *nom_programme) {
    printf("Utilisation: %s [options]\n", nom_programme);
    printf("Options:\n");
    printf("  -h, --help                 Afficher cette aide\n");
    printf("  --dry-run                  Test l'accès aux processus sans les afficher\n");
    printf("  -c, --remote-config FILE   Spécifier le fichier de configuration distant\n");
    printf("  -t, --connection-type TYPE Spécifier le type de connexion\n");
    printf("  -P, --port PORT            Spécifier le port de connexion\n");
    printf("  -l, --login LOGIN          Spécifier le login\n");
    printf("  -s, --remote-server SERVER Spécifier le serveur distant\n");
    printf("  -u, --username USERNAME    Spécifier le nom d'utilisateur\n");
    printf("  -p, --password PASSWORD    Spécifier le mot de passe\n");
    printf("  -a, --all                  Appliquer à tous les éléments\n");
}

void valider_options(struct program_options *options) {
    // -a necessite -c et -s
    if (options -> all && (options -> connection_type == NULL || options -> remote_server == NULL)) {
        printf("L'option --all nécessite que --connection-type et --remote-server soient spécifiés.\n");
        exit(EXIT_FAILURE);
    }
}