import numpy
import pytest
import scipy.sparse

import xswap.network_formats


@pytest.mark.parametrize('matrix,correct_edges,include_reverse_edges', [
    (numpy.array([[1,0,0,0],[0,0,1,0],[0,0,0,1]]), [(0, 0), (1, 2), (2, 3)], False),
    (numpy.array([[1,0,0],[0,0,1],[0,1,1]]), [(0, 0), (1, 2), (2, 2)], False),
    (numpy.array([[1,0,0],[0,0,1],[0,1,1]]), [(0, 0), (1, 2), (2, 1), (2, 2)], True),
])
def test_matrix_to_edges(matrix, correct_edges, include_reverse_edges):
    edges = xswap.network_formats.matrix_to_edges(matrix, include_reverse_edges)
    assert sorted(edges) == sorted(correct_edges)


@pytest.mark.parametrize('edges,correct_matrix,add_reverse_edges,shape,dtype,sparse', [
    (
        [(0, 1), (0, 3), (2, 2)],
        numpy.array([[0,1,0,1], [1,0,0,0], [0,0,1,0], [1,0,0,0]], dtype=int),
        True, (4, 4), int, False),
    (
        [(0, 1), (0, 3), (2, 2)],
        numpy.array([[0,1,0,1], [0,0,0,0], [0,0,1,0], [0,0,0,0]], dtype=int),
        False, (4, 4), int, False),
    (
        [(0, 1), (0, 3), (2, 2)],
        numpy.array([[0,1,0,1], [0,0,0,0], [0,0,1,0]], dtype=int),
        False, (3, 4), int, False),
    (
        [(0, 1), (0, 3), (2, 2)],
        numpy.array([[0,1,0,1], [0,0,0,0], [0,0,1,0]], dtype=float),
        False, (3, 4), float, False),
    (
        [(0, 1), (0, 3), (2, 2)],
        numpy.array([[0,1,0,1], [0,0,0,0], [0,0,1,0]], dtype=numpy.float32),
        False, (3, 4), numpy.float32, False),
    (
        [(0, 1), (0, 3), (2, 2)],
        scipy.sparse.csc_matrix([[0,1,0,1], [0,0,0,0], [0,0,1,0]], dtype=numpy.float32),
        False, (3, 4), numpy.float32, True),
])
def test_edges_to_matrix(edges, correct_matrix, add_reverse_edges, shape, dtype, sparse):
    matrix = xswap.network_formats.edges_to_matrix(
        edge_list=edges, add_reverse_edges=add_reverse_edges, shape=shape,
        dtype=dtype, sparse=sparse)

    assert matrix.dtype == dtype
    assert scipy.sparse.issparse(matrix) == sparse
    if sparse:
        assert (matrix != correct_matrix).nnz == 0
    else:
        assert numpy.array_equal(matrix, correct_matrix)
