#define _HASH_TABLE

#define CHAR_BITS 8*sizeof(char)

class EdgeHashTable
{
    public:
        EdgeHashTable(int max_source, int max_target);
        bool contains(int *edge);
        void add(int *edge);
        void remove(int *edge);
        void free_table();

    private:
        char* hash_table;
        void create_hash_table(int hash_size);
        int cantor_pair(int* edge);
        char get_bit(char word, char bit_position);
        void set_bit_true(char* word, char bit_position);
        void set_bit_false(char* word, char bit_position);
};
