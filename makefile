# Compilador y opciones
CC = gcc
CFLAGS = -Wall -Werror

# Directorios
BIN_DIR = bin

# Ejecutables
VIEW = $(BIN_DIR)/view
PLAYER = $(BIN_DIR)/player

# Archivos fuente
VIEW_SRC = view.c
PLAYER_SRC = player.c

# Archivos objeto
VIEW_OBJ = $(BIN_DIR)/view.o
PLAYER_OBJ = $(BIN_DIR)/player.o

# Regla por defecto: compilar todo
all: $(VIEW) $(PLAYER)

# Compilar la vista
$(VIEW): $(VIEW_OBJ)
	$(CC) $(CFLAGS) -o $(VIEW) $(VIEW_OBJ)

# Compilar el jugador
$(PLAYER): $(PLAYER_OBJ)
	$(CC) $(CFLAGS) -o $(PLAYER) $(PLAYER_OBJ)

# Regla genérica para compilar archivos .c en .o
$(BIN_DIR)/%.o: %.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Crear el directorio bin si no existe
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Limpiar archivos generados
clean:
	rm -f $(BIN_DIR)/*

# Ejecutar ChompChamps
run: all
	./ChompChamps -w 10 -h 10 -t 5 -p $(PLAYER) $(PLAYER) -v $(VIEW) -d 100
