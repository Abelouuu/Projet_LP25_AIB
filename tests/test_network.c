#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool verifier_login(const char *login) {
    // Cherche le caractère '@'
    const char *at_sign = strchr(login, '@');
    
    // Vérifie qu'il y a exactement un '@' et que ce n'est pas au début ni à la fin
    if (!at_sign || at_sign == login || *(at_sign + 1) == '\0') {
        return false;
    }

    // Vérifie qu'il n'y a pas d'autre '@' après le premier
    if (strchr(at_sign + 1, '@')) {
        return false;
    }

    return true;
}

int main() {
    const char *tests[] = {
        "alice@192.168.1.10",
        "bob@server.domain",
        "@serveur",
        "user@",
        "user@@server"
    };

    for (int i = 0; i < 5; i++) {
        printf("Login '%s' : %s\n", tests[i],
               verifier_login(tests[i]) ? "OK" : "Syntaxe invalide");
    }

    return 0;
}
