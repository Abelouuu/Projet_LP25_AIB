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
<<<<<<< HEAD

/* Affichage de la fenêtre d'aide */
void affiche_aide();
=======
>>>>>>> 332736e43b24e3bf92547d652e94fa2b9d82d5e5

/* Affichage de la fenêtre d'aide */
void affiche_aide();

#endif
