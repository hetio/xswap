#include <cstdlib>
#include <stdexcept>
#include <iostream>
#include "xswap.h"

int CHAR_BITS = 8*sizeof(char);
unsigned long long int MAX_MALLOC = 4000000000;

size_t cantor_pair(int* edge) {
    size_t source = edge[0];
    size_t target = edge[1];
    return ((source + target) * (source + target + 1) / 2) + target;
}

EdgeHashTable::EdgeHashTable(int max_source, int max_target) {
    int max_pair[2] = {max_source, max_target};
    max_cantor = cantor_pair(max_pair);
    create_hash_table(max_cantor);
}

EdgeHashTable::EdgeHashTable(Edges edges) {
    int max_pair[2] = {edges.max_source, edges.max_target};
    max_cantor = cantor_pair(max_pair);
    create_hash_table(max_cantor);
    for (int i = 0; i < edges.num_edges; i++) {
        add(edges.edge_array[i]);
    }
}

bool EdgeHashTable::contains(int *edge) {
    size_t edge_cantor = cantor_pair(edge);
    if (edge_cantor > max_cantor)
        throw std::out_of_range("Attempting to check membership for out-of-bounds element.");
    return (bool)get_bit(hash_table[edge_cantor / CHAR_BITS], edge_cantor % CHAR_BITS);
}

void EdgeHashTable::add(int *edge) {
    size_t edge_cantor = cantor_pair(edge);
    if (edge_cantor > max_cantor) {
        throw std::out_of_range("Attempting to add an out-of-bounds element to the hash table.");
    }
    if (get_bit(hash_table[edge_cantor / CHAR_BITS], edge_cantor % CHAR_BITS)) {
        throw std::logic_error("Attempting to add an existing element.");
    }
    set_bit_true(&hash_table[edge_cantor / CHAR_BITS], edge_cantor % CHAR_BITS);
}

void EdgeHashTable::remove(int *edge) {
    size_t edge_cantor = cantor_pair(edge);
    if (edge_cantor > max_cantor)
        throw std::out_of_range("Attempting to remove an out-of-bounds element.");
    if (!get_bit(hash_table[edge_cantor / CHAR_BITS], edge_cantor % CHAR_BITS))
        throw std::logic_error("Attempting to remove a nonexisting element.");
    set_bit_false(&hash_table[edge_cantor / CHAR_BITS], edge_cantor % CHAR_BITS);
}

void EdgeHashTable::free_table() {
    free(hash_table);
}

// num_elements corresponds to the minimum number of bits that are needed
void EdgeHashTable::create_hash_table(size_t num_elements) {
    // Minimum sufficient number of bytes for the table "ceil(num_elements / CHAR_BITS)"
    size_t bytes_needed = (num_elements + CHAR_BITS - (num_elements % CHAR_BITS)) / CHAR_BITS;
    if (bytes_needed > MAX_MALLOC) {
        throw std::runtime_error("Hash table requires too much memory.");
    }
    hash_table = (char*)calloc(bytes_needed, 1);
}

/* Gets the bit from byte `word` at position `bit_position`. In the table, bits
 correspond to cantor pair values 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, etc. To access
 the bit corresponding to cantor pair value 9, call `get_bit` with `word` equal
 to the second bit and `bit_position` equal to 1 (ie. the second bit).
 `word >> (7 - bit_position)` puts the selected bit in the least significant position */
char EdgeHashTable::get_bit(char word, char bit_position) {
    return (word >> (7 - bit_position)) & 0x1;
}

void EdgeHashTable::set_bit_true(char* word, char bit_position) {
    *word |= (0x1 << (7 - bit_position));
}

void EdgeHashTable::set_bit_false(char* word, char bit_position) {
    *word &= ~(0x1 << (7 - bit_position));
}

BigHashTable::BigHashTable(Edges edges) {
    for (int i = 0; i < edges.num_edges; i++) {
        int* edge = edges.edge_array[i];
        add(edge);
    }
}

bool BigHashTable::contains(int *edge) {
    size_t edge_cantor = cantor_pair(edge);
    return hash_table.count(edge_cantor);
}

void BigHashTable::add(int *edge) {
    if (contains(edge)) {
        throw std::logic_error("Attempting to add an existing element.");
    }
    size_t edge_cantor = cantor_pair(edge);
    hash_table.insert(edge_cantor);
    // hash_table.emplace(edge_cantor);
}

void BigHashTable::remove(int *edge) {
    if (!contains(edge)) {
        throw std::logic_error("Attempting to remove a nonexisting element.");
    }
    size_t edge_cantor = cantor_pair(edge);
    hash_table.erase(edge_cantor);
}

HashTable::HashTable(Edges edges) {
    int max_pair[2] = {edges.max_source, edges.max_target};
    size_t max_cantor = cantor_pair(max_pair);
    if (max_cantor < MAX_MALLOC) {
        std::cout << "Using fast hash table\n";
        uses_big = false;
        edge_hash_table = EdgeHashTable(edges);
    } else {
        std::cout << "Using slow hash table\n";
        uses_big = true;
        big_hash_table = BigHashTable(edges);
    }
}

bool HashTable::contains(int *edge) {
    if (uses_big) {
        return big_hash_table.contains(edge);
    } else {
        return edge_hash_table.contains(edge);
    }
}

void HashTable::add(int *edge) {
    if (uses_big) {
        return big_hash_table.add(edge);
    } else {
        return edge_hash_table.add(edge);
    }
}

void HashTable::remove(int *edge) {
    if (uses_big) {
        return big_hash_table.remove(edge);
    } else {
        return edge_hash_table.remove(edge);
    }
}

void HashTable::free_table() {
    if (uses_big) {
        return;
    } else {
        edge_hash_table.free_table();
    }
}
