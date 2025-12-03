#include "process.h"   
#include <stdio.h>
#include <unistd.h>
#include <string.h>

static void clear_screen(void) {
    printf("\033[H\033[J");
}

int main(int argc, char *argv[]) {
    int loop = 0;

    if (argc > 1 && strcmp(argv[1], "-loop") == 0) {
        loop = 1;
    }

    if (!loop) {
       
        Process *list = read_processes();
        if (!list) {
            fprintf(stderr, "Failed to read processes.\n");
            return 1;
        }

        list = sort_by_mem(list);
        print_processes(list, "local");
        free_processes(list);

        return 0;
    } else {
        
        while (1) {
            Process *list = read_processes();
            if (!list) {
                fprintf(stderr, "Failed to read processes.\n");
                return 1;
            }

            list = sort_by_mem(list);

            clear_screen();
            print_processes(list, "local");
            printf("\nPress Ctrl+C to exit. (loop mode)\n");

            free_processes(list);
            sleep(1);
        }
    }

    return 0;
}