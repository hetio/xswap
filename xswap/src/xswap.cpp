#include <random>
#include "xswap.h"

void swap_edges(Edges edges, int num_swaps, Conditions cond, statsCounter *stats) {
    // Initialize hash table for possible edges
    BitSet edges_set = BitSet(edges);

    // Initialize unbiased random number generator
    std::mt19937 rng(cond.seed);
    std::uniform_int_distribution<int> uni(0, edges.num_edges - 1);

    // Do XSwap
    for (int i = 0; i < num_swaps; i++) {
        // Draw edges randomly
        int edge_index_a = uni(rng);
        int edge_index_b = uni(rng);

        if (edge_index_a == edge_index_b) {
            stats->same_edge += 1;
            continue;
        }

        // Old edges
        int* edge_a = edges.edge_array[edge_index_a];
        int* edge_b = edges.edge_array[edge_index_b];

        // Form potential new edges
        int new_edge_a[2] = { edge_a[0], edge_b[1] };
        int new_edge_b[2] = { edge_b[0], edge_a[1] };
        int* new_edges[2] = { new_edge_a, new_edge_b };

        bool valid = is_valid_swap(new_edges, edges_set, cond, stats);
        if (valid) {
            edges_set.remove(edge_a);
            edges_set.remove(edge_b);

            int temp_target = edge_a[1];
            edge_a[1] = edge_b[1];
            edge_b[1] = temp_target;

            edges_set.add(new_edge_a);
            edges_set.add(new_edge_b);
        }
    }
}

bool is_valid_edge(int *new_edge, BitSet edges_set, Conditions valid_conditions,
                   statsCounter *stats) {
    // New edge would be a self-loop
    if (!valid_conditions.allow_self_loop && new_edge[0] == new_edge[1]) {
        stats->self_loop += 1;
        return false;
    }
    // New edge already exists
    if (edges_set.contains(new_edge)) {
        stats->duplicate += 1;
        return false;
    }
    // Undirected and reverse of new edge already exists
    int reversed[2] = { new_edge[1], new_edge[0] };
    if (!valid_conditions.allow_antiparallel && edges_set.contains(reversed)) {
        stats->undir_duplicate += 1;
        return false;
    }
    for (int i = 0; i < valid_conditions.excluded_edges.num_edges; i++) {
        if (valid_conditions.excluded_edges.edge_array[i][0] == new_edge[0] &&
            valid_conditions.excluded_edges.edge_array[i][1] == new_edge[1]) {
            stats->excluded += 1;
            return false;
        }
    }
    return true;
}

bool is_valid_swap(int **new_edges, BitSet edges_set, Conditions valid_conditions,
                   statsCounter *stats) {
    for (int i = 0; i < 2; i++) {
        bool is_valid = is_valid_edge(new_edges[i], edges_set, valid_conditions, stats);
        if (!is_valid) {
            return false;
        }
    }
    return true;
}
