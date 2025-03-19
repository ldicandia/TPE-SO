# Compilador y opciones
CC = gcc
CFLAGS = -Wall -Werror

# Ejecutables
VIEW = view
PLAYER = player

# Archivos fuente
VIEW_SRC = view.c
PLAYER_SRC = player.c

# Archivos objeto
VIEW_OBJ = $(VIEW_SRC:.c=.o)
PLAYER_OBJ = $(PLAYER_SRC:.c=.o)

# Regla por defecto: compilar todo
all: $(VIEW) $(PLAYER)

# Compilar la vista
$(VIEW): $(VIEW_OBJ)
	$(CC) $(CFLAGS) -o $(VIEW) $(VIEW_OBJ)

# Compilar el jugador
$(PLAYER): $(PLAYER_OBJ)
	$(CC) $(CFLAGS) -o $(PLAYER) $(PLAYER_OBJ)

# Regla genérica para compilar archivos .c en .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	rm -f $(VIEW) $(PLAYER) *.o

# Ejecutar ChompChamps
run: all
	./ChompChamps -w 10 -h 10 -t 5 -p ./player -v ./view -d 1000
