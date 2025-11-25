#ifndef UI_H
#define UI_H

/**
 * Initialise l'interface utilisateur.
 * À appeler avant tout affichage ou interaction.
 */
void ui_init(void);

/**
 * Affiche le menu principal ou l'écran principal.
 */
void ui_display_main(void);

/**
 * Nettoie l'interface et libère les ressources graphiques si nécessaire.
 */
void ui_cleanup(void);

#endif
