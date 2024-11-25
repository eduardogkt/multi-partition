#ifndef _VERIFICA_H_
#define _VERIFICA_H_

#ifndef llong
#define llong long long
#endif

// verifica se as partições criados por multi_partition estão corretas
void verifica_particoes(llong *Input, int Input_size, 
                        llong *P, int P_size, 
                        llong *Output, int *Pos);

#endif