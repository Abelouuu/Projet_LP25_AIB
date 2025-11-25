#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

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

void init_program_options(struct program_options *opt);
void parse_options(int argc, char **argv, struct program_options *opt);

#endif