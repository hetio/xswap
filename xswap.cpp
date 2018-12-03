# include <iostream>
# include <random>
# include <string>

#include "stdlib.h"
#include <stdio.h>
#include <string.h>

#define CHAR_SIZE sizeof(char)


// Load from the pipe
unsigned int** get_edges_from_stdin(unsigned int num_edges) {
	unsigned int** returnMatrix;
	returnMatrix = (unsigned int**)malloc(sizeof(unsigned int*)*num_edges);
	memset(returnMatrix, 0, sizeof(unsigned int*)*num_edges); //Can be removed for efficeny

	unsigned int source, target;
	for (unsigned int i = 0; i < num_edges; i++) {
		scanf("%d %d", &source, &target);
		returnMatrix[i] = (unsigned int*)malloc(sizeof(unsigned int) * 2);
		returnMatrix[i][0] = source;
		returnMatrix[i][1] = target;
	}
	return returnMatrix;
}


// Pipe back to Python
void push_edges_to_stdout(unsigned int num_edges, unsigned int** edgeMatrix) {
	for (unsigned int i = 0; i < num_edges; i++) {
		fprintf(stdout, "%d %d\n", edgeMatrix[i][0], edgeMatrix[i][1]);
		free(edgeMatrix[i]); // Make sure that this frees all of the memory
	}
	printf("-1 -1\n");
	fflush(stdout);
}


// Unique map from NxN to N. Used to get a unique "hash" value for a 2-element array
// https://en.wikipedia.org/wiki/Pairing_function#Cantor_pairing_function
unsigned int cantor_pair(unsigned int* edge) {
	unsigned int source = edge[0];
	unsigned int target = edge[1];
	return ((source + target) * (source + target + 1) / 2) + target;
}

char get_n_bit(char word, char bit_position) {
    return (word >> ((7 - bit_position) & 0x1));
}

void set_n_bit_true(char* word, char bit_position) {
    *word |= (0x1 << (7 - bit_position));
}

void set_n_bit_false(char* word, char bit_position) {
    *word &= ~(0x1 << (7 - bit_position));
}


// unsigned int** permute_edges(unsigned int** edges, unsigned int** excluded_edges, int seed, bool directed) {}


int main(int argc, char const *argv[]) {
	unsigned int num_edges = std::stoi(argv[1]);
	unsigned int num_excluded_edges = std::stoi(argv[2]);
	unsigned int num_swaps = std::stoi(argv[3]);
	int seed = std::stoi(argv[4]);
	unsigned int max_source = std::stoi(argv[5]);
	unsigned int max_target = std::stoi(argv[6]);

	// Whether edge is directed or not. This affects how reverse edges are treated.
	bool directed = false;
	if (std::stoi(argv[7])) directed = true;

	// Allocate enough memory in hash table for all possible edges
	unsigned int max_pair[2];
	max_pair[0] = max_source;
	max_pair[1] = max_target;
	unsigned int size_of_hash = ((cantor_pair(max_pair) + 1) / CHAR_SIZE) + 1;

	// Create hash table to efficiently lookup whether edges already exist
	char* edge_hash_table = (char*)malloc(sizeof(char) * size_of_hash);
	memset(edge_hash_table, (char)0, sizeof(char) * size_of_hash);

	// Get edges from iostream
	unsigned int** edges = get_edges_from_stdin(num_edges);
	for (unsigned int i = 0; i < num_edges; i++) {
        unsigned int cantor_value = cantor_pair(edges[i]);
        set_n_bit_true(&edge_hash_table[cantor_value / CHAR_SIZE], cantor_value % CHAR_SIZE);
	}

	// Get excluded edges from iostream
	unsigned int** excluded_edges = get_edges_from_stdin(num_excluded_edges);

	// Initialize unbiased random number generator
	std::mt19937 rng(seed);
	std::uniform_int_distribution<unsigned int> uni(0, num_edges - 1);

	// Initialize stats counters for failure reasons
	unsigned int same_edge = 0;
	unsigned int self_loop = 0;
	unsigned int duplicate = 0;
	unsigned int undir_duplicate = 0;
	unsigned int excluded = 0;

	for (unsigned int i = 0; i < num_swaps; i++) {
		// Draw edges randomly
		unsigned int edge_index_a = uni(rng);
		unsigned int edge_index_b = uni(rng);

		if (edge_index_a == edge_index_b) {
			same_edge += 1;
			continue;
		}

		// Old edges
		unsigned int* edge_a = edges[edge_index_a];
		unsigned int* edge_b = edges[edge_index_b];

		// Form potential new edges
		unsigned int new_edge_a[2] = { edge_a[0], edge_b[1] };
		unsigned int new_edge_b[2] = { edge_b[0], edge_a[1] };
		unsigned int* new_edges[2] = { new_edge_a, new_edge_b };

		// Check validity of both new edges
		bool valid = true;
		for (int i = 0; i < 2; i++) {
			unsigned int* new_edge = new_edges[i];
			// New edge would be a self-loop
			if (new_edge[0] == new_edge[1]) {
				self_loop += 1;
				valid = false;
				break;
			}
			// New edge already exists
            unsigned int new_edge_cantor = cantor_pair(new_edge);
			if (get_n_bit(edge_hash_table[new_edge_cantor / CHAR_SIZE], new_edge_cantor % CHAR_SIZE)) {
				duplicate += 1;
				valid = false;
				break;
			}
			// Undirected and reverse of new edge already exists
			unsigned int reversed[2] = { new_edge[1], new_edge[0] };
            unsigned int reversed_cantor = cantor_pair(reversed);
			if (!directed && get_n_bit(edge_hash_table[reversed_cantor / CHAR_SIZE], reversed_cantor % CHAR_SIZE)) {
				undir_duplicate += 1;
				valid = false;
				break;
			}
			// New edges are in excluded edges. Because few excluded edges are expected,
			// this iterates in O(n) rather than allocating a large hash table for all
			// possible edges. Considerable memory savings when excluding few edges.
			for (unsigned int i = 0; i < num_excluded_edges; i++) {
				if (excluded_edges[i][0] == new_edge[0] && excluded_edges[i][1] == new_edge[1]) {
					excluded += 1;
					valid = false;
					break;
				}
			}
		}
		if (valid) {
            unsigned int cantor_a = cantor_pair(edge_a);
            unsigned int cantor_b = cantor_pair(edge_b);

            set_n_bit_false(&edge_hash_table[cantor_a / CHAR_SIZE], cantor_a % CHAR_SIZE);
            set_n_bit_false(&edge_hash_table[cantor_b / CHAR_SIZE], cantor_b % CHAR_SIZE);

			unsigned int temp_target = edge_a[1];
			edge_a[1] = edge_b[1];
			edge_b[1] = temp_target;

            cantor_a = cantor_pair(edge_a);
            cantor_b = cantor_pair(edge_b);

            set_n_bit_true(&edge_hash_table[cantor_a / CHAR_SIZE], cantor_a % CHAR_SIZE);
            set_n_bit_true(&edge_hash_table[cantor_b / CHAR_SIZE], cantor_b % CHAR_SIZE);
		}
	}
	push_edges_to_stdout(num_edges, edges);
	printf("%d\n%d\n%d\n%d\n%d\n", same_edge, self_loop, duplicate, undir_duplicate, excluded);
	free(edges);
	free(excluded_edges);
	free(edge_hash_table);
	return 0;
}