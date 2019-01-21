#include <iostream>
#include <stdexcept>
#include "xswap.h"

int CHAR_BITS = 8*sizeof(char);
unsigned long long int MAX_MALLOC = 4000000000;

size_t cantor_pair(int* edge) {
    size_t source = edge[0];
    size_t target = edge[1];
    return ((source + target) * (source + target + 1) / 2) + target;
}

UncompressedBitSet::UncompressedBitSet(int max_source, int max_target) {
    int max_pair[2] = {max_source, max_target};
    max_cantor = cantor_pair(max_pair);
    create_bitset(max_cantor);
}

UncompressedBitSet::UncompressedBitSet(Edges edges) {
    int max_pair[2] = {edges.max_source, edges.max_target};
    max_cantor = cantor_pair(max_pair);
    create_bitset(max_cantor);
    for (int i = 0; i < edges.num_edges; i++) {
        add(edges.edge_array[i]);
    }
}

bool UncompressedBitSet::contains(int *edge) {
    size_t edge_cantor = cantor_pair(edge);
    if (edge_cantor > max_cantor)
        throw std::out_of_range("Attempting to check membership for out-of-bounds element.");
    return (bool)get_bit(bitset[edge_cantor / CHAR_BITS], edge_cantor % CHAR_BITS);
}

void UncompressedBitSet::add(int *edge) {
    size_t edge_cantor = cantor_pair(edge);
    if (edge_cantor > max_cantor) {
        throw std::out_of_range("Attempting to add an out-of-bounds element to the bitset.");
    }
    if (get_bit(bitset[edge_cantor / CHAR_BITS], edge_cantor % CHAR_BITS)) {
        throw std::logic_error("Attempting to add an existing element.");
    }
    set_bit_true(&bitset[edge_cantor / CHAR_BITS], edge_cantor % CHAR_BITS);
}

void UncompressedBitSet::remove(int *edge) {
    size_t edge_cantor = cantor_pair(edge);
    if (edge_cantor > max_cantor)
        throw std::out_of_range("Attempting to remove an out-of-bounds element.");
    if (!get_bit(bitset[edge_cantor / CHAR_BITS], edge_cantor % CHAR_BITS))
        throw std::logic_error("Attempting to remove a nonexisting element.");
    set_bit_false(&bitset[edge_cantor / CHAR_BITS], edge_cantor % CHAR_BITS);
}

void UncompressedBitSet::free_array() {
    free(bitset);
}

// num_elements corresponds to the minimum number of bits that are needed
void UncompressedBitSet::create_bitset(size_t num_elements) {
    // Minimum sufficient number of bytes for the array "ceil(num_elements / CHAR_BITS)"
    size_t bytes_needed = (num_elements + CHAR_BITS - (num_elements % CHAR_BITS)) / CHAR_BITS;
    if (bytes_needed > MAX_MALLOC) {
        throw std::runtime_error("Bitset requires too much memory.");
    }
    bitset = (char*)calloc(bytes_needed, 1);
}

/* Gets the bit from byte `word` at position `bit_position`. In the array, bits
 correspond to cantor pair values 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, etc. To access
 the bit corresponding to cantor pair value 9, call `get_bit` with `word` equal
 to the second bit and `bit_position` equal to 1 (ie. the second bit).
 `word >> (7 - bit_position)` puts the selected bit in the least significant position */
char UncompressedBitSet::get_bit(char word, char bit_position) {
    return (word >> (7 - bit_position)) & 0x1;
}

void UncompressedBitSet::set_bit_true(char* word, char bit_position) {
    *word |= (0x1 << (7 - bit_position));
}

void UncompressedBitSet::set_bit_false(char* word, char bit_position) {
    *word &= ~(0x1 << (7 - bit_position));
}

RoaringBitSet::RoaringBitSet(Edges edges) {
    for (int i = 0; i < edges.num_edges; i++) {
        add(edges.edge_array[i]);
    }
}

bool RoaringBitSet::contains(int *edge) {
    int edge_cantor = cantor_pair(edge);
    return bitmap.contains(edge_cantor);
}

void RoaringBitSet::add(int *edge) {
    int edge_cantor = cantor_pair(edge);
    bool success = bitmap.addChecked(edge_cantor);
    if (!success) {
        throw std::logic_error("Attempting to add an existing element.");
    }
}

void RoaringBitSet::remove(int *edge) {
    int edge_cantor = cantor_pair(edge);
    bool success = bitmap.removeChecked(edge_cantor);
    if (!success) {
        throw std::logic_error("Attempting to remove a nonexisting element.");
    }
}

BitSet::BitSet(Edges edges) {
    int max_pair[2] = {edges.max_source, edges.max_target};
    size_t max_cantor = cantor_pair(max_pair);
    if (max_cantor < MAX_MALLOC) {
        std::cout << "Using fast bitset\n";
        use_compressed = false;
        uncompressed_set = UncompressedBitSet(edges);
    } else {
        std::cout << "Using slow bitset\n";
        use_compressed = true;
        compressed_set = RoaringBitSet(edges);
    }
}

bool BitSet::contains(int *edge) {
    if (use_compressed) {
        return compressed_set.contains(edge);
    } else {
        return uncompressed_set.contains(edge);
    }
}

void BitSet::add(int *edge) {
    if (use_compressed) {
        return compressed_set.add(edge);
    } else {
        return uncompressed_set.add(edge);
    }
}

void BitSet::remove(int *edge) {
    if (use_compressed) {
        return compressed_set.remove(edge);
    } else {
        return uncompressed_set.remove(edge);
    }
}

void BitSet::free_array() {
    if (use_compressed) {
        return;
    } else {
        uncompressed_set.free_array();
    }
}
