#define _XSWAP

#ifndef _HASH_TABLE
#define _HASH_TABLE
#include "hash_table.h"
#endif

void swap_edges(int **edges, int num_edges, int **excluded_edges, int num_excluded_edges,
                 EdgeHashTable edges_set, int seed, int directed, int num_swaps,
                 int *same_edge, int *self_loop, int *duplicate,int *undir_duplicate,
                 int *excluded);
