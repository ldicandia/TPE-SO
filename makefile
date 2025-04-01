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
	./$(MASTER) -w 10 -h 10 -t 10 -p $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) -v $(VIEW) -d 10

# Test the executables
test: all
	@echo "Testing master_chomp, view, and player executables..."
	@RET1=0; RET2=0; RET3=0; RET4=0; RET5=0;
	@echo "Running master_chomp with parameters..."
	@./$(MASTER) -w 10 -h 10 -t 5 -p $(PLAYER) $(PLAYER) -d 30 || RET1=$$?
	@sleep 3
	@./$(MASTER) -w 15 -h 15 -t 10 -p $(PLAYER) $(PLAYER) $(PLAYER) -d 30 || RET2=$$?
	@sleep 3
	@./$(MASTER) -w 20 -h 20 -t 3 -p $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) -d 30 || RET3=$$?
	@sleep 3
	@./$(MASTER) -w 5 -h 5 -t 2 -p $(PLAYER) $(PLAYER) -v $(VIEW) -d 30 || RET4=$$?
	@sleep 3
	@valgrind --leak-check=full --show-leak-kinds=all ./$(MASTER) -w 25 -h 25 -t 15 -p $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) -d 30 || RET5=$$?
	@sleep 3
	@echo "All tests completed."
	@echo "Test Results:"
	@echo "master_chomp test 1 exit code: $$RET1"
	@echo "master_chomp test 2 exit code: $$RET2"
	@echo "master_chomp test 3 exit code: $$RET3"
	@echo "master_chomp test 4 exit code: $$RET4"
	@echo "master_chomp test 5 exit code: $$RET5"	


# Ejecutar el programa con master nuestro
#gcc master_chomp.c -o master_chomp
#./master_chomp -w 10 -h 10 -t 3 -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player -v ./bin/view -d 500
