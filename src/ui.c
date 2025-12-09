#include "ui.h"
#include "process.h"
#include "network.h"
#include "options.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <ncurses.h>
#include <stdbool.h>

/* Récupérer le i-ème processus dans la liste chaînée */
static Process *get_nth_process(Process *head, int index) {
    int i = 0;
    for (Process *p = head; p; p = p->next, i++) {
        if (i == index) return p;
    }
    return NULL;
}

void ui_loop_local(bool show_help) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    int selected = 0;
    int running  = 1;

        if (show_help) {
        affiche_aide("GestionRessources");
        // après fermeture de la fenêtre d'aide, on nettoie l'écran principal
        clear();
        refresh();
    }

    while (running) {
        Process *list = read_processes();
        if (!list) {
            endwin();
            fprintf(stderr, "Failed to read process list.\n");
            return;
        }

        update_mem_percentage(list);
        list = sort_by_mem(list);

        int count = 0;
        for (Process *p = list; p; p = p->next)
            count++;

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
                 "LP25 mini-htop (local)");

        /* Infos machine (exemple) */
        mvprintw(2, 0,
                 "%-15s | %-15s | %-6s | %-18s | %-15s",
                 "Nom", "Adresse", "Port", "Utilisateur", "Connexion");

        mvprintw(3, 0,
                 "%-15s | %-15s | %-6d | %-18s | %-15s",
                 "machine",
                 "127.37681.23",
                 22,
                 "Abel",
                 "ssh");
                 
        mvprintw(5, 0,
                     "Aide : fleches = deplacer  |  q = quitter  |  k = kill (soft)  |  p = pause  |  r = reprise");

        /* En-tête des processus */
        mvprintw(7, 0, "%s\t%s\t%s\t%s\t%s",
                 "PID", "USER", "MEM(%)", "ST", "CMD");

        int row  = 8;
        int idx  = 0;
        int max_rows = LINES - 2;

        for (Process *p = list; p && row < max_rows; p = p->next, row++, idx++) {
            if (idx == selected)
                attron(A_REVERSE);

            int max_cmd_len = COLS - 40;
            if (max_cmd_len < 0) max_cmd_len = 0;

            if (max_cmd_len > 0) {
                mvprintw(row, 0, "%d\t%s\t%2.2f\t%c\t%.*s",
                         p->pid, p->user, p->mem_pct, p->state,
                         max_cmd_len, p->cmd);
            } else {
                mvprintw(row, 0, "%d\t%s\t%2.2f\t%c\t",
                         p->pid, p->user, p->mem_pct, p->state);
            }

            if (idx == selected)
                attroff(A_REVERSE);
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
        usleep(150000);
    }

    endwin();
}