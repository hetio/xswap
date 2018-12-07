#include <Python.h>
#include "xswap_wrapper.h"

#define XSWAP_MODULE

static int** py_list_to_edges(PyObject *py_list, int num_edges) {
    int** edges_array = (int**)malloc(sizeof(int*) * num_edges);

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

static PyObject* edge_to_py_tuple(int *edge) {
    PyObject* edge_tuple = PyTuple_New(2);
    for (int j = 0; j < 2; j++) {
        PyObject* node_id = PyLong_FromLong(edge[j]);
        PyTuple_SET_ITEM(edge_tuple, j, node_id);
    }
    return edge_tuple;
}

static PyObject* edges_to_py_list(int **edges, int num_edges) {
    PyObject* py_list = PyList_New(num_edges);

    for (int i = 0; i < num_edges; i++) {
        PyObject* edge_tuple = edge_to_py_tuple(edges[i]);
        PyList_SET_ITEM(py_list, i, edge_tuple);
    }
    return py_list;
}

static PyObject* stats_to_py_dict(int num_swaps, int same_edge, int self_loop,
                                  int duplicate, int undir_duplicate, int excluded) {
    PyObject* py_num_swaps = PyLong_FromLong(num_swaps);
    PyObject* py_same_edge = PyLong_FromLong(same_edge);
    PyObject* py_self_loop = PyLong_FromLong(self_loop);
    PyObject* py_duplicate = PyLong_FromLong(duplicate);
    PyObject* py_undir_duplicate = PyLong_FromLong(undir_duplicate);
    PyObject* py_excluded = PyLong_FromLong(excluded);

    PyObject* dict = PyDict_New();
    PyDict_SetItemString(dict, "swap_attempts", py_num_swaps);
    PyDict_SetItemString(dict, "same_edge", py_same_edge);
    PyDict_SetItemString(dict, "self_loop", py_self_loop);
    PyDict_SetItemString(dict, "duplicate", py_duplicate);
    PyDict_SetItemString(dict, "undir_duplicate", py_undir_duplicate);
    PyDict_SetItemString(dict, "excluded", py_excluded);
    return dict;
}

static PyObject* wrap_xswap(PyObject *self, PyObject *args) {
    // Get arguments from python and compute quantities where needed
    PyObject *py_edges, *py_excluded_edges;
    int max_source, max_target, num_swaps, seed, directed;
    int parsed_successfully = PyArg_ParseTuple(args, "OOiipii", &py_edges,
        &py_excluded_edges, &max_source, &max_target, &directed, &num_swaps, &seed);
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

    // Initialize stats counters for failure reasons
    int same_edge = 0;
    int self_loop = 0;
    int duplicate = 0;
    int undir_duplicate = 0;
    int excluded = 0;

    // Perform XSwap
    swap_edges(edges, num_edges, excluded_edges, num_excluded_edges, edges_set,
              seed, directed, num_swaps, &same_edge, &self_loop, &duplicate,
              &undir_duplicate, &excluded);

    // Get new edges as python list
    PyObject* py_list = edges_to_py_list(edges, num_edges);

    // Get stats as python dict
    PyObject* stats_py_dict = stats_to_py_dict(num_swaps, same_edge, self_loop,
                                               duplicate, undir_duplicate, excluded);

    // Create and return a python tuple of new_edges, stats
    PyObject* return_tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(return_tuple, 0, py_list);
    PyTuple_SET_ITEM(return_tuple, 1, stats_py_dict);
    free(edges);
    free(excluded_edges);
    edges_set.free_table();
    return return_tuple;
}

static PyMethodDef XSwapMethods[] = {
    {"_xswap", wrap_xswap, METH_VARARGS, "Backend for edge permutation"},
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
