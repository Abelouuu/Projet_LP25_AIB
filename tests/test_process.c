#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"

static void print_usage(const char *prog) {
    printf("Usage:\n");
    printf("  %s              -> liste les processus, tries par %%MEM\n", prog);
    printf("  %s softkill PID -> envoie SIGTERM au PID\n", prog);
    printf("  %s hardkill PID -> envoie SIGKILL au PID\n", prog);
    printf("  %s pause PID    -> envoie SIGSTOP au PID\n", prog);
    printf("  %s cont PID     -> envoie SIGCONT au PID\n", prog);
}

int main(int argc, char *argv[]) {

    // --- Mode "commande sur un PID" ---
    if (argc == 3) {
        const char *cmd = argv[1];
        int pid = atoi(argv[2]);
        if (pid <= 0) {
            fprintf(stderr, "Invalid PID: %s\n", argv[2]);
            return 1;
        }

        if (strcmp(cmd, "softkill") == 0) {
            if (kill_process_soft(pid) == 0)
                printf("Sent SIGTERM to PID %d\n", pid);
        }
        else if (strcmp(cmd, "hardkill") == 0) {
            if (kill_process_hard(pid) == 0)
                printf("Sent SIGKILL to PID %d\n", pid);
        }
        else if (strcmp(cmd, "pause") == 0) {
            if (pause_process(pid) == 0)
                printf("Sent SIGSTOP to PID %d\n", pid);
        }
        else if (strcmp(cmd, "cont") == 0) {
            if (continue_process(pid) == 0)
                printf("Sent SIGCONT to PID %d\n", pid);
        }
        else {
            print_usage(argv[0]);
            return 1;
        }

        return 0;
    }

    // --- Mode listing (par défaut) ---
    if (argc != 1) {
        print_usage(argv[0]);
        return 1;
    }

    Process *plist = read_processes();
    if (!plist) return 1;

    update_mem_percentage(plist);
    plist = sort_by_mem(plist);
    print_processes(plist, "local");

    free_processes(plist);
    return 0;
}
