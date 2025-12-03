/* Implémente les fonctionnalités de gestion de processus sur une machine linux */

#define _GNU_SOURCE
#include "process.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <pwd.h>


// verifier si une chaîne est un nombre
static int is_number(const char *s) {
    if (!s || !*s) return 0;
    while (*s) {
        if (!isdigit((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}
// convertir uid en nom d'utilisateur
static void uid_to_user(uid_t uid, char *buffer, size_t size) {
    struct passwd *pw = getpwuid(uid);
    if (pw && pw->pw_name) {
        strncpy(buffer, pw->pw_name, size);
        buffer[size - 1] = '\0';
    } else {
        snprintf(buffer, size, "%d", (int)uid);
    }
}
// lire le fichier /proc/[pid]/status pour obtenir les informations du processus
static int read_status(int pid,
                       char *state,
                       long *mem_kb,
                       char *cmd,
                       size_t cmd_sz,
                       char *user,
                       size_t user_sz)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char line[512];
    *mem_kb = 0;
    *state = '?';
    cmd[0] = '\0';
    uid_t uid = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "Name:", 5) == 0)
            sscanf(line, "Name:\t%255s", cmd);

        else if (strncmp(line, "State:", 6) == 0)
            sscanf(line, "State:\t%c", state);

        else if (strncmp(line, "VmRSS:", 6) == 0)
            sscanf(line, "VmRSS:%ld kB", mem_kb);

        else if (strncmp(line, "Uid:", 4) == 0)
            sscanf(line, "Uid:\t%u", &uid);
    }
    fclose(f);
    uid_to_user(uid, user, user_sz);
    return 0;
}

// ajouter un processus au début de la liste chaînée
static Process *push_front(Process *head, Process *p) {
    p->next = head;
    return p;
}

 // Lecture de la liste des processus
 
    Process *read_processes(void) {
    DIR *d = opendir("/proc"); // ouvrir le répertoire /proc
    if (!d) { // erreur
        perror("opendir /proc");
        return NULL;
    }

    struct dirent *ent; // entrée de répertoire
    Process *head = NULL; // tête de la liste chaînée

    while ((ent = readdir(d)) != NULL) { // lire chaque entrée
        if (!is_number(ent->d_name)) // ignorer si ce n'est pas un nombre
            continue;

        int pid = atoi(ent->d_name); // convertir en entier

        Process *p = calloc(1, sizeof(Process)); // allouer de la mémoire pour le processus
        if (!p) continue;

        p->pid = pid; // définir le PID

        if (read_status(pid, &p->state, &p->mem_kb,
                        p->cmd, sizeof(p->cmd),
                        p->user, sizeof(p->user)) != 0)
        {
            free(p); // erreur de lecture, libérer la mémoire
            continue;
        }

        head = push_front(head, p); // ajouter à la liste chaînée
    }

    closedir(d);
    return head;
}

// Trier la liste des processus par utilisation de la mémoire (décroissant)
Process *sort_by_mem(Process *head) { 
    Process *sorted = NULL; // la tête de la liste triée

    while (head) { // parcourir chaque processus
        Process *next = head->next;

        if (!sorted || head->mem_kb > sorted->mem_kb) { // comparer et insérer
            head->next = sorted;
            sorted = head;
        } else { // sinon trouver la bonne position
            Process *cur = sorted;
            while (cur->next && cur->next->mem_kb >= head->mem_kb)
                cur = cur->next;

            head->next = cur->next;
            cur->next = head;
        }

        head = next; // passer au processus suivant
    }

    return sorted;
}

// petit focntion d'affichage des processus pour tester (deso basil)
void print_processes(Process *head, const char *machine_name) {
    if (!machine_name) machine_name = "local";

    printf("=== Process list from [%s] ===\n", machine_name);
    printf("%-7s %-12s %-8s %-5s %-s\n",
           "PID", "USER", "MEM(kB)", "ST", "CMD");
    printf("-----------------------------------------------------------\n");

    int count = 0;
    for (Process *p = head; p; p = p->next) {
        printf("%-7d %-12s %-8ld %-5c %-s\n",
               p->pid, p->user, p->mem_kb, p->state, p->cmd);

        if (++count == 50) {
            printf("... (truncated)\n");
            break;
        }
    }
}

// Libérer la mémoire allouée pour la liste des processus
void free_processes(Process *head) {
    while (head) {
        Process *tmp = head;
        head = head->next;
        free(tmp);
    }
}
// FIN LOCAL PROCESS

// Partie network (serialization et deserialization)
size_t serialize_processes(Process *head, char *buffer, size_t bufsize) {
    size_t used = 0; // nombre d'octets utilisés dans le tampon

    for (Process *p = head; p; p = p->next) { // parcourir chaque processus
        int n = snprintf(buffer + used, bufsize - used, "%d;%s;%ld;%c;%s\n", p->pid, p->user, p->mem_kb, p->state, p->cmd); // sérialiser le processus

        if (n < 0 || (size_t)n >= bufsize - used) // vérifier les erreurs si le token ne rentre pas
            break;

        used += (size_t)n; // mettre à jour le nombre d'octets utilisés
    }

    return used; // retourner le nombre total d'octets utilisés
}

 // Deserialization


static int parse_line(const char *line, Process *p) { // analyser la chaine qui représente un processus serialisé
    char tmp[512]; // token temporaire pour la ligne
    strncpy(tmp, line, sizeof(tmp)); // copier la ligne dans le token temporaire
    tmp[sizeof(tmp) - 1] = '\0';

    char *saveptr = NULL; // pointeur pour strtok_r

    char *pid_str   = strtok_r(tmp, ";", &saveptr); // extraire chaque champ
    char *user_str  = strtok_r(NULL, ";", &saveptr);
    char *mem_str   = strtok_r(NULL, ";", &saveptr);
    char *state_str = strtok_r(NULL, ";", &saveptr);
    char *cmd_str   = strtok_r(NULL, "\n", &saveptr);

    if (!pid_str || !user_str || !mem_str || !state_str || !cmd_str) // vérifier si tous les champs sont présents
        return -1;

    p->pid = atoi(pid_str); // convertir et assigner les champs
    strncpy(p->user, user_str, sizeof(p->user)); // copier le nom d'utilisateur
    p->mem_kb = atol(mem_str); // convertir et assigner la mémoire 
    p->state = state_str[0]; // assigner l'état
    strncpy(p->cmd, cmd_str, sizeof(p->cmd)); // copier la commande
    p->next = NULL; // initialiser le pointeur suivant à NULL
    return 0;
}
// désérialiser la chaîne en une liste chaînée de processus
Process *deserialize_processes(const char *buffer) {
    if (!buffer) return NULL;

    char *copy = strdup(buffer); // faire une copie modifiable du tampon
    if (!copy) return NULL; // vérifier l'allocation

    Process *head = NULL; // tête de la liste chaînée
    char *saveptr = NULL; // pointeur pour strtok_r
    char *line = strtok_r(copy, "\n", &saveptr); // diviser en lignes
 
    while (line) { // pour chaque ligne
        Process *p = calloc(1, sizeof(Process)); // allouer de la mémoire pour un nouveau processus
        if (!p) break;

        if (parse_line(line, p) == 0) //    analyser la ligne et ajouter à la liste chaînée
            head = push_front(head, p); // ajouter à la liste chaînée
        else
            free(p); // erreur d'analyse, libérer la mémoire

        line = strtok_r(NULL, "\n", &saveptr); // passer à la ligne suivante
    }

    free(copy); // libérer la copie temporaire
    return head; // retourner la tête de la liste chaînée
}
