#include "verifica_particoes.h"

#include <stdio.h>

// verifica se as partições criados por multi_partition estão corretas
void verifica_particoes(llong* Input, int Input_size, 
                        llong *P, int P_size, 
                        llong *Output, int *Pos) {

    printf("VERIFICAÇÃO DE PARTIÇÕES\n");

    for (int pos = 0; pos < P_size; pos++) {
        int part_start_idx = Pos[pos];
        int part_end_idx = pos == P_size - 1 ? Input_size : Pos[pos+1];

        llong part_start = pos == 0 ? 0 : P[pos-1];
        llong part_end = P[pos]; 

        #if DEBUG
        printf("\nPARTIÇÃO [%lld, %lld) (%d, %d)\n", part_start, part_end, part_start_idx, part_end_idx);
        #endif

        for (int i = part_start_idx; i < part_end_idx; i++) {

            #if DEBUG
            printf("%lld ", Output[i]);
            #endif

            if (Output[i] >= part_start && Output[i] < part_end) {
                continue;
            }
            else {
                printf("\nPARTIÇÃO [%lld, %lld) INCORRETA\n\n", part_start, part_end);
                return;
            }
        }
    }
    printf("\nPARTICIONAMENTO CORRETO\n\n");
}
