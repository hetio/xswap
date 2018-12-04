#include <cstring>
#include <Python.h>
# include <random>

#include "hash_table.h"


int** py_list_to_edges(PyObject *py_list, int num_edges) {
    int** edges_array;

    edges_array = (int**)malloc(sizeof(int*) * num_edges);

    for (int i = 0; i < num_edges; i++) {
        edges_array[i] = (int*)malloc(sizeof(int) * 2);
        PyObject* py_tuple = PyList_GetItem(py_list, i);
        for (int j = 0; j < 2; j++) {
            PyObject* temp = PyTuple_GetItem(py_tuple, j);
            int value = (int)PyLong_AsLong(temp);
            edges_array[i][j] = value;
        }
    }
    return edges_array;
}

PyObject* edge_to_py_tuple(int *edge) {
    PyObject* edge_tuple = PyTuple_New(2);
    for (int j = 0; j < 2; j++) {
        PyObject* node_id = PyLong_FromLong(edge[j]);
        PyTuple_SET_ITEM(edge_tuple, j, node_id);
    }
    return edge_tuple;
}

PyObject* edges_to_py_list(int **edges, int num_edges) {
    PyObject* py_list = PyList_New(num_edges);

    for (int i = 0; i < num_edges; i++) {
        PyObject* edge_tuple = edge_to_py_tuple(edges[i]);
        PyList_SET_ITEM(py_list, i, edge_tuple);
    }
    return py_list;
}

static PyObject* _xswap_backend(PyObject *self, PyObject *args) {
    // Get arguments from python side of the API and compute quantities where needed
    PyObject *py_edges, *py_excluded_edges;
    int max_source, max_target, num_swaps, seed, directed;
    int parsed_successfully = PyArg_ParseTuple(args, "OOiipii", &py_edges, &py_excluded_edges,
        &max_source, &max_target, &directed, &num_swaps, &seed);
    if (!parsed_successfully)
        return NULL;

    // Load edges from python list
    int num_edges = (int)PyList_Size(py_edges);
    int num_excluded = (int)PyList_Size(py_excluded_edges);
    int** edges = py_list_to_edges(py_edges, num_edges);
    int** excluded_edges = py_list_to_edges(py_excluded_edges, num_excluded);

    // Create hash table for possible edges and add existing edges
    EdgeHashTable edges_set = EdgeHashTable(max_source, max_target);
    for (int i = 0; i < num_edges; i++) {
        edges_set.add(edges[i]);
    }

    // Initialize unbiased random number generator
    std::mt19937 rng(seed);
	std::uniform_int_distribution<int> uni(0, num_edges - 1);

    // Initialize stats counters for failure reasons
    int same_edge = 0;
	int self_loop = 0;
	int duplicate = 0;
	int undir_duplicate = 0;
	int excluded = 0;

    // Do XSwap
    for (int i = 0; i < num_swaps; i++) {
		// Draw edges randomly
		int edge_index_a = uni(rng);
		int edge_index_b = uni(rng);

		if (edge_index_a == edge_index_b) {
			same_edge += 1;
			continue;
		}

		// Old edges
		int* edge_a = edges[edge_index_a];
		int* edge_b = edges[edge_index_b];

		// Form potential new edges
		int new_edge_a[2] = { edge_a[0], edge_b[1] };
		int new_edge_b[2] = { edge_b[0], edge_a[1] };
		int* new_edges[2] = { new_edge_a, new_edge_b };

		// Check validity of both new edges
		bool valid = true;
		for (int i = 0; i < 2; i++) {
			int* new_edge = new_edges[i];
			// New edge would be a self-loop
			if (new_edge[0] == new_edge[1]) {
				self_loop += 1;
				valid = false;
				break;
			}
			// New edge already exists
            int new_edge_cantor = cantor_pair(new_edge);
			if (get_n_bit(edge_hash_table[new_edge_cantor / CHAR_SIZE], new_edge_cantor % CHAR_SIZE)) {
				duplicate += 1;
				valid = false;
				break;
			}
			// Undirected and reverse of new edge already exists
			int reversed[2] = { new_edge[1], new_edge[0] };
            int reversed_cantor = cantor_pair(reversed);
			if (!directed && get_n_bit(edge_hash_table[reversed_cantor / CHAR_SIZE], reversed_cantor % CHAR_SIZE)) {
				undir_duplicate += 1;
				valid = false;
				break;
			}
			// New edges are in excluded edges. Because few excluded edges are expected,
			// this iterates in O(n) rather than allocating a large hash table for all
			// possible edges. Considerable memory savings when excluding few edges.
			for (int i = 0; i < num_excluded_edges; i++) {
				if (excluded_edges[i][0] == new_edge[0] && excluded_edges[i][1] == new_edge[1]) {
					excluded += 1;
					valid = false;
					break;
				}
			}
		}
		if (valid) {
            int cantor_a = cantor_pair(edge_a);
            int cantor_b = cantor_pair(edge_b);

            set_n_bit_false(&edge_hash_table[cantor_a / CHAR_SIZE], cantor_a % CHAR_SIZE);
            set_n_bit_false(&edge_hash_table[cantor_b / CHAR_SIZE], cantor_b % CHAR_SIZE);

			int temp_target = edge_a[1];
			edge_a[1] = edge_b[1];
			edge_b[1] = temp_target;

            cantor_a = cantor_pair(edge_a);
            cantor_b = cantor_pair(edge_b);

            set_n_bit_true(&edge_hash_table[cantor_a / CHAR_SIZE], cantor_a % CHAR_SIZE);
            set_n_bit_true(&edge_hash_table[cantor_b / CHAR_SIZE], cantor_b % CHAR_SIZE);
		}
	}

    // Get new edges as python list
    // Get stats as python dict
    // Create and return a python tuple of new_edges, stats
    return PyLong_FromLong(6);
}

static PyObject* testo_parse_list_of_tuples(PyObject *self, PyObject *args) {
    PyObject* py_edges, py_excluded_edges;
    int max_source, max_target, num_swaps, seed, directed;
    int parsed_successfully = PyArg_ParseTuple(args, "OOiipii", &py_edges, &py_excluded_edges,
        &max_source, &max_target, &directed, &num_swaps, &seed);
    if (!parsed_successfully)
        return NULL;
    return PyLong_FromLong(5);
}

static PyObject* test_args(PyObject *self, PyObject *args) {
    int a, b, c;
    int d;
    if (!PyArg_ParseTuple(args, "iiip", &a, &b, &c, &d))
        return NULL;
    a += 1;
    b += 1;
    c += 1;
    PyObject* py_a = PyLong_FromLong(a);
    PyObject* py_b = PyLong_FromLong(b);
    PyObject* py_c = PyLong_FromLong(c);
    PyObject* return_tuple = PyTuple_New(4);
    PyObject* py_d = PyBool_FromLong(!d);
    PyTuple_SET_ITEM(return_tuple, 0, py_a);
    PyTuple_SET_ITEM(return_tuple, 1, py_b);
    PyTuple_SET_ITEM(return_tuple, 2, py_c);
    PyTuple_SET_ITEM(return_tuple, 3, py_d);
    return return_tuple;
}

static PyMethodDef TestoMethods[] = {
    {"_test_args", test_args, METH_VARARGS, "Test argument parsing."},
    {"parse_tuples", testo_parse_list_of_tuples, METH_VARARGS, "Parse args"},
    {"_xswap_backend", _xswap_backend, METH_VARARGS, "Backend for edge permutation"},
    {NULL, NULL, 0, NULL}
};

char test_doc[] = "TEST DOCS";

static struct PyModuleDef testomodule = {
    PyModuleDef_HEAD_INIT,
    "testo",   /* name of module */
    test_doc, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    TestoMethods
};

PyMODINIT_FUNC PyInit_testo(void) {
    return PyModule_Create(&testomodule);
}