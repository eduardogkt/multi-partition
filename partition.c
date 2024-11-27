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

// #define DEBUG 1
#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>

#include "src/chrono.h"
#include "src/utils.h"
#include "src/verifica_particoes.h"

typedef struct thread_data {
    int id;           // id da tarefa/thread
    llong *P;         // vetor de elementos de partição
    llong *Input;     // segmento do vetor de entrada da thread
    int P_size;       // tamanho do vetor de elementos de partição
    int Input_size;   // tamanho do segmento do vetor de entrada da thread
    int Input_start;  // indice inicial da fatia do vetor de entrada da thread
    int *Pos;         // vetor de indices de partição local
    int *Part_sizes;  // tamanho de cada partição local
    int *Partitions;  // Partitions[i] indica em que partição Input[i] está
} thread_data_t;

llong InputG[MAX_SIZE];  // vetor global de input
llong OutputG[MAX_SIZE]; // vetor global de saída
llong PG[MAX_SIZE];      // vetor global de posições
int PosG[MAX_SIZE];      // vetor global de indices

pthread_t threads[MAX_THREADS];
thread_data_t thread_data[MAX_THREADS];

pthread_barrier_t barrier_start, barrier_end;

chronometer_t chrono;

void *partitionate(void *arg) {
    while (1) {
        thread_data_t *data = (thread_data_t *) arg;

        pthread_barrier_wait(&barrier_start);

        // zerando tamanhos das partições
        memset(data->Part_sizes, 0, data->P_size * sizeof(int));

        // particiona o vetor Input, indicando em Partitions
        // a qual partição o valor pertence
        for (int i = 0; i < data->Input_size; i++) {
            // verifica em qual partição o valor atual pertence
            for (int j = 0; j < data->P_size; j++) {
                if (data->Input[i] < data->P[j]) {
                    data->Part_sizes[j]++;
                    data->Partitions[i] = j;
                    break;
                }
            }
        }

        // atualizar as posições das partições
        data->Pos[0] = 0;
        for (int i = 1; i < data->P_size; i++) {
            data->Pos[i] = data->Pos[i-1] + data->Part_sizes[i-1];
        }

        pthread_barrier_wait(&barrier_end);
    }
    pthread_exit(NULL);
}

/*void *partitionate(void *arg) {
    thread_data_t *data = (thread_data_t *) arg;

    while (1) {
        pthread_barrier_wait(&barrier_start);

        // zerando tamanhos das partições
        memset(data->Part_sizes, 0, data->P_size * sizeof(int));

        // particionar usando busca binária
        for (int i = 0; i < data->Input_size; i++) {
            int low = 0, high = data->P_size - 1;
            while (low <= high) {
                int mid = low + (high - low) / 2;
                if (data->Input[i] < data->P[mid]) {
                    high = mid - 1;
                } else {
                    low = mid + 1;
                }
            }
            // o índice low é a partição correspondente
            int partition_idx = low;
            data->Part_sizes[partition_idx]++;
            data->Partitions[i] = partition_idx;
        }

        // atualizar posições das partições
        data->Pos[0] = 0;
        for (int i = 1; i < data->P_size; i++) {
            data->Pos[i] = data->Pos[i - 1] + data->Part_sizes[i - 1];
        }

        pthread_barrier_wait(&barrier_end);
    }

    pthread_exit(NULL);
}*/

void set_vectors(thread_data_t *thread_data, llong *Input, llong *P, int P_size, llong *Output) {
    thread_data->Input = &Input[thread_data->Input_start];
    thread_data->P = P;
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
        thread_data[i].P_size = P_size;

        thread_data[i].Pos = malloc(sizeof(int) * P_size);
        thread_data[i].Part_sizes = malloc(sizeof(int) * P_size);
        thread_data[i].Partitions = malloc(sizeof(int) * Input_size);

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

void join_partitions(llong *Output, llong *P, int P_size, int *Pos, int num_threads) {
    int partitions_start[P_size]; // vetor de indices de incio de partição
    int partitions_sizes[P_size]; // vetor tamanhos de cada partição
    memset(partitions_start, 0, P_size * sizeof(int));
    memset(partitions_sizes, 0, P_size * sizeof(int));

    for (int i = 0; i < num_threads; i++) {
        for (int j = 0; j < P_size; j++) {
            partitions_start[j] += thread_data[i].Pos[j];
            partitions_sizes[j] += thread_data[i].Part_sizes[j];
            Pos[j] = partitions_start[j];
        }
    }

    for (int i = 0; i < num_threads; i++) {
        for (int j = 0; j < thread_data[i].Input_size; j++) {
            int partition = thread_data[i].Partitions[j];            
            // printf("%lld ", thread_data[i].Input[j]);

            Output[partitions_start[partition]++] = thread_data[i].Input[j];
        }
    }
    // printf("\n");
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
        // atribui os novos vetores globais a cada iteração
        for (int i = 0; i < num_threads; i++) {
            set_vectors(&thread_data[i], Input, P, P_size, Output);
        }
    }

    // entra na barreira para que as threads iniciem a execução
    pthread_barrier_wait(&barrier_start);

    // espera threads terminarem iteração para retornar o resultado de Pos
    pthread_barrier_wait(&barrier_end);

    // junta os resultados das threads
    join_partitions(Output, P, P_size, Pos, num_threads);
}

int main(int argc, char **argv) {
    srand(time(NULL));
    chrono_reset(&chrono);

    int Input_size, P_size, num_threads;

    if (!checkEntry(argc, argv, &Input_size, &P_size, &num_threads)) {
        exit(EXIT_FAILURE);
    }

    initialize_global_arrays(InputG, Input_size, PG, P_size);

    int Input_start, P_start, *Pos;
    llong *Input, *Output, *P;

    printf("\n-> chamando multi_partition %d vezes\n", N_TESTS);
    
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
        printf("Pos: ");
        print_array_int(Pos, P_size);

        printf("Output: ");
        print_array_llong(Output, Input_size);
        printf("\n");
        #endif

        verifica_particoes(Input, Input_size, P, P_size, Output, Pos);
    }

    llong total_partitioned = (double) Input_size * N_TESTS;
    double total_time_ns = (double) chrono_gettotal(&chrono);
    double total_time_s = total_time_ns / (1000.0 * 1000.0 * 1000.0);
    
    float throughput = (double) (total_partitioned / total_time_s) / 1e6;
    
    printf("Vazao: %.2f\n", throughput);
    printf("Tempo: %lf\n", total_time_s);

    return 0;
}
