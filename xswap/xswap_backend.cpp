#include <cstring>
#include <Python.h>
#include <random>

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

PyObject* stats_to_py_dict(int same_edge, int self_loop, int duplicate, int undir_duplicate, int excluded) {
    PyObject* py_same_edge = PyLong_FromLong(same_edge);
    PyObject* py_self_loop = PyLong_FromLong(self_loop);
    PyObject* py_duplicate = PyLong_FromLong(duplicate);
    PyObject* py_undir_duplicate = PyLong_FromLong(undir_duplicate);
    PyObject* py_excluded = PyLong_FromLong(excluded);

    PyObject* dict = PyDict_New();
    int code;
    code = PyDict_SetItemString(dict, "same_edge", py_same_edge);
    code = PyDict_SetItemString(dict, "self_loop", py_self_loop);
    code = PyDict_SetItemString(dict, "duplicate", py_duplicate);
    code = PyDict_SetItemString(dict, "undir_duplicate", py_undir_duplicate);
    code = PyDict_SetItemString(dict, "excluded", py_excluded);
    return dict;
}

static PyObject* _xswap(PyObject *self, PyObject *args) {
    // Get arguments from python and compute quantities where needed
    PyObject *py_edges, *py_excluded_edges;
    int max_source, max_target, num_swaps, seed, directed;
    int parsed_successfully = PyArg_ParseTuple(args, "OOiipii", &py_edges, &py_excluded_edges,
        &max_source, &max_target, &directed, &num_swaps, &seed);
    if (!parsed_successfully)
        return NULL;

    // Load edges from python list
    int num_edges = (int)PyList_Size(py_edges);
    int num_excluded_edges = (int)PyList_Size(py_excluded_edges);
    int** edges = py_list_to_edges(py_edges, num_edges);
    int** excluded_edges = py_list_to_edges(py_excluded_edges, num_excluded_edges);

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
            if (edges_set.contains(new_edge)) {
                duplicate += 1;
                valid = false;
                break;
            }
            // Undirected and reverse of new edge already exists
            int reversed[2] = { new_edge[1], new_edge[0] };
            if (!directed && edges_set.contains(reversed)) {
                undir_duplicate += 1;
                valid = false;
                break;
            }
            // New edges are in excluded edges. Because few excluded edges are expected,
            // this iterates in O(n) rather than allocating a large hash table for all
            // possible edges. Considerable memory savings when excluding few edges.
            // An alternative is to pass excluded edges as a Python set and to use the
            // built-in PySet_Contains function to check membership. This has the downside
            // that the edge must be instantiated as a new Python tuple object and the
            // values represented as new Python long integer objects.
            for (int i = 0; i < num_excluded_edges; i++) {
                if (excluded_edges[i][0] == new_edge[0] && excluded_edges[i][1] == new_edge[1]) {
                    excluded += 1;
                    valid = false;
                    break;
                }
            }
        }
        if (valid) {
            edges_set.remove(edge_a);
            edges_set.remove(edge_b);

            int temp_target = edge_a[1];
            edge_a[1] = edge_b[1];
            edge_b[1] = temp_target;

            edges_set.add(edge_a);
            edges_set.add(edge_b);
        }
    }

    // Get new edges as python list
    PyObject* py_list = edges_to_py_list(edges, num_edges);

    // Get stats as python dict
    PyObject* stats_py_dict = stats_to_py_dict(same_edge, self_loop, duplicate, undir_duplicate, excluded);

    // Create and return a python tuple of new_edges, stats
    PyObject* return_tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(return_tuple, 0, py_list);
    PyTuple_SET_ITEM(return_tuple, 1, stats_py_dict);
    free(edges);
    free(excluded_edges);
    return return_tuple;
}

static PyMethodDef XSwapMethods[] = {
    {"_xswap", _xswap, METH_VARARGS, "Backend for edge permutation"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef xswapmodule = {
    PyModuleDef_HEAD_INIT,
    "_xswap_backend",  /* name of module */
    NULL,  /* module documentation, NULL */
    -1,  /* -1 since the module keeps state in global variables. */
    XSwapMethods
};

PyMODINIT_FUNC PyInit__xswap_backend(void) {
    return PyModule_Create(&xswapmodule);
}
