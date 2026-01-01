EXEC=GestionRessources
CC=gcc
CFLAGS=-Wall -Wextra -Werror -g -Iinclude

# Fichiers sources
SRCS=$(wildcard src/*.c)

# Transformation des fichiers sources en fichiers objets
OBJS := $(patsubst src/%.c,obj/%.o,$(SRCS))

all: $(EXEC)
# création de l'exécutable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lncurses -lssh

# création des fichiers objets
obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)