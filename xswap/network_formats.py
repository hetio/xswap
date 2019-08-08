from typing import List, Tuple, TypeVar


def matrix_to_edges(matrix: numpy.ndarray, include_reverse_edges: bool=True):
    """
    Convert (bi)adjacency matrix to an edge list. Inverse of `edges_to_matrix`.

    Parameters
    ----------
    matrix : numpy.ndarray
        Adjacency matrix or biadjacency matrix of a network
    include_reverse_edges : bool
        Whether to return edges that are the inverse of existing edges. For
        example, if returning [(0, 1), (1, 0)] is desired or not. If False,
        then only edges where source <= target are returned. This parameter
        should be `True` when passing a biadjacency matrix, as matrix positions
        indicate separate nodes.

    Returns
    -------
    edge_list : List[Tuple[int, int]]
        Edge list with node ids as the corresponding matrix indices. For example,
        if `matrix` has `matrix[0, 2] == 1`, then `(0, 2)` will be among the
        returned edges.
    """
    import scipy.sparse
    sparse = scipy.sparse.coo_matrix(matrix)
    edges = zip(sparse.row, sparse.col)

    if not include_reverse_edges:
        edges = filter(lambda edge: edge[0] <= edge[1], edges)
    return list(edges)


def edges_to_matrix(edge_list: List[Tuple[int, int]], add_reverse_edges: bool,
                    shape: Tuple[int, int], dtype: TypeVar=bool, sparse: bool=True):
    """
    Convert edge list to (bi)adjacency matrix. Inverse of `matrix_to_edges`.

    Parameters
    ----------
    edge_list : List[Tuple[int, int]]
        An edge list mapped such that node ids correspond to desired matrix
        positions. For example, (0, 0) will mean that the resulting matrix has
        a positive value of type `dtype` in that position.
    add_reverse_edges : bool
        Whether to include the reverse of edges in the matrix. For example,
        if `edge_list = [(1, 0)]` and `add_reverse_edge = True`, then the
        returned matrix has `matrix[1, 0]` = `matrix[0, 1]` = 1. Else, the matrix
        only has `matrix[1, 0]` = 1. If a biadjacency matrix is desired, then
        set `add_reverse_edges = False`.
    shape : Tuple[int, int]
        Shape of the matrix to be returned. Allows edges to be converted to
        a matrix even when there are nodes without edges.
    dtype : data-type
        Dtype of the returned matrix. For example, `int`, `bool`, `float`, etc.
    sparse : bool
        Whether a sparse matrix should be returned. If `False`, returns a dense
        numpy.ndarray

    Returns
    -------
    matrix : scipy.sparse.csc_matrix or numpy.ndarray
    """
    import numpy
    import scipy.sparse
    matrix = scipy.sparse.csc_matrix(
        (numpy.ones(len(edge_list)), zip(*edge_list)), dtype=dtype, shape=shape,
    )

    if add_reverse_edges:
        matrix = (matrix + matrix.T) > 0
        matrix = matrix.astype(dtype)

    if not sparse:
        matrix = matrix.toarray()

    return matrix
