#include <stdlib.h>
#include <stdio.h>

#include "options.h"

int main(int argc, char **argv) {
    struct program_options opts;
    initialiser_options(&opts);
    traiter_options(argc, argv, &opts);
    valider_options(&opts);

    printf("\nOptions traitées avec succès.\n");
    printf("\n======= Options =======\n");
    printf("help: %s\n", opts.help ? "true" : "false");
    printf("dry Run: %s\n", opts.dry_run ? "true" : "false");
    printf("remote-onfig: %s\n", opts.remote_config ? opts.remote_config : "NULL");
    printf("connection-type: %s\n", opts.connection_type ? opts.connection_type : "NULL");
    printf("port: %d\n", opts.port);
    printf("login: %s\n", opts.login ? opts.login : "NULL");
    printf("remote-server: %s\n", opts.remote_server ? opts.remote_server : "NULL");
    printf("username: %s\n", opts.username ? opts.username : "NULL");
    printf("password: %s\n", opts.password ? opts.password : "NULL");
    printf("all: %s\n", opts.all ? "true" : "false");
    printf("=======================\n");

    return 0;
}

