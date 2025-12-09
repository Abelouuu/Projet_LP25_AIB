#include "ui.h"
#include "process.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <ncurses.h>
#include "network.h"

/* Récupérer le i-ème processus dans la liste chaînée */
static Process *get_nth_process(Process *head, int index) {
    int i = 0;
    for (Process *p = head; p; p = p->next, i++) {
        if (i == index) return p;
    }
    return NULL;
}

void ui_loop_local(void) {
    initscr();              // Initialise ncurses
    cbreak();               // Mode caractère par caractère
    noecho();               // Ne pas afficher les touches
    keypad(stdscr, TRUE);   // Activer les flèches
    curs_set(0);            // Cacher le curseur

    int selected = 0;
    int running  = 1;

    while (running) {
        Process *list = read_processes();
        if (!list) {
            endwin();
            fprintf(stderr, "Failed to read process list.\n");
            return;
        }

        list = sort_by_mem(list);

        // Compter les processus
        int count = 0;
        for (Process *p = list; p; p = p->next) count++;

        if (count == 0) {
            free_processes(list);
            endwin();
            fprintf(stderr, "No processes found.\n");
            return;
        }

        if (selected >= count) selected = count - 1;
        if (selected < 0)      selected = 0;

        clear();

        mvprintw(0, 0,
                 "LP25 mini-htop (local)  |  q=quit  fleches=move  k=kill  p=pause  r=unpause");

        mvprintw(2, 0,
            "%-15s | %-15s | %-6s | %-18s | %-15s",
            "Nom", "Adresse", "Port", "Utilisateur", "Connexion");

        mvprintw(3, 0,
            "%-15s | %-15s | %-6d | %-18s | %-15s",
            machine_test.name,
            machine_test.address,
            machine_test.port,
            machine_test.username,
            machine_test.conn_type);

        mvprintw(5, 0, "%-6s %-12s %-10s %-4s %-s",
                 "PID", "USER", "MEM(kB)", "ST", "CMD");
                 int row  = 6;
        int idx  = 0;
        int max_rows = LINES - 2;  // éviter de dépasser la hauteur de l'écran

        for (Process *p = list; p && row < max_rows; p = p->next, row++, idx++) {
            if (idx == selected) attron(A_REVERSE);

            int max_cmd_len = COLS - 40;   // largeur max pour CMD
            if (max_cmd_len < 0) max_cmd_len = 0;

            if (max_cmd_len > 0)
                mvprintw(row, 0, "%-6d %-12s %-10ld %-4c %-.*s",
                         p->pid, p->user, p->mem_kb, p->state,
                         max_cmd_len, p->cmd);
            else
                mvprintw(row, 0, "%-6d %-12s %-10ld %-4c",
                         p->pid, p->user, p->mem_kb, p->state);

            if (idx == selected) attroff(A_REVERSE);
        }

        refresh();

        int ch = getch();
        if (ch == KEY_UP) {
            selected--;
        } else if (ch == KEY_DOWN) {
            selected++;
        } else if (ch == 'q' || ch == 'Q') {
            running = 0;
        } else if ((ch == 'k' || ch == 'K') && count > 0) {
            Process *p = get_nth_process(list, selected);
            if (p) kill_process_soft(p->pid);
        } else if (ch == 'p' || ch == 'P') {
            Process *p = get_nth_process(list, selected);
            if (p) pause_process(p->pid); 
        } else if (ch == 'r' || ch == 'R') {
            Process *p = get_nth_process(list, selected);
            if (p) continue_process(p->pid);
        }

        free_processes(list);

        // petit délai pour ne pas surcharger la machine
        usleep(150000);  // 150 ms
    }

    endwin();
}
