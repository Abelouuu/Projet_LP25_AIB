#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>  // pour pid_t 
#include <stddef.h>     // pour size_t

// représentation d'un processus
typedef struct Process {
    int   pid;               // PID du processus
    char  user[32];          // nom d'utilisateur
    long  mem_kb;            // mémoire résidente en kB (VmRSS)
    double mem_pct;          // pourcentage de la RAM totale (%MEM)
    char  state;             // état (R, S, D, T, Z, etc.)
    char  cmd[256];          // nom de la commande
    struct Process *next;    // chaînage
} Process;

// lecture de la liste des processus locaux depuis /proc
Process *read_processes(void);

// calcule le %MEM pour chaque processus (en lisant /proc/meminfo)
void update_mem_percentage(Process *head);

// trie la liste par %MEM décroissant (plus gourmand en premier)
Process *sort_by_mem(Process *head);

// affiche les processus (limité à 50 lignes pour éviter le spam)
void print_processes(Process *head, const char *machine_name);

// libère toute la liste chaînée
void free_processes(Process *head);

// controle des processus via signaux
int kill_process_soft(int pid);   // SIGTERM (arrêt propre)
int kill_process_hard(int pid);   // SIGKILL (arrêt forcé)
int pause_process(int pid);       // SIGSTOP (pause)
int continue_process(int pid);    // SIGCONT (reprendre)

#endif /* PROCESS_H */