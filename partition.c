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
    int *Inits;       // indice de inicio de partições
} thread_data_t;

llong InputG[MAX_SIZE];  // vetor global de input
llong OutputG[MAX_SIZE]; // vetor global de saída
llong PG[MAX_SIZE];      // vetor global de posições
int PosG[MAX_SIZE];      // vetor global de indices

pthread_t threads[MAX_THREADS];
thread_data_t thread_data[MAX_THREADS];

pthread_barrier_t barrier_start, barrier_end;

chronometer_t chrono;

// encontra o tamanho de cada partição e armazena em Part_sizes
void find_partition_sizes(thread_data_t *data) {
    memset(data->Part_sizes, 0, sizeof(int) * data->P_size);
    for (int i = 0; i < data->Input_size; i++) {
        for (int j = 0; j < data->P_size; j++) {
            if (data->Output[i] < data->P[j]) {
                data->Part_sizes[j]++;
                break;
            }
        }
    }
}

// encontra o índice de início de cada partição e armazena em Inits
void find_partition_starts(thread_data_t *data) {
    data->Inits[0] = 0;
    for (int i = 1; i < data->P_size; i++) {
        data->Inits[i] = data->Inits[i-1] + data->Part_sizes[i-1];
    }
}

void *partitionate(void *arg) {

    while (1) {
        thread_data_t *data = (thread_data_t *) arg;

        pthread_barrier_wait(&barrier_start);

        // cria uma copia ordenada do vetor de entrada
        memcpy(data->Output, data->Input, sizeof(llong) * data->Input_size);
        qsort(data->Output, data->Input_size, sizeof(llong), compar);

        find_partition_sizes(data);
        find_partition_starts(data);

        // printf("Output %d: ", data->id);
        // print_array_llong(data->Output, data->Input_size);

        // printf("Parts %d: ", data->id);
        // print_array_int(data->Part_sizes, data->P_size);

        // printf("Inits %d: ", data->id);
        // print_array_int(data->Inits, data->P_size);

        pthread_barrier_wait(&barrier_end);
    }
    pthread_exit(NULL);
}

void set_vectors(thread_data_t *thread_data, llong *Input, llong *P, int P_size, llong *Output) {
    thread_data->Input = &Input[thread_data->Input_start];
    thread_data->Output = malloc(sizeof(llong) * thread_data->Input_size);
    thread_data->P = P;
    thread_data->Pos = malloc(sizeof(int) * P_size);
    thread_data->Part_sizes = malloc(sizeof(int) * P_size);
    thread_data->Inits = malloc(sizeof(int) * P_size);
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

void print_output_parts(int num_threads, thread_data_t *data) {
    #if DEBUG
    for (int i = 0; i < num_threads; i++) {
        printf("Part_sizes %d: ", thread_data[i].id);
        print_array_int(thread_data[i].Part_sizes, thread_data[i].P_size);

        printf("Output %d: ", thread_data[i].id);
        print_array_llong(thread_data[i].Output, thread_data[i].Input_size);
        printf("\n");
    }
    #endif
}

void join_partitions(llong *Output, llong *P, int P_size, int *Pos, int num_threads) {
    // printf("JUNTANDO...\n");
    // printf("Output: ");
    // print_array_llong(Output, 14);
    int out_idx = 0;
    for (int i = 0; i < P_size; i++) {
        // if (i == 0) {
        //     printf("\npartição %d [0, %lld]:\n", i, P[i]);
        // }
        // else {
        //     printf("\npartição %d [%lld, %lld]:\n", i, P[i-1], P[i]);
        // }
        Pos[i] = out_idx;
        for (int j = 0; j < num_threads; j++) {
            // printf("\n%d: init:%d tam:%d\n", thread_data[j].id, thread_data[j].Inits[i], thread_data[j].Part_sizes[i]);
            
            for (int k = thread_data[j].Inits[i]; k < thread_data[j].Inits[i] + thread_data[j].Part_sizes[i]; k++) {
                // printf("%lld ", thread_data[j].Output[k]);
                Output[out_idx] = thread_data[j].Output[k];
                out_idx++;
            }
        }
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

    // print_output_parts(num_threads, thread_data);

    // junta os resultados das threads
    join_partitions(Output, P, P_size, Pos, num_threads);
    
    for (int i = 0; i < num_threads; i++) {
        free(thread_data[i].Pos);
        free(thread_data[i].Part_sizes);
        free(thread_data[i].Inits);
    }
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

    llong total_partitioned = (double) P_size * N_TESTS;
    double total_time_ns = (double) chrono_gettotal(&chrono);
    double total_time_s = total_time_ns / (1000.0 * 1000.0 * 1000.0);
    
    float throughput = (double) (total_partitioned / total_time_s) / 1e6;
    
    printf("Vazao: %.2f\n", throughput);
    printf("Tempo: %lf\n", total_time_s);

    return 0;
}
