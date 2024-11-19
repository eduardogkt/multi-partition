#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

int compar(const void *a, const void *b) {
    llong n1 = *(llong *) a;
    llong n2 = *(llong *) b;
    
    return (n1 - n2);
}

void print_array_llong(llong *array, llong n_memb) {
    for (llong i = 0; i < n_memb; i++) {
        printf("%lld ", array[i]);
    }
    printf("\n");
}

void print_array_int(int *array, llong n_memb) {
    for (llong i = 0; i < n_memb; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
}

// llong *create_array(llong num_memb, int fill, int sort) {
//     llong *array = (llong *) malloc(sizeof(llong) * num_memb);
//     if (array == NULL) {
//         fprintf(stderr, "erro ao alocar memória para o vetor");
//         exit(EXIT_FAILURE);
//     }

//     for (llong i = 0; i < num_memb; i++) {
//         array[i] = fill ? rand() : -1;
//         // array[i] = i;
//     }

//     if (sort) {
//         qsort(array, num_memb, sizeof(llong), compar);
//     }

//     return array;
// }

long long geraAleatorioLL() {
    // returns a pseudo-random integer between 0 and RAND_MAX
    int a = rand();
    int b = rand();
    
    long long v = (long long)a * 100 + b;
    return v;
}

llong *create_array(llong num_memb) {
    llong *array = (llong *) malloc(sizeof(llong) * num_memb);
    if (array == NULL) {
        fprintf(stderr, "erro ao alocar memória para o vetor");
        exit(EXIT_FAILURE);
    }

    for (llong i = 0; i < num_memb; i++) {
        array[i] = -1;
    }

    return array;
}

llong *copy_array(llong *original, llong n_memb) {
    llong *copy = (llong *) malloc(n_memb * sizeof(llong));
    if (copy == NULL) {
        fprintf(stderr, "erro ao alocar memória para a cópia do array.\n");
        exit(EXIT_FAILURE);
    }
    for (llong i = 0; i < n_memb; i++) {
        copy[i] = original[i];
    }
    return copy;
}

void initialize_global_arrays(llong *InputG, llong Input_size, llong* PG, llong P_size) {
    // preenchendo Input com valores aleatórios
    for (llong j = 0; j < Input_size; j++) {
        InputG[j] = rand() % 100; // = geraAleatorioLL();
    }
    qsort(InputG, Input_size, sizeof(llong), compar);

    // fazendo replicas até ecncher InputG ou ser suficiente para o número testes
    for (llong i = 1; (i < MAX_SIZE / Input_size) && (i <= N_TESTS); i++) {
        for (llong j = 0; j < Input_size; j++) {
            InputG[(i * Input_size) + j] = InputG[j];
        }
    }
    // inserir maxlonglong no final

    // faz a mesma coisa para o vetor de pesquisa
    // não precisa dividir em 2 loops porque não precisa ser ordenado
    for (llong i = 0; (i < MAX_SIZE / P_size) && (i <= N_TESTS); i++) {
        for (llong j = 0; j < P_size; j++) {
            if (i == 0) {
                PG[j] = rand() % 100; // = geraAleatorioLL();
            }
            PG[(i * P_size) + j] = PG[j];
        }
    }
}

int checkEntry(int argc, char **argv, llong *P_size, int *num_threads) {

    if (argc < 3) {
        fprintf(stderr, "uso: %s <P_size> <num_threads>\n", argv[0]);
        return 0;
    }

    *P_size = atoi(argv[1]);
    *num_threads = atoi(argv[2]);

    if (*num_threads == 0) {
        fprintf(stderr, "uso: %s <P_size> <num_threads>\n", argv[0]);
        fprintf(stderr, "<num_threads> deve ser maior que 0\n");
        return 0;
    }
    if (*num_threads > MAX_THREADS) {
        fprintf(stderr, "uso: %s <P_size> <num_threads>\n", argv[0]);
        fprintf(stderr, "<num_threads> deve ser menor que %d\n", MAX_THREADS);
        return 0;
    }
    if (*P_size <= 0) {
        fprintf(stderr, "uso: %s <P_size> <num_threads>\n", argv[0]);
        fprintf(stderr, "<P_size> deve ser maior que 0\n");
        return 0;
    }

    return 1;
}
