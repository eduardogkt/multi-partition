#ifndef _VERIFICA_H_
#define _VERIFICA_H_

#ifndef llong
#define llong long long
#endif

// verifica se as partições criados por multi_partition estão corretas
void verifica_particoes(long long *Input, int n, long long *P, int np, long long *Output, int *Pos); 

#endif