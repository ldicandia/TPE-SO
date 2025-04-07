# TPE Sistemas Operativos: ChompChamps


## Integrantes

- Alfieri, Agostino

- Diaz Varela, Lola

- Di Candia, Lucas

## Introducción

Este proyecto implementa un juego llamado **ChompChamps**, que utiliza memoria compartida y semáforos para la sincronización entre procesos. El juego incluye un **master** que controla la lógica del juego, varios **players** que realizan movimientos, y una **view** que muestra el estado del tablero en tiempo real.

## Cómo Funciona

1. **Master**:
   - Inicializa el tablero y los jugadores.
   - Crea memoria compartida para el estado del juego y los semáforos.
   - Lanza procesos para los jugadores y la vista.
   - Coordina los movimientos de los jugadores y actualiza el estado del juego.

2. **Player**:
   - Lee el estado del juego desde la memoria compartida.
   - Realiza movimientos aleatorios en el tablero.
   - Escribe su movimiento en un pipe para que el master lo procese.

3. **View**:
   - Lee el estado del juego desde la memoria compartida.
   - Muestra el tablero y los puntajes en tiempo real.
   - Finaliza cuando el juego termina.

4. **Sincronización**:
   - Se utilizan semáforos para coordinar la escritura y lectura del estado del juego entre los procesos.

## Requisitos

- **Sistema Operativo**: Linux.
- **Compilador**: `gcc`.
- **Herramientas**: `valgrind` (opcional para pruebas de memoria).

## Compilación

Para compilar el proyecto, ejecuta:

```bash
make run
```

Para testear con diferentes parametros:

```bash
make test
```
