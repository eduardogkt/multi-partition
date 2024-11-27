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
    llong *Output;    // vetor de saida
    int P_size;       // tamanho do vetor de elementos de partição
    int Input_size;   // tamanho do segmento do vetor de entrada da thread
    int Input_start;  // indice inicial da fatia do vetor de entrada da thread
    int *Pos;         // vetor de indices de partição local
    int *Part_sizes;  // tamanho de cada partição local
    int *partitions;  // partitions[i] indica em que partição Input[i] está
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
    memset(data->Part_sizes, 0, data->P_size * sizeof(int));
    int out_idx = 0;
    data->Pos[0] = 0;
    for (int i = 0; i < data->P_size; i++) {
        printf("achando partições de P = %lld\n", data->P[i]);
        for (int j = out_idx; j < data->Input_size; j++) {
            if (data->Output[j] < data->P[i]) {
                data->Part_sizes[i]++;
                out_idx++;
            }
            else {
                data->Pos[i+1] = out_idx;
                break; // fim da partição
            }
        }
    }
}

void *partitionate(void *arg) {
    while (1) {
        thread_data_t *data = (thread_data_t *) arg;

        pthread_barrier_wait(&barrier_start);

        memset(data->Part_sizes, 0, data->P_size * sizeof(int));

        // particionar o segmento de Input diretamente em Output
        for (int i = 0; i < data->Input_size; i++) {
            // verifica em qual partição o valor atual pertence
            for (int j = 0; j < data->P_size; j++) {
                if (data->Input[i] < data->P[j]) {
                    data->Part_sizes[j]++;
                    data->partitions[i] = j;
                    break;
                }
            }
        }

        // atualizar as posições das partições
        data->Pos[0] = 0;
        for (int i = 1; i < data->P_size; i++) {
            data->Pos[i] = data->Pos[i-1] + data->Part_sizes[i-1];
        }

        // printf("Input %d: ", data->id);
        // print_array_llong(data->Input, data->Input_size);
        // printf("Parts  %d: ", data->id);
        // print_array_int(data->Part_sizes, data->P_size);
        // printf("Pos    %d: ", data->id);
        // print_array_int(data->Pos, data->P_size);
        // printf("partitions %d: ", data->id);
        // print_array_int(data->partitions, data->Input_size);

        pthread_barrier_wait(&barrier_end);
    }
    pthread_exit(NULL);
}

/*
void *partitionate(void *arg) {
    thread_data_t *data = (thread_data_t *) arg;

    while (1) {
        // Sincronização inicial
        pthread_barrier_wait(&barrier_start);

        // Reinicializar tamanhos das partições
        memset(data->Part_sizes, 0, data->P_size * sizeof(int));

        // Particionar usando busca binária
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
            // O índice 'low' é a partição correspondente
            int partition_idx = low;
            data->Part_sizes[partition_idx]++;
            data->partitions[i] = partition_idx;
        }

        // Atualizar posições das partições
        data->Pos[0] = 0;
        for (int i = 1; i < data->P_size; i++) {
            data->Pos[i] = data->Pos[i - 1] + data->Part_sizes[i - 1];
        }

        // Sincronização final
        pthread_barrier_wait(&barrier_end);
    }

    pthread_exit(NULL);
}
 */

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

        thread_data[i].Output = malloc(sizeof(llong) * Input_size);
        thread_data[i].Pos = malloc(sizeof(int) * P_size);
        thread_data[i].Part_sizes = malloc(sizeof(int) * P_size);
        thread_data[i].partitions = malloc(sizeof(int) * Input_size);

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
    // int out_idx = 0;

    // // passa por cada thread juntando a partição p
    // for (int i = 0; i < P_size; i++) {
    //     printf("\npartição %d [%lld, %lld]:\n", i, i == 0 ? 0 : P[i-1], P[i]);

    //     Pos[i] = out_idx;
    //     for (int j = 0; j < num_threads; j++) {
    //         int start_idx = thread_data[j].Pos[i];
    //         int end_idx = start_idx + thread_data[j].Part_sizes[i];
    //         // printf("de %d até %d\n", start_idx, end_idx);

    //         printf("start:%d, end:%d\n", start_idx, end_idx);

    //         printf("thr%d: ", j);

    //         for (int k = start_idx; k < end_idx; k++) {
    //             printf("%lld ", thread_data[j].Output[k]);
    //             Output[out_idx] = thread_data[j].Output[k];
    //             out_idx++;
    //         }
    //         printf("\n");

    //         // memcpy(&Output[out_idx], &thread_data[j].Output[start_idx], 
    //         //        (end_idx - start_idx) * sizeof(llong));

    //         // out_idx += (end_idx - start_idx);
    //     }
    // }


    // printf("JUNTANDO...\n");

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
    // printf("part inits: ");
    // print_array_int(partitions_start, P_size);

    // printf("part sizes: ");
    // print_array_int(partitions_sizes, P_size);

    // printf("INSERINDO...\n");
    for (int i = 0; i < num_threads; i++) {
        for (int j = 0; j < thread_data[i].Input_size; j++) {
            int partition = thread_data[i].partitions[j];            
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
