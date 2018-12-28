#include <unordered_set>

extern int CHAR_BITS;
extern unsigned long long int MAX_MALLOC;

struct Edges {
    int** edge_array;
    int num_edges;
    int max_source;
    int max_target;
};

// Wrapper class for the two hash table implementations
class EdgeHashTable
{
    public:
        EdgeHashTable()  = default;
        EdgeHashTable(int max_source, int max_target);
        EdgeHashTable(Edges edges);
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

// Slower edge hash table based on C++ standard library that can handle more edges
class BigHashTable
{
    public:
        BigHashTable() = default;
        BigHashTable(Edges edges);
        bool contains(int *edge);
        void add(int *edge);
        void remove(int *edge);

    private:
        std::unordered_set<size_t> hash_table;
};

// Faster edge hash table for smaller numbers of edges
class HashTable
{
    public:
        HashTable(Edges edges);
        bool contains(int *edge);
        void add(int *edge);
        void remove(int *edge);
        void free_table();

    private:
        bool uses_big;
        EdgeHashTable edge_hash_table;
        BigHashTable big_hash_table;
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

void swap_edges(Edges edges, int num_swaps, Conditions cond, statsCounter stats);

bool is_valid_edge(int *edge, HashTable edges_set, Conditions cond,
                   statsCounter stats);

bool is_valid_swap(int **new_edges, HashTable edges_set, Conditions cond,
                   statsCounter stats);
