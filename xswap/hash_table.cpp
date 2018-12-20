#include <cstdlib>
#include <stdexcept>
#include <iostream>
#include "xswap.h"

EdgeHashTable::EdgeHashTable(int max_source, int max_target) {
    int max_pair[2] = {max_source, max_target};
    int max_cantor = cantor_pair(max_pair);
    hash_table.resize(max_cantor + 1);
}

EdgeHashTable::EdgeHashTable(Edges edges) {
    int max_pair[2] = {edges.max_source, edges.max_target};
    int max_cantor = cantor_pair(max_pair);
    hash_table.resize(max_cantor + 1);
    for (int i = 0; i < edges.num_edges; i++) {
        add(edges.edge_array[i]);
    }
}

bool EdgeHashTable::contains(int *edge) {
    int edge_cantor = cantor_pair(edge);
    return hash_table.at(edge_cantor);
}

void EdgeHashTable::add(int *edge) {
    int edge_cantor = cantor_pair(edge);
    if (hash_table.at(edge_cantor)) {
        throw std::logic_error("Attempting to add an existing element.");
    }
    hash_table.at(edge_cantor) = true;
}

void EdgeHashTable::remove(int *edge) {
    int edge_cantor = cantor_pair(edge);
    if (!hash_table.at(edge_cantor)) {
        throw std::logic_error("Attempting to remove a nonexisting element.");
    }
    hash_table.at(edge_cantor) = false;
}

int EdgeHashTable::cantor_pair(int* edge) {
    int source = edge[0];
    int target = edge[1];
    return ((source + target) * (source + target + 1) / 2) + source;
}
