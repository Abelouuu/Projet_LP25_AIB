#define _GNU_SOURCE
#include "process.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>

// Lecture des processus
Process *read_proc(int sockfd, ssh_session session) {
    char output[65536];
    char *line;
    Process *head = NULL;
    Process *tail = NULL;

    if (sockfd >= 0) {
        /* lecture via Telnet */
        if (telnet_exec(sockfd,
                     "ps -eo pid,user,pcpu,pmem,state,args --no-headers\n",
                     output) != 0) {
            return NULL;
        }

    } else if (session) {
        /* lecture via SSH */
        if (ssh_exec(session,
                     "ps -eo pid,user,pcpu,pmem,state,args --no-headers",
                     output,
                     sizeof(output)) != 0) {
            return NULL;
        }
    } else {
        /* lecture locale */
        FILE *fp = popen("ps -eo pid,user,pcpu,pmem,state,args --no-headers", "r");
        if (!fp) {
            return NULL;
        }

        size_t len = 0;
        output[0] = '\0';
        while (fgets(output + len, sizeof(output) - len, fp)) {
            len = strlen(output);
            if (len >= sizeof(output) - 1) break;
        }
        pclose(fp);
    }

    /* parsing ligne par ligne */
    line = strtok(output, "\n");
    while (line) {
        Process *proc = malloc(sizeof(Process));
        if (!proc) {
            while (head) {
                Process *tmp = head;
                head = head->next;
                free(tmp);
            }
            return NULL;
        }

        memset(proc, 0, sizeof(Process));

        if (sscanf(line,
           "%d %31s %lf %lf %c %255s",
           &proc->pid,
           proc->user,
           &proc->cpu_pct,
           &proc->mem_pct,
           &proc->state,
           proc->cmd) != 6) {
            free(proc);
            line = strtok(NULL, "\n");
            continue;
        }

        proc->next = NULL;
        if (!head) {
            head = proc;
            tail = proc;
        } else {
            tail->next = proc;
            tail = proc;
        }

        line = strtok(NULL, "\n");
    }

    return head;
}
// tri par %MEM décroissant
Process *sort_by_mem(Process *head) {
    Process *sorted = NULL;

    while (head) {
        Process *current = head;
        head = head->next;

        /* insertion en tête */
        if (!sorted || current->mem_pct > sorted->mem_pct) {
            current->next = sorted;
            sorted = current;
        } else {
            Process *p = sorted;

            while (p->next && p->next->mem_pct >= current->mem_pct) {
                p = p->next;
            }

            current->next = p->next;
            p->next = current;
        }
    }

    return sorted;
}

// libération mémoire
void free_processes(Process *head) {
    while (head) {
        Process *tmp = head;
        head = head->next;
        free(tmp);
    }
}

// envoi de signal à un processus donné (gere local ou distant via SSH et Telnet)
static int send_signal_to_pid(int pid, int sig, const char *action_name) {

    if(liste_machines[current_page].port==22 && current_ssh_session) {
        if(send_signal_ssh(current_ssh_session, pid, sig) == -1){
            perror(action_name);
            return -1;
        }
        else {
            return 0;
        }
    }

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
