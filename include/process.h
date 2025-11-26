// process.h
#ifndef PROCESS_H
#define PROCESS_H

#include <stdbool.h>

typedef struct process_info {
    int pid;
    char name[256];
    char user[64];
    float cpu_usage;
    float mem_usage;
    long int runtime; // en secondes
} process_info;

// Liste tous les processus sur la machine locale
int liste_processsus_locaux(process_info **processes);

// Libère la mémoire allouée pour la liste des processus
void libere_liste_processus(process_info *processes, int count);

#endif // PROCESS_H
