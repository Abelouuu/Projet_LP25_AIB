#include "ui.h"
#include "process.h"
#include "network.h"
#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>

/* Récupérer le i-ème processus */
static Process *get_nth_process(Process *head, int index) {
    int i = 0;
    for (Process *p = head; p; p = p->next, i++) {
        if (i == index) {
            return p;
        }
    }
    return NULL;
}


void ui_init() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
}

void ui_shutdown() {
    endwin();
}


void affiche_aide() {
    int height = 15;
    int width  = 70;
    int starty = (LINES - height) / 2;
    int startx = (COLS  - width)  / 2;

    if (starty < 0) {
        starty = 0;
    }

    if (startx < 0) {
        startx = 0;
    }

    WINDOW *win = newwin(height, width, starty, startx);
    box(win, 0, 0);

    mvwprintw(win, 1, 2, "Utilisation : ./GestionRessources [options]");
    mvwprintw(win, 2, 2, "Options :");

    mvwprintw(win, 4,  2, "  -h, --help                 Afficher cette aide");
    mvwprintw(win, 5,  2, "  --dry-run                  Tester l'accès sans afficher");
    mvwprintw(win, 6,  2, "  -c, --remote-config FILE   Fichier de configuration distant");
    mvwprintw(win, 7,  2, "  -t, --connection-type TYPE Type de connexion");
    mvwprintw(win, 8,  2, "  -P, --port PORT            Port de connexion");
    mvwprintw(win, 9,  2, "  -l, --login LOGIN          Login");
    mvwprintw(win,10, 2, "  -s, --remote-server SERVER Serveur distant");
    mvwprintw(win,11, 2, "  -u, --username USERNAME    Nom d'utilisateur");
    mvwprintw(win,12, 2, "  -p, --password PASSWORD    Mot de passe");
    mvwprintw(win,13, 2, "  -a, --all                  Appliquer à tous les éléments");

    mvwprintw(win, height - 2, 2, "Appuyez sur une touche pour fermer cette aide...");

    wrefresh(win);
    wgetch(win);
    delwin(win);
}


void affichePrinc(Process *list, int selected) {

    clear();

    start_color();
    use_default_colors(); // pour pouvoir utiliser le fond par défaut du terminal

    // Définition des paires de couleurs
    init_pair(1, COLOR_CYAN, -1);    // titre
    init_pair(2, COLOR_YELLOW, -1);  // noms de colonnes
    init_pair(3, COLOR_GREEN, -1);   // première machine
    init_pair(4, COLOR_MAGENTA, -1); // ligne d'aide
    init_pair(5, COLOR_BLUE, -1);    // titres processus

    // Titre
    attron(COLOR_PAIR(1));
    mvprintw(0, 0, "LP25 mini-htop (local)");
    attroff(COLOR_PAIR(1));

    // En-tête tableau
    attron(COLOR_PAIR(2));
    mvprintw(2, 0,
             "%-15s | %-15s | %-6s | %-18s | %-15s",
             "Nom", "Adresse", "Port", "Utilisateur", "Connexion");
    attroff(COLOR_PAIR(2));

    // Première ligne de données
    attron(COLOR_PAIR(3));
    mvprintw(3, 0,
             "%-15s | %-15s | %-6d | %-18s | %-15s",
             "machine",
             "127.37681.23",
             22,
             "Abel",
             "ssh");
    attroff(COLOR_PAIR(3));

    // Ligne d'aide
    attron(COLOR_PAIR(4));
    mvprintw(5, 0,
             "Aide : fleches = deplacer  |  q = quitter  |  k = kill (soft)  |  p = pause  |  r = reprise | h = aide");
    attroff(COLOR_PAIR(4));

    // Titres des processus
    attron(COLOR_PAIR(5));
    mvprintw(7, 0, "%s\t%s\t%s\t%s\t%s",
             "PID", "USER", "MEM(%)", "ST", "CMD");
    attroff(COLOR_PAIR(5));

    int row = 8;
    int idx = 0;
    int max_rows = LINES - 2;

    for (Process *p = list; p && row < max_rows; p = p->next, row++, idx++) {

        if (idx == selected) {
            //Mettre en surbrillance
            attron(A_REVERSE);
        }

        int max_cmd_len = COLS - 40;
        if (max_cmd_len < 0) {
            max_cmd_len = 0;
        }

        if (max_cmd_len > 0) {
            mvprintw(row, 0, "%d\t%s\t%2.2f\t%c\t%.*s",
                     p->pid, p->user, p->mem_pct, p->state,
                     max_cmd_len, p->cmd);
        } else {
            mvprintw(row, 0, "%d\t%s\t%2.2f\t%c\t",
                     p->pid, p->user, p->mem_pct, p->state);
        }

        if (idx == selected) {
            //Désactiver la surbrillance
            attroff(A_REVERSE);
        }
    }

    refresh();
}


void ui_traite_event(int ch, int *selected, int count, int *running, Process *list) {

    if (ch == KEY_UP) {
        (*selected)--;
        if (*selected < 0) {
            *selected = 0;
        }
    }
    else if (ch == KEY_DOWN) {
        (*selected)++;
        if (*selected >= count) {
            *selected = count - 1;
        }
    }
    else if (ch == 'q' || ch == 'Q') {
        *running = 0;
    }
    else if (ch == 'h' || ch == 'H') {
        affiche_aide();
    }
    else if ((ch == 'k' || ch == 'K') && count > 0) {
        Process *p = get_nth_process(list, *selected);
        if (p) {
            kill_process_soft(p->pid);
        } 
    }
    else if (ch == 'p' || ch == 'P') {
        Process *p = get_nth_process(list, *selected);
        if (p) {
            pause_process(p->pid);
        }
    }
    else if (ch == 'r' || ch == 'R') {
        Process *p = get_nth_process(list, *selected);
        if (p) {
            continue_process(p->pid);
        }
    }
}

>>>>>>> Basile
