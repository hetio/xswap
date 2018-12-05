#include <cstdlib>
# include <cstring>
#include "hash_table.h"

EdgeHashTable::EdgeHashTable(int max_source, int max_target) {
    int max_pair[2] = {max_source, max_target};
    int size_of_hash = ((cantor_pair(max_pair) + 1) / CHAR_SIZE) + 1;
    create_hash_table(size_of_hash);
}

bool EdgeHashTable::contains(int *edge) {
    int edge_cantor = cantor_pair(edge);
    return (bool)get_bit(hash_table[edge_cantor / CHAR_SIZE], edge_cantor % CHAR_SIZE);
}

void EdgeHashTable::add(int *edge) {
    int edge_cantor = cantor_pair(edge);
    set_bit_true(&hash_table[edge_cantor / CHAR_SIZE], edge_cantor % CHAR_SIZE);
}

void EdgeHashTable::remove(int *edge) {
    int edge_cantor = cantor_pair(edge);
    set_bit_false(&hash_table[edge_cantor / CHAR_SIZE], edge_cantor % CHAR_SIZE);
}

void EdgeHashTable::create_hash_table(int hash_size) {
    hash_table = (char*)calloc(hash_size, sizeof(char));
}

int EdgeHashTable::cantor_pair(int* edge) {
    int source = edge[0];
    int target = edge[1];
    return ((source + target) * (source + target + 1) / 2) + target;
}

char EdgeHashTable::get_bit(char word, char bit_position) {
    return (word >> ((7 - bit_position) & 0x1));
}

void EdgeHashTable::set_bit_true(char* word, char bit_position) {
    *word |= (0x1 << (7 - bit_position));
}

void EdgeHashTable::set_bit_false(char* word, char bit_position) {
    *word &= ~(0x1 << (7 - bit_position));
}
