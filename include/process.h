#ifndef PROCESS_H
#define PROCESS_H

#include "network.h"

// représentation d'un processus
typedef struct Process {
    int   pid;               // PID du processus
    char  user[32];          // nom d'utilisateur
    double cpu_pct;
    double mem_pct;          // pourcentage de la RAM totale (%MEM)
    char  state;             // état (R, S, D, T, Z, etc.)
    char  cmd[256];          // nom de la commande
    struct Process *next;    // chaînage pour la liste des processus
} Process;

Process *read_proc(int sockfd, ssh_session session);


// trie la liste par %MEM décroissant (plus gourmand en premier)
Process *sort_by_mem(Process *head);

// Libère la mémoire allouée pour la liste des processus
void free_processes(Process *processes);

// Envoie les signaux au pid
int kill_process_soft(int pid); // Arrêter
int kill_process_hard(int pid); // Tuer
int pause_process(int pid);
int continue_process(int pid);

// Recherche un processus par PID ou par nom de commande
int processus_recherche(const Process *p, const char *rech);

#endif // PROCESS_H
