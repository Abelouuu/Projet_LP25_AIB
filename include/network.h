<<<<<<< HEAD
#ifndef NETWORK_H
#define NETWORK_H

typedef struct {
    char *name;
    char *address;
    int port;
    char *username;
    char *password;
    char *conn_type;
} remote_machine;

// Lire le fichier de configuration et remplir la liste des machines distantes
void lire_config(const char *path, remote_machine **list, int *count);

// Ajouter une machine distante à la liste
void ajouter_machine(remote_machine **list, int *count, remote_machine machine);

//Ajouter une machine renseignée par l'utilisateur
void ajouter_machine_utilisateur(remote_machine **list, int *count, char *address, char *username, char *password , int port, char *conn_type);

// Vérifier si une machine distante existe déjà dans la liste
int machine_existe(remote_machine *list, int count, char *address);

// Libérer la mémoire allouée pour la liste des machines distantes
void free_machine_list(remote_machine *list, int count);

#endif
=======
#ifndef NETWORK_H
#define NETWORK_H

typedef struct {
    char *name;
    char *address;
    int port;
    char *username;
    char *password;
    char *conn_type;
} remote_machine;

// Lire le fichier de configuration et remplir la liste des machines distantes
void lire_config(const char *path, remote_machine **list, int *count);

// Ajouter une machine distante à la liste
void ajouter_machine(remote_machine **list, int *count, remote_machine machine);

//Ajouter une machine renseignée par l'utilisateur
void ajouter_machine_utilisateur(remote_machine **list, int *count, char *address, char *username, char *password , int port, char *conn_type);

// Vérifier si une machine distante existe déjà dans la liste
int machine_existe(remote_machine *list, int count, char *address);

// Libérer la mémoire allouée pour la liste des machines distantes
void free_machine_list(remote_machine *list, int count);

#endif
>>>>>>> Basile
