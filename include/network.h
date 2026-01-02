#ifndef NETWORK_H
#define NETWORK_H

#include <libssh/libssh.h>


typedef struct {
    char *name;
    char *address;
    int port;
    char *username;
    char *password;
    char *conn_type;
} remote_machine;

// Lire le fichier de configuration et remplir la liste des machines distantes
int lire_config(const char *path);

// Ajouter une machine distante à la liste
void ajouter_machine(remote_machine machine);

// créer la machine a partir des données entrée par l'utilisateur et l'ajoute à la liste des machines
void ajouter_machine_utilisateur(char *address, char *username, char *password , int port, char *conn_type);

//Ajouter la machine locale à la liste
void ajouter_machine_local();

// Libérer la mémoire allouée pour la liste des machines distantes
void free_machine_list();

ssh_session connection_ssh(remote_machine *machine);
int connection_telnet(remote_machine *machine);

int ssh_exec(ssh_session session, const char *command, char *output, size_t output_size);

int telnet_exec(int sockfd,const char *command, char *output);

int send_signal_ssh(ssh_session session, int pid, int signal);


#endif
