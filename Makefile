EXEC=GestionRessources
CC=gcc
CFLAGS=-Wall -Wextra -Werror -g -Iinclude

# Fichiers sources
SRCS=src/main.c src/ui.c src/process.c src/manager.c src/network.c

# Transformation des fichiers sources en fichiers objets
OBJS=$(SRCS:.c=.o)

all: $(EXEC)
# création de l'exécutable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# création des fichiers objets
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

#Dépendances
src/main.o: src/main.c include/ui.h include/process.h include/manager.h include/network.h
src/ui.o: src/ui.c include/ui.h include/process.h
src/process.o: src/process.c include/process.h
src/manager.o: src/manager.c include/manager.h include/process.h include/ui.h include/network.h
src/network.o: src/network.c include/network.h

clean:
	rm -f $(OBJS) $(EXEC)