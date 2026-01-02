#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>

#include "options.h"
#include "network.h"
#include "process.h"

void initialiser_options(program_options *options) {
    options->help = false;
    options->dry_run = false;
    options->remote_config = NULL;
    options->connection_type = "ssh";
    options->port = 22;
    options->login = NULL;
    options->remote_server = NULL;
    options->username = NULL;
    options->password = NULL;
    options->all = false;
}

void traiter_options(int argc, char **argv, program_options *options){
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
                break;
            case 1:
                options->dry_run = true;
                break;
            case 'c':
                if (optarg == NULL || optarg[0] == '-') {
                    fprintf(stderr, "Erreur : l'option -c ou --remote-config nécessite un mot de passe.\n");
                    exit(EXIT_FAILURE);
                }
                options->remote_config = strdup(optarg);
                break;
            case 't':
                if (optarg == NULL || optarg[0] == '-') {
                    fprintf(stderr, "Erreur : l'option -t ou --connection-type nécessite un mot de passe.\n");
                    exit(EXIT_FAILURE);
                }
                options->connection_type = strdup(optarg);
                break;
            case 'P':
                if (optarg == NULL || optarg[0] == '-') {
                    fprintf(stderr, "Erreur : l'option -P ou --port nécessite un mot de passe.\n");
                    exit(EXIT_FAILURE);
                }
                options->port = atoi(optarg);
                break;
            case 'l':
                if (optarg == NULL || optarg[0] == '-') {
                    fprintf(stderr, "Erreur : l'option -l ou --login nécessite un mot de passe.\n");
                    exit(EXIT_FAILURE);
                }
                options->login = strdup(optarg);
                break;
            case 's':
                if (optarg == NULL || optarg[0] == '-') {
                    fprintf(stderr, "Erreur : l'option -s ou --remote-server nécessite un mot de passe.\n");
                    exit(EXIT_FAILURE);
                }
                options->remote_server = strdup(optarg);
                break;
            case 'u':
                if (optarg == NULL || optarg[0] == '-') {
                    fprintf(stderr, "Erreur : l'option -u ou --user nécessite un mot de passe.\n");
                    exit(EXIT_FAILURE);
                }
                options->username = strdup(optarg);
                break;
            case 'p':
                if (optarg == NULL || optarg[0] == '-') {
                    fprintf(stderr, "Erreur : l'option -p ou --password nécessite un mot de passe.\n");
                    exit(EXIT_FAILURE);
                }
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



void valider_options(program_options *options) {
    // -a necessite -c et -s
    if (options->all && options->remote_config == NULL && options->remote_server == NULL) {
        printf("L'option --all nécessite que --remote-config ou --remote-server soient spécifiés.\n");
        exit(EXIT_FAILURE);
    }

    //verifie que le type de connection est ssh ou telnet
    if ((strcmp(options->connection_type, "ssh") != 0) && (strcmp(options->connection_type, "telnet") != 0)) {
        printf("Erreur : le type de connexion n'est pas pris en compte (choisir ssh ou telnet)\n");
        exit(EXIT_FAILURE);
    }

    //gerer si le type de connection correspond au port
    if ((strcmp(options->connection_type, "ssh") == 0 && options->port != 22) ||
    (strcmp(options->connection_type, "telnet") == 0 && options->port != 23)) {
        printf("Erreur : le port ne correspond pas au type de connexion\n");
        exit(EXIT_FAILURE);
    }

    //Gérer les conflits entre login, username, remote-server et demander le password si il n'as pas été donnée
    if(options->login){
        //verification de la syntaxe du login
        char *at_sign = strchr(options->login, '@');
        if (!at_sign || at_sign == options->login || *(at_sign + 1) == '\0') {
            exit(EXIT_FAILURE);
        }
        if (strchr(at_sign + 1, '@')) {
            exit(EXIT_FAILURE);
        }

        //=====syntaxe bonne=====//

        //verifie les conflits avec username et remote-server
        if (options->username || options->remote_server)
        {
            char reponse;
            printf("\n[WARNING] L'options --login (-l) ne peut pas être utilisé en même temps que les options --username (-u) et --password (-p)\n");
            printf("Les options --username et --password vont être écraser pas l'argument --login \n");
            printf("Êtes-vous sur de vouloir continué ? [Y/o] ");
            
            do {
                reponse = getchar();
            } while (isspace(reponse));

            if (reponse != 'Y' && reponse != 'y') {
                printf("Opération annulée par l'utilisateur.\n");
                exit(EXIT_FAILURE);
            }
        }

        //changer le username et le remote-server

        //on sépare le login en 2
        *at_sign = '\0';

        free(options->username);
        free(options->remote_server);

        options->username = strdup(options->login);
        options->remote_server = strdup(at_sign+1);
    }

    //si -s est rentré, alors -u et -p doivent contenir une valeur
    if (options->remote_server) {
        if(!options->username){
            printf("username de la machine %s: ", options->username);
            options->username = malloc(100);
            scanf("%99s", options->username);
        }
        if(!options->password) {
            printf("password de la machine %s: ", options->username);
            options->password = malloc(100);
            scanf("%99s", options->password);
        }
    }
}

void free_options(program_options *opt) {
    free(opt->remote_config);
    free(opt->connection_type);
    free(opt->login);
    free(opt->remote_server);
    free(opt->username);
    free(opt->password);
}

int dry_run(void) {
    for (int i = 0; i < nb_machines; i++) {
        remote_machine *m = &liste_machines[i];

        //si c'est la machine local
        if (strcmp(m->name, "Localhost") == 0) {
            Process *p = read_proc(-1, NULL);
            if (!p) {
                fprintf(stderr, "Dry-run: échec accès processus locaux\n");
                return -1;
            }
            free_processes(p);
        } 
        //si c'est la machine connecté via ssh
        else if (m->port == 22) {
            ssh_session s = connection_ssh(m);
            if (!s) {
                fprintf(stderr, "Dry-run: échec connexion SSH (%s)\n", m->name);
                return -1;
            }

            Process *p = read_proc(-1, s);
            if (!p) {
                fprintf(stderr, "Dry-run: échec lecture processus SSH (%s)\n", m->name);
                ssh_disconnect(s);
                ssh_free(s);
                return -1;
            }

            free_processes(p);
            ssh_disconnect(s);
            ssh_free(s);
        } 
        //si c'est la machine connecté via telnet
        else {
            int sock = connection_telnet(m);
            if (sock == -1) {
                fprintf(stderr, "Dry-run: échec connexion Telnet (%s)\n", m->name);
                return -1;
            }

            Process *p = read_proc(sock, NULL);
            if (!p) {
                fprintf(stderr, "Dry-run: échec lecture processus Telnet (%s)\n", m->name);
                close(sock);
                return -1;
            }

            free_processes(p);
            close(sock);
        }
    }

    printf("Dry-run: succès.\n");
    return 0;
}
