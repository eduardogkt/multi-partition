#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int compar(const void *a, const void *b) {
    return (*(llong *) a - *(llong *) b);
}

void print_array_llong(llong *array, llong n_memb) {
    if (array == NULL) {
        printf("<NULL>\n");
        return;
    }

    for (llong i = 0; i < n_memb; i++) {
        printf("%lld ", array[i]);
    }
    printf("\n");
}

void print_array_int(int *array, llong n_memb) {
    if (array == NULL) {
        return;
    }
    
    for (llong i = 0; i < n_memb; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
}

long long geraAleatorioLL() {
    // returns a pseudo-random integer between 0 and RAND_MAX
    int a = rand();
    int b = rand();
    
    long long v = (long long)a * 100 + b;
    return v;
}

// #define TEST_EXEMPLO 1
#define TEST_ALEAT 1

#if TEST_EXEMPLO
int in[100] = {95, 63, 82, 73, 14, 46, 52, 36, 31, 43, 73, 45, 73, 62}; // {8, 4, 7, 11, 3, 7, 7, 13, 44, 46, 44, 100, 100, 110 };
llong p[100] = {19, 22, 56, LLONG_MAX}; // {12, 70, 90, LLONG_MAX}; 
#endif

void initialize_global_arrays(llong *InputG, int Input_size, llong *PG, int P_size) {
    // fazendo replicas até encher InputG ou ser suficiente para o número testes
    for (llong i = 0; (i < MAX_SIZE / Input_size) && (i <= N_TESTS); i++) {
        for (llong j = 0; j < Input_size; j++) {
            if (i == 0) {
                #if TEST_EXEMPLO
                InputG[j] = in[j];
                #elif TEST_ALEAT
                InputG[j] = geraAleatorioLL();
                #else
                InputG[j] = rand() % 100;
                #endif
            }
            else {
                InputG[(i * Input_size) + j] = InputG[j];
            }
        }
    }

    // a primeira iteração é isolada porque os valores precisam ser ordenados
    for (llong i = 0; i < P_size; i++) {
        #if TEST_EXEMPLO
        PG[i] = p[i];
        #elif TEST_ALEAT
        PG[i] = geraAleatorioLL();
        #else
        PG[i] = rand() % 100;
        #endif
    }
    PG[P_size - 1] = LLONG_MAX;
    #ifndef TEST_EXEMPLO
    qsort(PG, P_size - 1, sizeof(llong), compar);
    #endif
    // fazendo replicas até encher PG ou ser suficiente para o número de testes 
    for (llong i = 1; (i < MAX_SIZE / P_size) && (i <= N_TESTS); i++) {
        for (llong j = 0; j < P_size; j++) {
            PG[(i * P_size) + j] = PG[j];
        }
    }
}

int checkEntry(int argc, char **argv, 
               int *Input_size, int *P_size, 
               int *num_threads) {

    if (argc < 4) {
        fprintf(stderr, "uso: %s <Input_size> <P_size> <num_threads>\n", argv[0]);
        return 0;
    }

    *Input_size = atoi(argv[1]);
    *P_size = atoi(argv[2]);
    *num_threads = atoi(argv[3]);

    if (*Input_size <= 0) {
        fprintf(stderr, "uso: %s <Input_size> <P_size> <num_threads>\n", argv[0]);
        fprintf(stderr, "<Input_size> deve ser maior que 0\n");
        return 0;
    }
    if (*Input_size > MAX_SIZE) {
        fprintf(stderr, "uso: %s <Input_size> <num_threads> <opt:Q_size>\n", argv[0]);
        fprintf(stderr, "<Input_size> deve ser no máximo %d\n", MAX_SIZE);
        return 0;
    }

    if (*num_threads <= 0) {
        fprintf(stderr, "uso: %s <Input_size> <P_size> <num_threads>\n", argv[0]);
        fprintf(stderr, "<num_threads> deve ser maior que 0\n");
        return 0;
    }
    if (*num_threads > MAX_THREADS) {
        fprintf(stderr, "uso: %s <Input_size> <P_size> <num_threads>\n", argv[0]);
        fprintf(stderr, "<num_threads> deve ser menor que %d\n", MAX_THREADS);
        return 0;
    }

    if (*P_size <= 0) {
        fprintf(stderr, "uso: %s <Input_size> <P_size> <num_threads>\n", argv[0]);
        fprintf(stderr, "<P_size> deve ser maior que 0\n");
        return 0;
    }

    return 1;
}
