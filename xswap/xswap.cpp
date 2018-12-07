#include <random>
#include "xswap.h"

void swap_edges(int **edges, int num_edges, int **excluded_edges, int num_excluded_edges,
                 EdgeHashTable edges_set, int seed, int directed, int num_swaps,
                 int *same_edge, int *self_loop, int *duplicate,int *undir_duplicate,
                 int *excluded) {
    // Initialize unbiased random number generator
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> uni(0, num_edges - 1);

    // Do XSwap
    for (int i = 0; i < num_swaps; i++) {
        // Draw edges randomly
        int edge_index_a = uni(rng);
        int edge_index_b = uni(rng);

        if (edge_index_a == edge_index_b) {
            same_edge += 1;
            continue;
        }

        // Old edges
        int* edge_a = edges[edge_index_a];
        int* edge_b = edges[edge_index_b];

        // Form potential new edges
        int new_edge_a[2] = { edge_a[0], edge_b[1] };
        int new_edge_b[2] = { edge_b[0], edge_a[1] };
        int* new_edges[2] = { new_edge_a, new_edge_b };
        // Check validity of both new edges
        bool valid = true;
        for (int i = 0; i < 2; i++) {
            int* new_edge = new_edges[i];
            // New edge would be a self-loop
            if (new_edge[0] == new_edge[1]) {
                self_loop += 1;
                valid = false;
                break;
            }
            // New edge already exists
            if (edges_set.contains(new_edge)) {
                duplicate += 1;
                valid = false;
                break;
            }
            // Undirected and reverse of new edge already exists
            int reversed[2] = { new_edge[1], new_edge[0] };
            if (!directed && edges_set.contains(reversed)) {
                undir_duplicate += 1;
                valid = false;
                break;
            }
            // New edges are in excluded edges. Because few excluded edges are expected,
            // this iterates in O(n) rather than allocating a large hash table for all
            // possible edges. Considerable memory savings when excluding few edges.
            // An alternative is to pass excluded edges as a Python set and to use the
            // built-in PySet_Contains function to check membership. This has the downside
            // that the edge must be instantiated as a new Python tuple object and the
            // values represented as new Python long integer objects.
            for (int i = 0; i < num_excluded_edges; i++) {
                if (excluded_edges[i][0] == new_edge[0] && excluded_edges[i][1] == new_edge[1]) {
                    excluded += 1;
                    valid = false;
                    break;
                }
            }
        }
        if (valid) {
            edges_set.remove(edge_a);
            edges_set.remove(edge_b);

            int temp_target = edge_a[1];
            edge_a[1] = edge_b[1];
            edge_b[1] = temp_target;

            edges_set.add(edge_a);
            edges_set.add(edge_b);
        }
    }
}
