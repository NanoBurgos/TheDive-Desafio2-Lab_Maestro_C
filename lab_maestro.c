#include <stdio.h> // entrada/salida: printf, scanf, putchar, perror
#include <stdlib.h> // utilidades: malloc, free, rand, atoi, srand, exit
#include <time.h> // funciones de tiempo: time, clock, CLOCKS_PER_SEC

// Portabilidad Linux / Windows
#ifdef _WIN32
#include <windows.h> // incluye Sleep() en Windows
#else
#include <unistd.h> // incluye usleep() en Unix-like
#endif

#define PAREDES   '#' // símbolo para paredes
#define CAMINOS   ' ' // símbolo para caminos (espacio)
#define SOLUCION  '*' // símbolo para marcar la solución final

int filas, columnas; // dimensiones del laberinto 
int tablero_filas, tablero_columnas; // dimensiones de la malla real (filas*2+1, columnas*2+1)
char *laberinto; // puntero al arreglo 1D que contiene la malla 

// Macro para convertir coordenadas 2D (f,c) a índice 1D en 'laberinto'
#define IDX(f, c) ((f) * tablero_columnas + (c))

// Limpieza de pantalla 
void clear_screen() { // función para limpiar la consola
#ifdef _WIN32
    system("cls"); // en Windows ejecuta el comando cls
#else
    system("clear"); // en Unix ejecuta el comando clear
#endif
}

// Pausa en milisegundos (portátil)
void sleep_ms(int ms) { // duerme ms milisegundos
#ifdef _WIN32
    Sleep(ms); // Sleep usa milisegundos en Windows
#else
    usleep(ms * 1000); // usleep usa microsegundos en Unix
#endif
}

// Prototipos de funciones
void mostrar_laberinto(int p_fila, int p_columna); // imprime laberinto sin marcar ruta del jugador
void mostrar_jugador_laberinto(int p_fila, int p_columna, int pasos_recorridos); // imprime laberinto mostrando jugador y  '.'
void crear_laberinto(int f, int c); // genera laberinto mediante DFS recursivo
int resolver_laberinto_bfs(); // resuelve laberinto con BFS y marca solución
void juego_usuario(); // bucle interactivo para que el usuario juegue

// Mostrar el laberinto (vista general)
void mostrar_laberinto(int p_fila, int p_columna) { // muestra el mapa actual
    clear_screen(); // limpia la pantalla antes de dibujar
    for (int f = 0; f < tablero_filas; f++) { // recorre cada fila de la malla real
        for (int c = 0; c < tablero_columnas; c++) { // recorre cada columna de la malla real
            if (f == 1 && c == 0)
                putchar('S'); // entrada ( S = start)
            else if (f == tablero_filas - 2 && c == tablero_columnas - 1)
                putchar('E'); // salida (E = end)
            else if (f == p_fila && c == p_columna)
                putchar('P'); // posición del jugador si coincide
            else
                putchar(laberinto[IDX(f, c)]); // muestra el contenido almacenado en laberinto
        }
        putchar('\n'); // nueva línea al terminar la fila
    }
}

// Mostrar laberinto mientras el jugador se mueve
void mostrar_jugador_laberinto(int p_fila, int p_columna, int pasos_recorridos) {
    clear_screen(); // limpia consola
    for (int f = 0; f < tablero_filas; f++) { // itera filas
        for (int c = 0; c < tablero_columnas; c++) { // itera columnas
            if (f == 1 && c == 0)
                putchar('S'); // entrada 
            else if (f == tablero_filas - 2 && c == tablero_columnas - 1)
                putchar('E'); // salida 
            else if (f == p_fila && c == p_columna)
                putchar('P'); // jugador
            else if (pasos_recorridos && laberinto[IDX(f,c)] == '.')
                putchar('.'); // muestra los pasos recorridos si se pidió
            else
                putchar(laberinto[IDX(f,c)]); // caso por defecto: muestra el contenido
        }
        putchar('\n'); // nueva línea
    }
}

// Inicializar el laberinto lleno de muros
void laberinto_inicial() {
    laberinto = malloc(tablero_filas * tablero_columnas); // reservar memoria para la malla/grilla
    if (!laberinto) { // comprobar que malloc no devolvió NULL
        perror("malloc"); // imprime error si falla la reserva
        exit(EXIT_FAILURE); // termina el programa con error
    }
    for (int f = 0; f < tablero_filas; f++) // recorre filas
        for (int c = 0; c < tablero_columnas; c++) // recorre columnas
            laberinto[IDX(f, c)] = PAREDES; // inicializa cada posición como pared
}

// DFS recursivo para generar el laberinto
void crear_laberinto(int f, int c) {
    int direcciones[4][2] = {{0,2},{0,-2},{2,0},{-2,0}}; // movimientos en pasos de 2 (para tallar pasajes)
    for (int i = 0; i < 4; i++) { // mezcla aleatoria de direcciones 
        int j = rand() % 4; // índice aleatorio
        int tmp0 = direcciones[i][0], tmp1 = direcciones[i][1]; // guarda temporal
        direcciones[i][0] = direcciones[j][0]; direcciones[i][1] = direcciones[j][1]; // swap
        direcciones[j][0] = tmp0; direcciones[j][1] = tmp1; // swap
    }

    for (int i = 0; i < 4; i++) { // intenta cada dirección 
        int nf = f + direcciones[i][0]; // fila candidata (dos pasos)
        int nc = c + direcciones[i][1]; // columna candidata (dos pasos)

        if (nf > 0 && nf < tablero_filas-1 && nc > 0 && nc < tablero_columnas-1
            && laberinto[IDX(nf,nc)] == PAREDES) { // si la celda está dentro y es muro (no visitada)
            laberinto[IDX(f + direcciones[i][0]/2, c + direcciones[i][1]/2)] = CAMINOS; // abre la pared intermedia
            laberinto[IDX(nf,nc)] = CAMINOS; // abre la celda destino
            mostrar_laberinto(-1, -1); // muestra animación de creación
            sleep_ms(50); // pausa para ver la animación
            crear_laberinto(nf,nc); // llamada recursiva desde la nueva celda
        }
    }
}

// Estructura para nodos (fila, columna)
typedef struct { int f, c; } Node; // representa una celda en la malla

// BFS para resolver el laberinto automáticamente
int resolver_laberinto_bfs() {
    int visitados[tablero_filas][tablero_columnas]; // matriz de visitados (en pila)
    Node padre[tablero_filas][tablero_columnas]; // matriz de padres para reconstruir ruta

    for (int f = 0; f < tablero_filas; f++) // inicializa matrices
        for (int c = 0; c < tablero_columnas; c++) {
            visitados[f][c] = 0; // no visitado
            padre[f][c] = (Node){-1,-1}; // padre vacío
        }

    Node queue[tablero_filas * tablero_columnas]; // cola con capacidad máxima
    int front = 0, back = 0; // índices de la cola

    Node start = {1,0}; // nodo de inicio (entrada)
    Node end   = {tablero_filas-2, tablero_columnas-1}; // nodo destino (salida)

    queue[back++] = start; // encola inicio
    visitados[start.f][start.c] = 1; // marca inicio como visitado

    int direcciones[4][2] = {{1,0},{-1,0},{0,1},{0,-1}}; // movimientos de 1 paso (4 vecinos)

    while (front < back) { // mientras la cola no esté vacía
        Node cur = queue[front++]; // desencola el siguiente nodo
        if (cur.f == end.f && cur.c == end.c) break; // si llegó al destino, salir del bucle

        for (int i = 0; i < 4; i++) { // examina los 4 vecinos
            int nf = cur.f + direcciones[i][0]; // fila vecina
            int nc = cur.c + direcciones[i][1]; // columna vecina
            if (nf >= 0 && nf < tablero_filas && nc >= 0 && nc < tablero_columnas
                && !visitados[nf][nc] && laberinto[IDX(nf,nc)] == CAMINOS) { // válido y no visitado
                visitados[nf][nc] = 1; // marca visitado
                padre[nf][nc] = cur; // guarda el padre para reconstruir
                queue[back++] = (Node){nf,nc}; // encola vecino
                laberinto[IDX(nf,nc)] = '.'; // marca visualmente como visitado
                mostrar_laberinto(-1,-1); // animación de la búsqueda
                sleep_ms(20); // pausa para ver la animación
            }
        }
    }

    // Antes de reconstruir, comprobar si el destino fue alcanzado
    if (!visitados[end.f][end.c]) { // si no se visitó la celda final
        fprintf(stderr, "No se encontro camino desde inicio a fin.\n"); // mensaje de error
        return 0; // indicar fallo
    }

    Node cur = end; // empezar en la salida para reconstruir el camino
    while (!(cur.f == start.f && cur.c == start.c)) { // hasta llegar al inicio
        if (!(cur.f == end.f && cur.c == end.c)) // no pintar la celda de la salida
            laberinto[IDX(cur.f,cur.c)] = SOLUCION; // marca la solución con '*'
        cur = padre[cur.f][cur.c]; // avanza al padre
        mostrar_laberinto(-1,-1); // muestra animación de reconstrucción
        sleep_ms(50); // pausa para animación
    }

    return 1; // éxito
}

// Juego interactivo animado para el usuario
void juego_usuario() {
    int p_fila = 1, p_columna = 0; // posición inicial del jugador (entrada)
    time_t tiempo_inicio = time(NULL); // tiempo de inicio en segundos

    mostrar_jugador_laberinto(p_fila, p_columna, 0); // dibuja sin pasos_recorridos

    while (!(p_fila == tablero_filas-2 && p_columna == tablero_columnas-1)) { // mientras no llegue a la salida
        char eleccion_usuario; // variable para la tecla leída
        scanf(" %c", &eleccion_usuario); // lee un caracter (requiere Enter)
        int nf = p_fila, nc = p_columna; // posición tentativa
        if(eleccion_usuario == 'w') nf--; // mover arriba
        else if(eleccion_usuario == 's') nf++; // mover abajo
        else if(eleccion_usuario == 'a') nc--; // mover izquierda
        else if(eleccion_usuario == 'd') nc++; // mover derecha

        if(nf >= 0 && nf < tablero_filas && nc >=0 && nc < tablero_columnas
           && laberinto[IDX(nf,nc)] != PAREDES) { // si la celda destino no es pared
            laberinto[IDX(p_fila,p_columna)] = '.'; // marca la celda previa con '.'
            p_fila = nf; p_columna = nc; // actualiza posición del jugador
        }
        mostrar_jugador_laberinto(p_fila, p_columna, 1); // muestra la nueva posición y los pasos
        sleep_ms(50); // pequeña pausa para animación
    }

    time_t tiempo_fin = time(NULL); // tiempo de finalización
    printf("¡Felicidades! Encontraste la salida en %ld segundos.\n", tiempo_fin - tiempo_inicio); // muestra tiempo transcurrido
}

// Programa principal
int main(int argumentos_c, char *argumentos_usuarios[]) {
    if (argumentos_c < 3) { // si no se pasaron dimensiones por línea de comandos
        printf("No se indicaron dimensiones, usando 10x10 por defecto.\n"); // aviso
        filas = 10; // valor por defecto
        columnas = 10; // valor por defecto
    } else {
        filas = atoi(argumentos_usuarios[1]); // convierte argumento 1 a entero
        columnas = atoi(argumentos_usuarios[2]); // convierte argumento 2 a entero
    }

    if (filas < 2 || columnas < 2) { // valida tamaño mínimo
        printf("Error: dimensiones minimas 2x2.\n"); // mensaje de error
        return 1; // termina con error
    }

    tablero_filas = filas * 2 + 1; // calcula filas reales de la malla
    tablero_columnas = columnas * 2 + 1; // calcula columnas reales de la malla

    srand(time(NULL)); // inicializa la semilla para rand()
    laberinto_inicial(); // reserva memoria e inicializa a PAREDES

    laberinto[IDX(1,0)] = CAMINOS; // abre la entrada en la malla
    laberinto[IDX(tablero_filas-2, tablero_columnas-1)] = CAMINOS; // abre la salida
    laberinto[IDX(1,1)] = CAMINOS; // abre la celda inicial desde donde se empieza a tallar

    // medir tiempo de creación del laberinto
    clock_t inicio_generacion = clock(); // tiempo CPU inicio
    crear_laberinto(1,1); // genera el laberinto empezando en (1,1)
    clock_t final_generacion = clock(); // tiempo CPU fin
    double tiempo_transc_gen_laberinto = (double)(final_generacion - inicio_generacion) / CLOCKS_PER_SEC; // calcula segundos
    printf("\nEl laberinto se genero en %.3f segundos.\n", tiempo_transc_gen_laberinto); // imprime tiempo

    int eleccion; // opción del usuario: jugar o ver resolución
    printf("Elige una opcion:\n1. Jugar\n2. Ver animacion de resolucion\nOpcion: "); // pide opción
    scanf("%d", &eleccion); // lee la opción

    if(eleccion == 1) { // si elige jugar
        juego_usuario(); // lanza el modo interactivo
    } else { // si elige ver animación de resolución
        clock_t inicio_resolucion = clock(); // tiempo inicio resolución
        resolver_laberinto_bfs(); // resuelve con BFS y muestra animación
        clock_t fin_resolucion = clock(); // tiempo fin resolución
        double tiempo_transc_resolv_laberinto = (double)(fin_resolucion - inicio_resolucion) / CLOCKS_PER_SEC; // segundos
        mostrar_laberinto(-1,-1); // muestra resultado final
        printf("\nEl laberinto se resolvio en %.3f segundos.\n", tiempo_transc_resolv_laberinto); // imprime tiempo
    }

    free(laberinto); // libera memoria reservada para el laberinto
    return 0; // fin del programa exitoso
}
