# Compilador y opciones
CC = gcc
CFLAGS = -Wall -Werror -I./tads -pthread -lrt

# Directorios
BIN_DIR = bin
SRC_DIR = .
TADS_DIR = tads

# Ejecutables
MASTER = $(BIN_DIR)/master_chomp
VIEW = $(BIN_DIR)/view
PLAYER = $(BIN_DIR)/player

# Archivos fuente
MASTER_SRC = $(SRC_DIR)/master_chomp.c
VIEW_SRC = $(SRC_DIR)/view.c
PLAYER_SRC = $(SRC_DIR)/player.c
SHMEMORY_SRC = $(TADS_DIR)/shmemory.c
GAME_LOGIC_SRC = $(TADS_DIR)/game_logic.c
ARG_PARSE_SRC = $(TADS_DIR)/arg_parser.c

# Archivos objeto
MASTER_OBJ = $(BIN_DIR)/master_chomp.o
VIEW_OBJ = $(BIN_DIR)/view.o
PLAYER_OBJ = $(BIN_DIR)/player.o
SHMEMORY_OBJ = $(BIN_DIR)/shmemory.o
GAME_LOGIC_OBJ = $(BIN_DIR)/game_logic.o

# Regla por defecto: compilar todo
all: $(MASTER) $(VIEW) $(PLAYER)

# Compilar master_chomp
$(MASTER): $(MASTER_OBJ) $(SHMEMORY_OBJ) $(GAME_LOGIC_OBJ)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(MASTER) $(MASTER_OBJ) $(SHMEMORY_OBJ) $(GAME_LOGIC_OBJ) $(ARG_PARSE_SRC)

# Compilar la vista
$(VIEW): $(VIEW_OBJ)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(VIEW) $(VIEW_OBJ) $(SHMEMORY_OBJ) $(GAME_LOGIC_OBJ)

# Compilar el jugador
$(PLAYER): $(PLAYER_OBJ)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(PLAYER) $(PLAYER_OBJ) $(SHMEMORY_OBJ) $(GAME_LOGIC_OBJ)

# Regla genérica para compilar archivos .c en .o
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(TADS_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Crear el directorio bin si no existe
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Limpiar archivos generados
clean:
	rm -f $(BIN_DIR)/*
	rm -f PVS-Studio.log
	rm -f report.tasks
	rm -f strace_out
	rm -f .viminfo
	rm -f .bash_history

# Ejecutar ChompChamps
run: all
	./$(MASTER) -w 10 -h 10 -t 10 -p $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) -v $(VIEW) -d 100

run_chomp: all
	chmod +x ChompChamps
	./ChompChamps -w 10 -h 10 -t 10 -p $(PLAYER) $(PLAYER) $(PLAYER) -v $(VIEW) -d 100	

# Test the executables
test: all
	@echo "Testing master_chomp, view, and player executables..."
	@RET1=0; RET2=0; RET3=0; RET4=0; RET5=0;
	@sleep 2
	@./$(MASTER) -w 10 -h 10 -t 5 -p $(PLAYER) $(PLAYER) -v $(VIEW) -d 50 || RET1=$$?
	@sleep 3
	@./$(MASTER) -w 15 -h 15 -t 10 -p $(PLAYER) $(PLAYER) $(PLAYER) -v $(VIEW) -d 100 || RET2=$$?
	@sleep 3
	@./$(MASTER) -w 20 -h 20 -t 3 -p $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) -v $(VIEW) -d 200 || RET3=$$?
	@sleep 3
	@./$(MASTER) -w 5 -h 5 -t 2 -p $(PLAYER) $(PLAYER) -v $(VIEW) -d 50 || RET4=$$?
	@sleep 3
	@valgrind --leak-check=full --show-leak-kinds=all ./$(MASTER) -w 25 -h 25 -t 15 -p $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) -v $(VIEW) -d 50 || RET5=$$?
	@sleep 3
	@./$(MASTER) -w 5 -h 5 -t 2 -p $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER) $(PLAYER)  -d 50 
	@./$(MASTER) -w 5 -h 5 -t 2 -d 50 


	@echo "All tests completed."
	@echo "Test Results:"
	@echo "master_chomp test 1 exit code: $$RET1"
	@echo "master_chomp test 2 exit code: $$RET2"
	@echo "master_chomp test 3 exit code: $$RET3";
	@echo "master_chomp test 4 exit code: $$RET4"
	@echo "master_chomp test 5 exit code: $$RET5"	

# Ejecutar el programa con master nuestro
#gcc master_chomp.c -o master_chomp
#./bin/master_chomp -w 25 -h 25 -t 10 -p ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player ./bin/player -v ./bin/view -d 50

#cd pvs-studio
#./install.sh
#make clean
#pvs-studio-analyzer trace -- make
#pvs-studio-analyzer analyze
#plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log
