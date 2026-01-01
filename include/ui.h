#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include "process.h"
#include "network.h"
#include "global.h"

typedef enum {
    MODE_NORMAL,
    MODE_RECHERCHE
} ui_mode_t;

typedef struct {
    ui_mode_t mode;
    char search_query[128];
} ui_state_t;

//foncton de gestion du redimensionnement de la fenêtre
void handle_resize();

/* Initialisation et fermeture de l'interface */
void ui_init();
void ui_shutdown();

/* Affichage principal */
void affichePrinc(Process *list, int selected, remote_machine machine);

/* Gestion d'un événement utilisateur */
void ui_traite_event(int ch, int *selected, int count, int *running, Process *list);

/* Affichage de la fenêtre d'aide */
void affiche_aide();

void draw_search_bar();
#endif
