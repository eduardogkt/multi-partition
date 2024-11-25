#define DEBUG 1
/*
Eduardo Gabriel Kenzo Tanaka GRR20211791

void multi_partition(long long *Input, int n, long long *P, int np, long long *Output, int *Pos); 

Input: array de entrada com os valores aleatórios desordenados
n: tamanho do array de entrada Input
P: vetor de valores que indicam o início de cada faixa de partição
np: tamanho do vetor P
Output: vetor com valores de entrada ordenados para verificação
Pos: vetor de indices que indicam o início de cada faixa de partição

n = 16000000 por default
*/

#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <limits.h>

#include "src/chrono.h"
#include "src/utils.h"
#include "src/verifica_particoes.h"

typedef struct thread_data {
    int id;           // id da tarefa/thread
    llong *P;         // vetor de elementos de partição
    llong *Input;     // segmento do vetor de entrada da thread
    llong *Output;    // vetor de saida
    int P_size;       // tamanho do vetor de elementos de partição
    int Input_size;   // tamanho do segmento do vetor de entrada da thread
    int Input_start;  // indice inicial da fatia do vetor de entrada da thread
    int Input_end;    // indice final da fatia do vetor de entrada da thread
    int *Pos;         // vetor de indices de partição local
    int *Part_sizes;  // tamanho de cada partição local
} thread_data_t;

llong InputG[MAX_SIZE];  // vetor global de input
llong OutputG[MAX_SIZE]; // vetor global de saída
llong PG[MAX_SIZE];      // vetor global de posições
int PosG[MAX_SIZE];      // vetor global de indices

pthread_t threads[MAX_THREADS];
thread_data_t thread_data[MAX_THREADS];

pthread_barrier_t barrier_start, barrier_end;

chronometer_t chrono;

// retorna o indice de inicio da partição do vetor
int partition(llong *Input, int Input_size, llong P) {
    int start = 0;
    printf("\n\ncomenço %d", start);
    printf("\nthread procurando %lld -> ", P);
    
    for (int i = start; i < Input_size; i++) {
        if (Input[i] >= P) {
            printf("%d", i);
            return i;
        }
    }
    printf("não encontrou retornando %d", Input_size - 1);
    return Input_size - 1;
}

void *partitionate(void *arg) {

    while (1) {
        thread_data_t *data = (thread_data_t *) arg;

        // esperando ser liberada para pela thread principal
        // #if DEBUG
        // printf("\nthread %d esperando...\n", data->id);
        // #endif

        pthread_barrier_wait(&barrier_start);

        printf("\nP: ");
        print_array_llong(data->P, data->P_size);

        // cria uma copia ordenada do vetor de entrada
        memcpy(data->Output, data->Input, sizeof(llong) * data->Input_size);
        qsort(data->Output, data->Input_size, sizeof(llong), compar);
        printf("\nOutput %d: ", data->id);
        print_array_llong(data->Output, data->Input_size);

        // #if DEBUG
        // printf("\nthread %d liberada\n", data->id);
        // #endif
        
        // acha as partições
        for (int i = 0; i < data->Input_size; i++) {
            for (int j = 0; j < data->P_size; j++) {
                if (data->Output[i] < data->P[j]) {
                    data->Part_sizes[j]++;
                    break;
                }
            }
        }
        printf("Part_sizes %d: ", data->id);
        print_array_int(data->Part_sizes, data->P_size);
        
        // #if DEBUG
        // printf("\nthread %d: ", data->id);
        // print_array_int(data->Pos, P_size);
        // #endif

        pthread_barrier_wait(&barrier_end);
    }
    pthread_exit(NULL);
}

void set_vectors(thread_data_t *thread_data, llong *Input, llong *P, int P_size, llong *Output) {
    thread_data->Input = &Input[thread_data->Input_start];
    thread_data->Output = &Output[thread_data->Input_start];
    thread_data->P = P;
    thread_data->Pos = malloc(sizeof(int) * P_size);
    thread_data->Part_sizes = malloc(sizeof(int) * P_size);
}

void initialize_data(llong *Input, int Input_size, 
                     llong *P, int P_size, 
                     int *Pos, int num_threads,
                     llong *Output) {

    int segment_size = Input_size / num_threads;

    #if DEBUG
    printf("\n-> inicializando threads\n\n");
    #endif
    
    pthread_barrier_init(&barrier_start, NULL, num_threads + 1);
    pthread_barrier_init(&barrier_end, NULL, num_threads + 1);

    for (int i = 0; i < num_threads; i++) {
        
        thread_data[i].id = i;
        thread_data[i].Input_size = (i == num_threads - 1) ? 
                                    Input_size - i * segment_size : 
                                    segment_size;
        thread_data[i].Input_start = i * segment_size;
        thread_data[i].Input_end = (i == num_threads - 1) ? 
                                   Input_size : 
                                   (i + 1) * segment_size;
        thread_data[i].P_size = P_size;
        set_vectors(&thread_data[i], Input, P, P_size, Output);

        #if DEBUG
        printf("criando thread %d\n", i);
        llong start = thread_data[i].Input_start;
        llong end = thread_data[i].Input_start + thread_data[i].Input_size;
        printf("Input[%lld..%lld]: ", start, end);
        print_array_llong(thread_data[i].Input, thread_data[i].Input_size);
        printf("\n");
        #endif

        pthread_create(&threads[i], NULL, partitionate, (void *) &thread_data[i]);
    }
}

void multi_partition(llong *Input, int Input_size, 
                     llong *P, int P_size, 
                     llong *Output, int *Pos, 
                     int num_threads) {

    static int initialized = 0;

    // inicializa a thread dando uma parte do vetor P para cada thread resolver
    if (!initialized) {
        initialize_data(Input, Input_size, P, P_size, Pos, num_threads, Output);
        initialized = 1;
    }
    else {
        // atribui os novos vetores globais a cada iteração para evitar
        // efeito de cache
        for (int i = 0; i < num_threads; i++) {
            set_vectors(&thread_data[i], Input, P, P_size, Output);
        }
    }

    // entra na barreira para que as threads iniciem a execução
    pthread_barrier_wait(&barrier_start);

    // espera threads terminarem iteração para retornar o resultado de Pos
    pthread_barrier_wait(&barrier_end);

    // passa por cada thread juntando os valores nos vetores finais
    // provavelmente um loop duplo

    for (int i = 0; i < num_threads; i++) {
        free(thread_data[i].Pos);
        free(thread_data[i].Part_sizes);
    }
}

int main(int argc, char **argv) {
    srand(time(NULL));

    int Input_size, P_size, num_threads;

    if (!checkEntry(argc, argv, &Input_size, &P_size, &num_threads)) {
        exit(EXIT_FAILURE);
    }

    initialize_global_arrays(InputG, Input_size, PG, P_size);

    chrono_reset(&chrono);
    printf("\n-> chamando multi_partition %d vezes\n", N_TESTS);

    int Input_start, P_start;
    llong *Input, *Output, *P;
    int *Pos;
    
    for (int i = 0; i < N_TESTS; i++) {

        Input_start = i * Input_size;
        P_start = i * P_size;
        
        if ((Input_size + Input_start) > MAX_SIZE) {
            Input_start = 0;
        }
        if ((P_size + P_start) > MAX_SIZE) {
            P_start = 0;
        }
        
        Input = &InputG[Input_start];
        Output = &OutputG[Input_start];
        P = &PG[P_start];
        Pos = &PosG[P_start];

        #if DEBUG
        printf("Input_start: %d\n", Input_start);
        printf("P_start: %d\n", P_start);

        printf("num threads: %d\n", num_threads);
        printf("Input (%d): ", Input_size);
        print_array_llong(Input, Input_size);
        printf("P (%d): ", P_size);
        print_array_llong(P, P_size);
        printf("\n");
        #endif

        chrono_start(&chrono);
        multi_partition(Input, Input_size, P, P_size, Output, Pos, num_threads);
        chrono_stop(&chrono);

        #if DEBUG
        printf("\nPos: ");
        print_array_int(Pos, P_size);

        printf("\nOutput: ");
        print_array_llong(Output, Input_size);
        printf("\n\n");
        #endif

        // verifica_particoes(Input, Input_size, P, P_size, Output, Pos);
    }

    llong total_partitioned = (double) P_size * N_TESTS;
    double total_time_ns = (double) chrono_gettotal(&chrono);
    double total_time_s = total_time_ns / (1000.0 * 1000.0 * 1000.0);
    
    float throughput = (double) (total_partitioned / total_time_s) / 1e6;
    
    printf("Vazao: %.2f\n", throughput);
    printf("Tempo: %lf\n", total_time_s);

    return 0;
}
