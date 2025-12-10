#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include "process.h"

/* Initialisation et fermeture de l'interface */
void ui_init();
void ui_shutdown();

/* Affichage principal */
void affichePrinc(Process *list, int selected);

/* Gestion d'un événement utilisateur */
void ui_traite_event(int ch, int *selected, int count, int *running, Process *list);

/* Affichage de la fenêtre d'aide */
void affiche_aide();

#endif