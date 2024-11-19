#ifndef __UTILS_H__
#define __UTILS_H__

#define llong long long

// numero maximo de threads
#define MAX_THREADS 64

// quantidade de elemento long long que cabem em uma cache 2 GB de RAM
#define MAX_SIZE (100 * 1000 * 1000)

// valores de entrada para testes
#define SIZE_100K 100000
#define SIZE_1M 1000000
#define SIZE_2M 2000000
#define SIZE_4M 4000000
#define SIZE_8M 8000000
#define SIZE_16M 16000000

#define INPUT_SIZE SIZE_1M
#define OUTPUT_SIZE INPUT_SIZE
#define P_SIZE SIZE_100K
#define POS_SIZE P_SIZE

// quantidade de testes realizados
#define N_TESTS 1

// imprime o array de elementos tipo llong
void print_array_llong(llong *array, llong n_memb);

// imprime o array de elementos tipo int
void print_array_int(int *array, llong n_memb);

// cria uma array de elementos tipo llong
// o vetor pode ser preenchido com valores aleatórios e ordenado
llong *create_array(llong num_memb);

// faz uma cópia do vetor original
llong *copy_array(llong *original, llong n_memb);

// inicializa os vetores globais com sequencias de valores aleatorios
void initialize_global_arrays(llong *InputG, llong Input_size, llong* QG, llong Q_size);

// checa os parametros de entrada para garantir que estão no formato correto
int checkEntry(int argc, char **argv, llong *Input_size, llong *P_size, int *num_threads);

#endif
