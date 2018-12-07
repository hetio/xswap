#include <cstdlib>
#include "hash_table.h"

EdgeHashTable::EdgeHashTable(int max_source, int max_target) {
    int max_pair[2] = {max_source, max_target};
    int max_cantor = cantor_pair(max_pair);
    create_hash_table(max_cantor);
}

bool EdgeHashTable::contains(int *edge) {
    int edge_cantor = cantor_pair(edge);
    return (bool)get_bit(hash_table[edge_cantor / CHAR_BITS], edge_cantor % CHAR_BITS);
}

void EdgeHashTable::add(int *edge) {
    int edge_cantor = cantor_pair(edge);
    set_bit_true(&hash_table[edge_cantor / CHAR_BITS], edge_cantor % CHAR_BITS);
}

void EdgeHashTable::remove(int *edge) {
    int edge_cantor = cantor_pair(edge);
    set_bit_false(&hash_table[edge_cantor / CHAR_BITS], edge_cantor % CHAR_BITS);
}

void EdgeHashTable::free_table() {
    free(hash_table);
}

// num_elements corresponds to the minimum number of bits that are needed
void EdgeHashTable::create_hash_table(int num_elements) {
    // Minimum sufficient number of bytes for the table "ceil(num_elements / CHAR_BITS)"
    int bytes_needed = (num_elements + CHAR_BITS - (num_elements % CHAR_BITS)) / CHAR_BITS;
    hash_table = (char*)calloc(bytes_needed, 1);
}

int EdgeHashTable::cantor_pair(int* edge) {
    int source = edge[0];
    int target = edge[1];
    return ((source + target) * (source + target + 1) / 2) + source;
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
