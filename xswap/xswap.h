#define _XSWAP
#define CHAR_BITS 8*sizeof(char)

struct Edges {
    int** edge_array;
    int num_edges;
    int max_source;
    int max_target;
};

class EdgeHashTable
{
    public:
        EdgeHashTable(int max_source, int max_target);
        EdgeHashTable(Edges edges);
        bool contains(int *edge);
        void add(int *edge);
        void remove(int *edge);
        void free_table();

    private:
        char* hash_table;
        int max_cantor;
        void create_hash_table(int hash_size);
        int cantor_pair(int* edge);
        char get_bit(char word, char bit_position);
        void set_bit_true(char* word, char bit_position);
        void set_bit_false(char* word, char bit_position);
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

void swap_edges(Edges edges, int num_swaps, Conditions cond, statsCounter stats);

bool is_valid_edge(int *edge, EdgeHashTable edges_set, Conditions cond,
                   statsCounter stats);

bool is_valid_swap(int **new_edges, EdgeHashTable edges_set, Conditions cond,
                   statsCounter stats);
