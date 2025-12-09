#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include "process.h"



/* Boucle d'affichage interactive façon htop (avec ncurses) */
void ui_loop_local(bool show_help);

#endif