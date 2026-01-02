#include "network.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>

//Vérifie si la machine est déja dans la liste des machines (doublons)
static int machine_existe(char *adress){
    for(int i = 0; i<nb_machines; i++){
        if(strcmp(liste_machines[i].address, adress) == 0){
            return 1; // La machine existe déjà
        }
    }
    return 0; // La machine n'existe pas
}

// Lis le fichier de config donnée en paramètre
void lire_config(const char *chemin){
    FILE *fichier = fopen(chemin, "r");
    if (!fichier) {
        perror("Erreur lors de l'ouverture du fichier de configuration");
        return;
    }
    char ligne[256];
    while (fgets(ligne, sizeof(ligne), fichier)) {
        remote_machine machine;

        machine.name = strdup(strtok(ligne, ":"));
        machine.address = strdup(strtok(NULL, ":"));
        machine.port = atoi(strtok(NULL, ":"));
        machine.username = strdup(strtok(NULL, ":"));
        machine.password = strdup(strtok(NULL, ":"));
        machine.conn_type = strdup(strtok(NULL, ":\n"));


        if(!machine_existe(machine.address)) {
            ajouter_machine(machine);
        } else {

            //libère l'espace alloué pour les champs de la machine ignorée car la machine est un doublons
            free(machine.name);
            free(machine.address);
            free(machine.username);
            free(machine.password);
            free(machine.conn_type);
        }
    }
    fclose(fichier);
}

// Ajoute une machine distante à la liste des machines
void ajouter_machine(remote_machine machine){
    liste_machines = realloc(liste_machines, (nb_machines +1)*sizeof(remote_machine));
    if (!liste_machines) {
        perror("Erreur de réallocation de mémoire");
        exit(EXIT_FAILURE);
    }
    liste_machines[nb_machines] = machine;
    nb_machines++;
}

// Créer la machine a partir des données entrée par l'utilisateur et l'ajoute à la liste des machines
void ajouter_machine_utilisateur(char *address, char *username, char *password, int port, char *conn_type) {
    remote_machine machine;
    
    machine.name = strdup("Machine User");
    machine.address = strdup(address);
    machine.port = port;
    machine.username = strdup(username);
    machine.password = strdup(password);
    machine.conn_type = strdup(conn_type);

    //Ajouter la machine a la liste
    ajouter_machine(machine);
}

// ajouter la machine local (cet ajout sert essentiellement de repère pour les indices. Ex: liste_machine[1] => première machine distante)
void ajouter_machine_local(void) {
    remote_machine machine;

    machine.name = strdup("Localhost");
    machine.address = strdup("127.0.0.1");
    machine.port = 0;
    machine.username = strdup("localuser");
    machine.password = strdup("");
    machine.conn_type = strdup("local");

    ajouter_machine(machine);
}


//libérer l'espace des machines
void free_machine_list(){
    for(int i = 0; i<nb_machines; i++){
        free(liste_machines[i].name);
        free(liste_machines[i].address);
        free(liste_machines[i].username);
        free(liste_machines[i].password);
        free(liste_machines[i].conn_type);
    }
    free(liste_machines);
}

// Se connecter à une machine via ssh (renvoie la session)
ssh_session connection_ssh(remote_machine *machine){
    ssh_session session;
    int rc;

    session = ssh_new();
    if (session == NULL){
        fprintf(stderr, "Erreur : création de la session SSH\n");
        return NULL;
    }

    /* configuration de la session */
    ssh_options_set(session, SSH_OPTIONS_HOST, machine->address);
    ssh_options_set(session, SSH_OPTIONS_PORT, &machine->port);
    ssh_options_set(session, SSH_OPTIONS_USER, machine->username);
    /* connection server */
    rc = ssh_connect(session);
    if(rc != SSH_OK) {
        fprintf(stderr, "Erreur SSH : %s\n", ssh_get_error(session));
        ssh_free(session);
        return NULL;
    }

    /* Authentification avec password */
    rc = ssh_userauth_password(session, NULL, machine->password);
    if(rc != SSH_AUTH_SUCCESS) {
        fprintf(stderr, "Erreur authentification : %s\n", ssh_get_error(session));
        ssh_disconnect(session);
        ssh_free(session);
        return NULL;
    }
    return session;
}

// Se connecter à une machine via Telnet (renvoie le socket)
int connection_telnet(remote_machine *machine) {
    int sockfd;
    struct sockaddr_in server;
    struct hostent *he;

    if (machine == NULL || machine->address == NULL) {
        fprintf(stderr, "machine invalide\n");
        return -1;
    }

    /* création du socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return -1;
    }

    /* résolution DNS */
    he = gethostbyname(machine->address);
    if (he == NULL) {
        perror("gethostbyname");
        close(sockfd);
        return -1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(machine->port > 0 ? machine->port : 23);
    memcpy(&server.sin_addr, he->h_addr_list[0], he->h_length);

    /* connexion */
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("connect");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

//executer une commande sur le terminale de la machine distante (ssh)
int ssh_exec(ssh_session session, const char *command, char *output, size_t output_size) {
    if (!session || !command || !output || output_size == 0)
        return -1;

    ssh_channel channel = ssh_channel_new(session);
    if (!channel)
        return -1;

    if (ssh_channel_open_session(channel) != SSH_OK) {
        ssh_channel_free(channel);
        return -1;
    }

    if (ssh_channel_request_exec(channel, command) != SSH_OK) {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return -1;
    }

    size_t total_read = 0;
    while (1) {
        if (total_read >= output_size - 1)
            break;

        int nbytes = ssh_channel_read(channel, output + total_read, output_size - 1 - total_read, 0);
        if (nbytes < 0) {
            ssh_channel_close(channel);
            ssh_channel_free(channel);
            return -1;
        }
        if (nbytes == 0)
            break;

        total_read += nbytes;
    }

    output[total_read] = '\0';  // Terminer la chaîne

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    return 0;
}

int telnet_exec(int sockfd,const char *command, char *output){
    const char *cmd = command;
    if (write(sockfd, cmd, strlen(cmd)) <= 0) {
        return -1;
    }

    ssize_t bytes = read(sockfd, output, sizeof(output) - 1);
    if (bytes <= 0) {
        return -1;
    }
    output[bytes] = '\0';
    return 0;
}

// conversion signal en chaîne pour la commande kill en distant
static const char *signal_to_string(int signal)
{
    switch (signal) {
    case SIGTERM: return "TERM";
    case SIGKILL: return "KILL";
    case SIGSTOP: return "STOP";
    case SIGCONT: return "CONT";
    default:      return "TERM";
    }
}

int send_signal_ssh(ssh_session session, int pid, int signal) {
    ssh_channel channel;
    char cmd[64];

    channel = ssh_channel_new(session);
    if (channel == NULL)
        return -1;

    if (ssh_channel_open_session(channel) != SSH_OK) {
        ssh_channel_free(channel);
        return -1;
    }
    snprintf(cmd, sizeof(cmd),
             "kill -%s %d",
             signal_to_string(signal),
             pid);

    if (ssh_channel_request_exec(channel, cmd) != SSH_OK) {
        ssh_channel_free(channel);
        return -1;
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    return 0;
}