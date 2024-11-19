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
    int id;            // id da tarefa/thread
    llong *Input;      // vetor de entrada
    llong Input_size;  // tamanho do vetor de entrada
} thread_data_t;

llong InputG[MAX_SIZE];  // vetor global de input
llong OutputG[MAX_SIZE]; // vetor global de saída
llong PG[MAX_SIZE];      // vetor global de posições
llong PosG[MAX_SIZE];    // vetor global de indices

pthread_t threads[MAX_THREADS];
thread_data_t thread_data[MAX_THREADS];

pthread_barrier_t barrier_start, barrier_end;

// void *find_P(void *arg) {

//     while (1) {
//         thread_data_t *data = (thread_data_t *) arg;

//         // esperando ser liberada para pela thread principal
//         // #if DEBUG
//         // printf("\nthread %d esperando...\n", data->id);
//         // #endif

//         pthread_barrier_wait(&barrier_start);

//         // #if DEBUG
//         // printf("\nthread %d liberada\n", data->id);
//         // #endif
        
//         data->pos = my_bsearch(data->Input, data->Input_size, data->q);
        
//         #if DEBUG
//         printf("\nthread %d: q %lld -> local %lld, global %lld\n", 
//                data->id, data->q, data->pos, data->pos + data->Input_start);
//         #endif

//         // adiciona ao inicio do segmento do vetor Input 
//         // para encontar a posição global de q
//         data->pos += data->Input_start;

//         pthread_barrier_wait(&barrier_end);
//     }
//     pthread_exit(NULL);
// }

// void initialize_data(llong *Input, llong Input_size, 
//                      int num_threads, llong *P, llong qpos) {
    
//     int segment_size = Input_size / num_threads;
    
//     #if DEBUG
//     printf("-> inicializando threads\n\n");
//     #endif
    
//     pthread_barrier_init(&barrier_start, NULL, num_threads + 1);
//     pthread_barrier_init(&barrier_end, NULL, num_threads + 1);

//     for (int i = 0; i < num_threads; i++) {
        
//         thread_data[i].id = i;
//         thread_data[i].Input_size = (i == num_threads - 1) ? 
//                                     Input_size - i * segment_size : 
//                                     segment_size;;
//         thread_data[i].Input_start = i * segment_size;
//         thread_data[i].Input = &Input[thread_data[i].Input_start];
//         thread_data[i].q = P[qpos];
//         thread_data[i].pos = -1;

//         #if DEBUG
//         printf("criando thread %d\n", i);
//         llong start = thread_data[i].Input_start;
//         llong end = thread_data[i].Input_start + thread_data[i].Input_size;
//         printf("Input[%lld..%lld]: ", start, end);
//         print_array(&Input[thread_data[i].Input_start], end - start);
//         printf("\n");
//         #endif

//         pthread_create(&threads[i], NULL, find_P, (void *) &thread_data[i]);
//     }
// }

// llong *find_positions(llong *Input, llong Input_size, 
//                       llong* P, llong P_size, 
//                       int num_threads) {
    
//     static int initialized = 0;
//     llong *Pos = create_array(POS_SIZE);
//     llong Pos_partial[num_threads];

//     // encontra cada um dos valores de P
//     for (llong qpos = 0; qpos < P_size; qpos++) {
//         // inicializa a thread dando uma parte do vetor P para cada thread resolver
//         if (!initialized) {
//             initialize_data(Input, Input_size, num_threads, P, qpos);
//             initialized = 1;
//         }
//         else {
//             for (int i = 0; i < num_threads; i++) {
//                 // associando a valor de pesquisa atual
//                 thread_data[i].q = P[qpos];
//             }
//         }

//         // entra na barreira para que as threads iniciem a execução
//         pthread_barrier_wait(&barrier_start);

//         // espera threads terminarem iteração para retornar o resultado de Pos
//         pthread_barrier_wait(&barrier_end);  

//         // cria um novo vetor com os valores das posições dados por cada thread
//         for (int i = 0; i < num_threads; i++) {
//             Pos_partial[i] = Input[thread_data[i].pos];
//         }

//         #if DEBUG
//         printf("Pos_partial: ");
//         print_array(Pos_partial, num_threads);
//         #endif

//         // posição de q no vetor Inuput
//         Pos[qpos] = thread_data[my_bsearch(Pos_partial, num_threads, P[qpos])].pos;
//     }

//     return Pos;
// }

int main(int argc, char **argv) {
    srand(time(NULL));

    chronometer_t chrono;
    llong Input_size;
    llong P_size, *Output = NULL;
    int num_threads;

    if (!checkEntry(argc, argv, &Input_size, &P_size, &num_threads)) {
        exit(EXIT_FAILURE);
    }

    initialize_global_arrays(InputG, Input_size, PG, P_size);

    chrono_reset(&chrono);
    printf("\n-> chamando multi_partition %d vezes\n", N_TESTS);

    for (int i = 0; i < N_TESTS; i++) {

        llong Input_start = i * Input_size;
        llong P_start = i * P_size;
        
        if ((Input_size + Input_start) > MAX_SIZE) {
            Input_start = 0;
        }
        if ((P_size + P_start) > MAX_SIZE) {
            P_start = 0;
        }
        
        llong *Input = &InputG[Input_start];
        llong *P = &PG[P_start];

        #if DEBUG
        printf("Input_start: %lld\n", Input_start);
        printf("P_start: %lld\n", P_start);

        printf("num threads: %d\n", num_threads);
        printf("Input (%lld): ", Input_size);
        print_array_llong(Input, Input_size);
        printf("P (%lld): ", P_size);
        print_array_llong(P, P_size);
        printf("\n");
        #endif

        chrono_start(&chrono);
        int Pos[5] = {1, 2, 3, 4, 5};
        chrono_stop(&chrono);

        #if DEBUG
        printf("\nfinal Pos: ");
        print_array_int(Pos, P_size);
        printf("\n\n");
        #endif

        // free(Pos);
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
