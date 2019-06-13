#include "xswap.h"

#define XSWAP_MODULE

static Edges py_list_to_edges(PyObject *py_list) {
    int num_edges = (int)PyList_Size(py_list);
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
    Edges return_object;
    return_object.edge_array = edges_array;
    return_object.num_edges = num_edges;
    return return_object;
}

static PyObject* edge_to_py_tuple(int *edge) {
    PyObject* edge_tuple = PyTuple_New(2);
    for (int j = 0; j < 2; j++) {
        PyObject* node_id = PyLong_FromLong(edge[j]);
        PyTuple_SET_ITEM(edge_tuple, j, node_id);
    }
    return edge_tuple;
}

static PyObject* edges_to_py_list(Edges edges) {
    int num_edges = edges.num_edges;
    PyObject* py_list = PyList_New(num_edges);

    for (int i = 0; i < num_edges; i++) {
        PyObject* edge_tuple = edge_to_py_tuple(edges.edge_array[i]);
        PyList_SET_ITEM(py_list, i, edge_tuple);
    }
    return py_list;
}

static PyObject* stats_to_py_dict(statsCounter& stats) {
    PyObject* py_num_swaps = PyLong_FromLong(stats.num_swaps);
    PyObject* py_same_edge = PyLong_FromLong(stats.same_edge);
    PyObject* py_self_loop = PyLong_FromLong(stats.self_loop);
    PyObject* py_duplicate = PyLong_FromLong(stats.duplicate);
    PyObject* py_undir_duplicate = PyLong_FromLong(stats.undir_duplicate);
    PyObject* py_excluded = PyLong_FromLong(stats.excluded);

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
    int max_id, num_swaps, seed, allow_self_loop, allow_antiparallel;
    unsigned long long int max_malloc;
    int parsed_successfully = PyArg_ParseTuple(args, "OOippiiK", &py_edges,
        &py_excluded_edges, &max_id, &allow_self_loop,
        &allow_antiparallel, &num_swaps, &seed, &max_malloc);
    if (!parsed_successfully)
        return NULL;

    // Load edges from python list
    Edges edges = py_list_to_edges(py_edges);
    edges.max_id = max_id;
    Edges excluded_edges = py_list_to_edges(py_excluded_edges);

    // Set the conditions under which new edges are accepted
    Conditions valid_cond;
    valid_cond.seed = seed;
    valid_cond.allow_self_loop = allow_self_loop;
    valid_cond.allow_antiparallel = allow_antiparallel;
    valid_cond.excluded_edges = excluded_edges;

    // Initialize stats counters for failure reasons
    statsCounter stats;
    stats.num_swaps = num_swaps;

    // Perform XSwap
    swap_edges(edges, num_swaps, valid_cond, &stats, (unsigned long long int)max_malloc);

    // Get new edges as python list
    PyObject* py_list = edges_to_py_list(edges);

    // Get stats as python dict
    PyObject* stats_py_dict = stats_to_py_dict(stats);

    // Create and return a python tuple of new_edges, stats
    PyObject* return_tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(return_tuple, 0, py_list);
    PyTuple_SET_ITEM(return_tuple, 1, stats_py_dict);
    for (int i = 0; i < edges.num_edges; i++) {
        free(edges.edge_array[i]);
    }
    free(edges.edge_array);
    for (int i = 0; i < valid_cond.excluded_edges.num_edges; i++) {
        free(valid_cond.excluded_edges.edge_array[i]);
    }
    free(valid_cond.excluded_edges.edge_array);
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
