#include "ui.h"
#include "process.h"
#include "network.h"
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

ui_state_t ui_state;

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

void handle_resize() {
    endwin();
    refresh();
    clear();
    refresh();
}

void ui_init() {
    printf("connection...");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(150);//permet de ne pas bloquer l'attente d'une entrée utilisateur (getch)
    start_color();
    use_default_colors(); // pour pouvoir utiliser le fond par défaut du terminal

    // Définition des paires de couleurs
    init_pair(1, COLOR_CYAN, -1);    // titre
    init_pair(2, COLOR_YELLOW, -1);  // noms de colonnes
    init_pair(3, COLOR_GREEN, -1);   // première machine
    init_pair(4, COLOR_MAGENTA, -1); // ligne d'aide
    init_pair(5, COLOR_BLUE, -1);    // titres processus
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
    if (!win) {
        return;
    }
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

void afficher_erreur_connection(remote_machine *machine) {
    int height = 10, width = 60;
    int starty = (LINES - height) / 2;
    int startx = (COLS - width) / 2;

    WINDOW *win = newwin(height, width, starty, startx);
    box(win, 0, 0);

    char message[256];
    snprintf(message, sizeof(message),
             "Impossible de se connecter à '%s' (%s:%d)",
             machine->name, machine->address, machine->port);

    mvwprintw(win, 2, (width - strlen(message)) / 2, "%s", message);
    mvwprintw(win, 4, 2, "Appuyez sur F2 pour passer à l'onglet suivant");
    mvwprintw(win, 5, 2, "Appuyez sur F3 pour revenir à l'onglet précédent");

    wrefresh(win);

    int ch;
    while ((ch = wgetch(win)) != KEY_F(2) && ch != KEY_F(3));
    delwin(win);

    touchwin(stdscr);
    refresh();

    // ici tu peux gérer current_page selon F2/F3
}


void affichePrinc(Process *list, int selected, remote_machine machine) {
    clear();

    // Titre
    attron(COLOR_PAIR(1));
    mvprintw(0, 0, "LP25 mini-htop Page :%d/%d", current_page + 1, nb_machines);
    attroff(COLOR_PAIR(1));

    // En-tête tableau
    attron(COLOR_PAIR(2));
    mvprintw(2, 0,
             "%-15s | %-15s | %-6s | %-18s | %-15s",
             "Nom", "Adresse", "Port", "Utilisateur", "Connexion");
    attroff(COLOR_PAIR(2));

    // Première ligne de données
    attron(COLOR_PAIR(3));
    if(strcmp(machine.name,"Localhost")==0){
        mvprintw(3, 0,
             "Machine Locale ");
    } else {
        mvprintw(3, 0,
             "%-15s | %-15s | %-6d | %-18s | %-15s",
             machine.name,
             machine.address,
             machine.port,
             machine.username,
             machine.conn_type);
    }
    attroff(COLOR_PAIR(3));

    // Ligne d'aide
    attron(COLOR_PAIR(4));
    mvprintw(5, 0,
             "Aide : fleches = deplacer  |  q = quitter  |  F2 = page précedentes  |  F3 = page suivante F4 = rechercher | F5 = pause | F6 = arreter | F7 = tuer | F8 = redémarrer");
    attroff(COLOR_PAIR(4));

    // Titres des processus
    attron(COLOR_PAIR(5));
    mvprintw(7, 0, "%s\t%s\t%s\t%s\t%s",
             "PID", "USER", "MEM(%)", "ST", "CMD");
    attroff(COLOR_PAIR(5));

    if (list == NULL)
    {
        mvprintw(9, 0, "Erreur de connexion à la machine distante ou aucun processus récupéré.");
        refresh();
        return;
    } 
    else {
        int row = 9;
        int idx = 0;
        int max_rows = LINES - 2;

        // création de l'effet de "scroll" lorsque l'utilisateur descent trop bas dans les processus
        int decalage = 0;
        if (selected >= max_rows - 8) {
            decalage = selected - (max_rows - 8) + 1; // on fait défiler pour garder la sélection visible
        }

        for (Process *p = list; p && row < max_rows; p = p->next, idx++) {
            if (idx < decalage) {
                continue;
            }

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

            row++;
        }

        refresh();

        if (ui_state.mode == MODE_RECHERCHE) {
            draw_search_bar();
        }
    }
}


void ui_traite_event(int ch, int *selected, int nbproc, int *running, Process *list) {

    // Verifie d'abord si il s'agit d'une commande de changement de mode
    if (ch == KEY_F(4)) {
        if(ui_state.mode == MODE_RECHERCHE) {
            // quitter le mode recherche
            ui_state.mode = MODE_NORMAL;
            ui_state.search_query[0] = '\0';
            noecho();
            curs_set(0);
        } else {
            // entrer en mode recherche
            ui_state.mode = MODE_RECHERCHE;
            ui_state.search_query[0] = '\0';
            draw_search_bar();
        }
    }

    // MODE RECHERCHE
    if (ui_state.mode == MODE_RECHERCHE) {
        if (ch == '\n') {
            /* lancer la recherche */
            ui_state.mode = MODE_NORMAL;

            noecho();
            curs_set(0);
            if(ui_state.search_query[0] != '\0') {
                int idx = 0;
                int found = -1;
                for(Process *p = list;p;p = p->next, idx++) {
                    if(processus_recherche(p,ui_state.search_query) == 1) {
                        found = idx;
                        break;
                    }
                }
                if(found >=0) {
                    *selected = found;
                }
            }
        } else if (ch == 27) { // ESC
            ui_state.mode = MODE_NORMAL;
            noecho();
            curs_set(0);
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            size_t len = strlen(ui_state.search_query);
            if (len > 0)
                ui_state.search_query[len - 1] = '\0';
        } else if (isprint(ch)) {
            size_t len = strlen(ui_state.search_query);
            if (len < sizeof(ui_state.search_query) - 1) {
                ui_state.search_query[len] = ch;
                ui_state.search_query[len + 1] = '\0';
            }
        }
        return;
    }


    // MODE NORMAL
    if (ch == KEY_UP) {
        (*selected)--;
        if (*selected < 0) {
            *selected = 0;
        }
    }
    else if (ch == KEY_DOWN) {
        (*selected)++;
        if (*selected >= nbproc) {
            *selected = nbproc - 1;
        }
    }
    else if (ch == 'q' || ch == 'Q') {
        *running = 0;
    }
    else if (ch == KEY_F(1)) {
        affiche_aide();
    }

    //page précedentes
    else if (ch == KEY_F(2)) {
        if(current_page>0) {
            //déconnecter de ssh si c'était une connection ssh
            if(liste_machines[current_page].port==22 && current_ssh_session) {
                ssh_disconnect(current_ssh_session);
                ssh_free(current_ssh_session);
            } 
            //déconnecter de telnet si c'était une connection telnet
            else if(liste_machines[current_page].port==23 && current_telnet_session != -1){
                close(current_telnet_session);
                current_telnet_session = -1;
            }

            // revenir à la page précédente
            current_page--;

            if(current_page != 0){
                //si la nouvelle page correspond à une connection ssh
                if(liste_machines[current_page].port==22) {
                    current_ssh_session = connection_ssh(&liste_machines[current_page]);
                } else if(liste_machines[current_page].port==23){
                    //sinon c'est une connection telnet
                    current_telnet_session = connection_telnet(&liste_machines[current_page]);
                }
                
            }
        }
    }

    //page suivante
    else if (ch == KEY_F(3)) {
        if(current_page<nb_machines-1) {
    
            if(liste_machines[current_page].port==22) {
                if(current_ssh_session) {
                    ssh_disconnect(current_ssh_session);
                    ssh_free(current_ssh_session);
                }
            } 
            else if(liste_machines[current_page].port==23 && current_telnet_session != -1){
                close(current_telnet_session);
                current_telnet_session = -1;
            }

            current_page++;
            
            if(liste_machines[current_page].port==22) {
                current_ssh_session = connection_ssh(&liste_machines[current_page]);
            }
            else if(liste_machines[current_page].port==23){
                current_telnet_session = connection_telnet(&liste_machines[current_page]);
            }
        }
    }

    //mettre en pause le processus sélectionné
    else if (ch == KEY_F(5)) {
        Process *p = get_nth_process(list, *selected);
        if (p) {
            pause_process(p->pid);
        }
    }
    // arreter le processus sélectionné
    else if ((ch == KEY_F(6)) && nbproc > 0) {
        Process *p = get_nth_process(list, *selected);
        if (p) {
            kill_process_soft(p->pid);
        } 
    }
    // tuer le processus sélectionné
    else if ((ch == KEY_F(7)) && nbproc > 0) {
        Process *p = get_nth_process(list, *selected);
        if (p) {
            kill_process_hard(p->pid);
        } 
    }
    // redémarrer le processus sélectionné
    else if (ch == KEY_F(8)) {
        Process *p = get_nth_process(list, *selected);
        if (p) {
            continue_process(p->pid);
        }
    }
}

void draw_search_bar() {
    int row = LINES - 1;

    echo();
    curs_set(1);

    mvprintw(row, 0, "Recherche: %s", ui_state.search_query);
    clrtoeol();
    move(row, 11 + strlen(ui_state.search_query));
}
