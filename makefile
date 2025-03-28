# Compilador y opciones
CC = gcc
CFLAGS = -Wall -Werror

# Directorios
BIN_DIR = bin

# Ejecutables
MASTER = $(BIN_DIR)/master_chomp
VIEW = $(BIN_DIR)/view
PLAYER = $(BIN_DIR)/player

# Archivos fuente
MASTER_SRC = master_chomp.c
VIEW_SRC = view.c
PLAYER_SRC = player.c

# Archivos objeto
MASTER_OBJ = $(BIN_DIR)/master_chomp.o
VIEW_OBJ = $(BIN_DIR)/view.o
PLAYER_OBJ = $(BIN_DIR)/player.o

# Regla por defecto: compilar todo
all: $(MASTER) $(VIEW) $(PLAYER)

# Compilar master_chomp
$(MASTER): $(MASTER_SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(MASTER) $(MASTER_SRC)


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
	./$(MASTER) -w 10 -h 10 -t 5 -p $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) -v $(VIEW) -d 500


# Ejecutar el programa con master nuestro
#gcc master_chomp.c -o master_chomp
#./master_chomp -w 10 -h 10 -t 5 -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player -v ./bin/view -d 500
