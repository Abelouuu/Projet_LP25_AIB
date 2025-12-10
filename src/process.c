#define _GNU_SOURCE
#include "process.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>



// vérifier si une chaîne est entièrement numérique (pour les PID)
static int is_number(const char *s) {
    if (!s || !*s) return 0;
    while (*s) {
        if (!isdigit((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}

// recherche à la casse si une chaîne contient une autre
static int contient(const char *txt, const char *rech) {
    if (!txt || !rech || !*rech) return 0;

    // Versions en minuscules
    char c1, c2;
    size_t len_txt = strlen(txt);
    size_t len_rech = strlen(rech);

    if (len_rech > len_txt) return 0;

    for (size_t i = 0; i <= len_txt - len_rech; i++) {
        size_t j = 0;
        while (j < len_rech) {
            c1 = (char)tolower((unsigned char)txt[i + j]);
            c2 = (char)tolower((unsigned char)rech[j]);
            if (c1 != c2) break;
            j++;
        }
        if (j == len_rech) return 1; // trouvé
    }
    return 0;
}


// convertir un uid  en nom d'utilisateur
static void uid_to_user(uid_t uid, char *buffer, size_t size) {
    struct passwd *pw = getpwuid(uid);
    if (pw && pw->pw_name) {
        strncpy(buffer, pw->pw_name, size);
        buffer[size - 1] = '\0';
    } else {
        snprintf(buffer, size, "%d", (int)uid);
    }
}

// lire /proc/[pid]/status pour récupérer : Name, State, VmRSS, Uid
// lire /proc/[pid]/status pour récupérer : Name, State, VmRSS, Uid
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

        if (strncmp(line, "Name:", 5) == 0) {
           
            if (cmd && cmd_sz > 1) {
                char fmt[32];
                snprintf(fmt, sizeof(fmt), "Name:\t%%%zus", cmd_sz - 1);
                if (sscanf(line, fmt, cmd) != 1) {
                    cmd[0] = '\0';
                }
            }

        } else if (strncmp(line, "State:", 6) == 0) {
            // ligne de type : "State:\tS (sleeping)"
            sscanf(line, "State:\t%c", state);

        } else if (strncmp(line, "VmRSS:", 6) == 0) {
            // ligne de type : "VmRSS:\t  12345 kB"
            if (sscanf(line, "VmRSS: %ld kB", mem_kb) != 1) {
                *mem_kb = 0;
            }

        } else if (strncmp(line, "Uid:", 4) == 0) {
            // ligne de type : "Uid:\t1000\t1000\t1000\t1000"
            if (sscanf(line, "Uid: %u", &uid) != 1) {
                uid = 0;
            }
        }
    }

    fclose(f);
    uid_to_user(uid, user, user_sz);
    return 0;
}


// insérer un Process au début de la liste
static Process *push_front(Process *head, Process *p) {
    p->next = head;
    return p;
}

// lire MemTotal (RAM totale) depuis /proc/meminfo, en kB
static long get_total_mem_kb(void) {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) {
        perror("fopen /proc/meminfo");
        return 0;
    }

    char line[256];
    long mem_total = 0;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            // ex: "MemTotal:       16343428 kB"
            sscanf(line, "MemTotal: %ld kB", &mem_total);
            break;
        }
    }

    fclose(f);
    return mem_total;
}


// Lecture des processus

Process *read_processes(void) {
    DIR *d = opendir("/proc");
    if (!d) {
        perror("opendir /proc");
        return NULL;
    }

    Process *head = NULL;
    struct dirent *ent;

    while ((ent = readdir(d)) != NULL) {
        if (!is_number(ent->d_name))
            continue;

        int pid = atoi(ent->d_name);

        Process *p = calloc(1, sizeof(Process));
        if (!p) continue;

        p->pid = pid;

        if (read_status(pid, &p->state, &p->mem_kb,
                        p->cmd, sizeof(p->cmd),
                        p->user, sizeof(p->user)) != 0)
        {
            free(p);
            continue;
        }

        // %MEM sera calculé plus tard par update_mem_percentage()
        p->mem_pct = 0.0;

        head = push_front(head, p);
    }

    closedir(d);
    return head;
}


// calcul du pourcentage de mémoire (%MEM)

void update_mem_percentage(Process *head) {
    long total_kb = get_total_mem_kb();
    if (total_kb <= 0) {
        // impossible de calculer, on met tout à 0
        for (Process *p = head; p; p = p->next) {
            p->mem_pct = 0.0;
        }
        return;
    }

    for (Process *p = head; p; p = p->next) { // calcule de memoire par percentage
        p->mem_pct = (double)p->mem_kb * 100.0 / (double)total_kb;
    }
}


// tri par %MEM décroissant

Process *sort_by_mem(Process *head) {
    Process *sorted = NULL;

    while (head) {
        Process *next = head->next;

        if (!sorted || head->mem_pct > sorted->mem_pct) {
            head->next = sorted;
            sorted = head;
        } else {
            Process *cur = sorted;
            while (cur->next && cur->next->mem_pct >= head->mem_pct)
                cur = cur->next;

            head->next = cur->next;
            cur->next = head;
        }

        head = next;
    }

    return sorted;
}


// affichage texte simple (pour débogage)

void print_processes(Process *head, const char *machine_name) {
    if (!machine_name) machine_name = "local";

    printf("=== Process list from [%s] ===\n", machine_name);
    printf("%-7s %-12s %-7s %-4s %-s\n",
           "PID", "USER", "%MEM", "ST", "CMD");
    printf("-----------------------------------------------------------\n");

    int count = 0;
    for (Process *p = head; p; p = p->next) {
        printf("%-7d %-12s %6.2f %-4c %-s\n",
               p->pid, p->user, p->mem_pct, p->state, p->cmd);

        if (++count == 50) {
            printf("... (truncated)\n");
            break;
        }
    }
}


// libération mémoire

void free_processes(Process *head) {
    while (head) {
        Process *tmp = head;
        head = head->next;
        free(tmp);
    }
}
// envoi de signal à un processus donné
static int send_signal_to_pid(int pid, int sig, const char *action_name) {
    if (kill(pid, sig) == -1) {
        perror(action_name);
        return -1;
    }
    return 0;
}

// Arrêt (SIGTERM)
int kill_process_soft(int pid) {
    return send_signal_to_pid(pid, SIGTERM, "kill(SIGTERM)");
}

// Arrêt (SIGKILL)
int kill_process_hard(int pid) {
    return send_signal_to_pid(pid, SIGKILL, "kill(SIGKILL)");
}

// Pause (SIGSTOP)
int pause_process(int pid) {
    return send_signal_to_pid(pid, SIGSTOP, "kill(SIGSTOP)");
}

// Reprise (SIGCONT)
int continue_process(int pid) {
    return send_signal_to_pid(pid, SIGCONT, "kill(SIGCONT)");
}
/* Implémente les fonctionnalités de gestion de processus sur une machine linux */

// recherche un processus par PID ou par nom de commande

    int processus_recherche(const Process *p, const char *rech) {
    if (!p) return 0;
    if (!rech || !*rech) return 1;  // si recherche vide → tout afficher

    char pid_txt[16];
    snprintf(pid_txt, sizeof(pid_txt), "%d", p->pid);

    if (contient(p->cmd, rech)) return 1;
    if (contient(p->user, rech)) return 1;
    if (contient(pid_txt, rech)) return 1;

    return 0;
}


