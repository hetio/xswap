#include "../lib/roaring.hh"

extern int CHAR_BITS;
extern unsigned long long int MAX_MALLOC;

struct Edges {
    int** edge_array;
    int num_edges;
    int max_source;
    int max_target;
};

// Slower bitset
class RoaringBitSet
{
    public:
        RoaringBitSet() = default;
        RoaringBitSet(Edges edges);
        bool contains(int *edge);
        void add(int *edge);
        void remove(int *edge);

    private:
        Roaring bitmap;
};

// Faster edge hash table for smaller numbers of edges
class UncompressedBitSet
{
    public:
        UncompressedBitSet() = default;
        UncompressedBitSet(int max_source, int max_target);
        UncompressedBitSet(Edges edges);
        bool contains(int *edge);
        void add(int *edge);
        void remove(int *edge);
        void free_table();

    private:
        char* hash_table;
        size_t max_cantor;
        void create_hash_table(size_t hash_size);
        char get_bit(char word, char bit_position);
        void set_bit_true(char* word, char bit_position);
        void set_bit_false(char* word, char bit_position);
};

// Wrapper class for the two hash table implementations
class BitSet
{
    public:
        BitSet(Edges edges);
        bool contains(int *edge);
        void add(int *edge);
        void remove(int *edge);
        void free_table();
        UncompressedBitSet uncompressed_set;

    private:
        bool use_compressed;
        RoaringBitSet compressed_set;
};

struct statsCounter {
    int num_swaps;
    int same_edge = 0;
    int self_loop = 0;
    int duplicate = 0;
    int undir_duplicate = 0;
    int excluded = 0;
};

struct Conditions {
    int seed;
    bool allow_antiparallel;
    bool allow_self_loop;
    Edges excluded_edges;
};

size_t cantor_pair(int* edge);

void swap_edges(Edges edges, int num_swaps, Conditions cond, statsCounter *stats);

bool is_valid_edge(int *edge, BitSet edges_set, Conditions cond,
                   statsCounter *stats);

bool is_valid_swap(int **new_edges, BitSet edges_set, Conditions cond,
                   statsCounter *stats);
