from typing import List, Tuple

import numpy
import pandas
import scipy.sparse

import xswap._xswap_backend
import xswap.network_formats


def compute_xswap_occurrence_matrix(edge_list: List[Tuple[int, int]],
                                    n_permutations: int,
                                    shape: Tuple[int, int],
                                    allow_self_loops: bool = False,
                                    allow_antiparallel: bool = False,
                                    swap_multiplier: float = 10,
                                    initial_seed: int = 0,
                                    max_malloc: int = 4000000000):
    """
    Compute the XSwap prior probability for every node pair in a network. The
    XSwap prior is the probability of a node pair having an edge between them in
    degree-preserving permutations of a network. The prior value for a node
    pair can be considered as the probability of an edge existing between two
    nodes given only the network's degree sequence.

    Parameters
    ----------
    edge_list : List[Tuple[int, int]]
        Edge list representing the graph whose XSwap edge priors are to be
        computed. Tuples contain integer values representing nodes. No value
        should be greater than C++'s `INT_MAX`, in this case 2_147_483_647.
        An adjacency matrix will be created assuming that a node's value is its
        index in the matrix. If not, map edges (identifiers can be string or
        otherwise) using `xswap.preprocessing.map_str_edges`.
    n_permutations : int
        The number of permuted networks used to compute the empirical XSwap prior
    shape : Tuple[int, int]
        The shape of the matrix to be returned. In other words, a tuple of the
        number of source and target nodes.
    allow_self_loops : bool
        Whether to allow edges like (0, 0). In the case of bipartite graphs,
        such an edge represents a connection between two distinct nodes, while
        in other graphs it may represent an edge from a node to itself, in which
        case an edge may or may not be meaningful depending on context.
    allow_antiparallel : bool
        Whether to allow simultaneous edges like (0, 1) and (1, 0). In the case
        of bipartite graphs, these edges represent two connections between four
        distinct nodes, while for other graphs, these may be connections between
        the same two nodes.
    swap_multiplier : float
        The number of edge swap attempts is determined by the product of the
        number of existing edges and multiplier. For example, if five edges are
        passed and multiplier is set to 10, 50 swaps will be attempted. Non-integer
        products will be rounded down to the nearest integer.
    initial_seed : int
        Random seed that will be passed to the C++ Mersenne Twister 19937 random
        number generator. `initial_seed` will be used for the first permutation,
        and the seed used for each subsequent permutation will be incremented by
        one. For example, if `initial_seed` is 0 and `n_permutations` is 2, then
        the two permutations will pass seeds 0 and 1, respectively.
    max_malloc : int (`unsigned long long int` in C)
        The maximum amount of memory to be allocated using `malloc` when making
        a bitset to hold edges. An uncompressed bitset is implemented for
        holding edges that is significantly faster than alternatives. However,
        it is memory-inefficient and will not be used if more memory is required
        than `max_malloc`. Above the threshold, a Roaring bitset will be used.

    Returns
    -------
    edge_counter : scipy.sparse.csc_matrix
        Adjacency matrix with entries equal to the number of permutations in
        which a given edge appeared
    """
    if len(edge_list) != len(set(edge_list)):
        raise ValueError("Edge list contained duplicate edges. "
                         "XSwap does not support multigraphs.")

    num_swaps = int(swap_multiplier * len(edge_list))

    max_id = max(map(max, edge_list))

    edge_counter = scipy.sparse.csc_matrix(shape, dtype=int)

    for i in range(n_permutations):
        permuted_edges, stats = xswap._xswap_backend._xswap(
            edge_list, [], max_id, allow_self_loops, allow_antiparallel,
            num_swaps, initial_seed + i, max_malloc)
        permuted_matrix = xswap.network_formats.edges_to_matrix(
            permuted_edges, add_reverse_edges=(not allow_antiparallel),
            shape=shape, dtype=int, sparse=True)
        edge_counter += permuted_matrix

    return edge_counter


def compute_xswap_priors(edge_list: List[Tuple[int, int]], n_permutations: int,
                         shape: Tuple[int, int], allow_self_loops: bool = False,
                         allow_antiparallel: bool = False,
                         swap_multiplier: int = 10, initial_seed: int = 0,
                         max_malloc: int = 4000000000,
                         dtypes = {'id': numpy.uint16, 'degree': numpy.uint16,
                                   'edge': bool, 'xswap_prior': float},
                        ):
    """
    Compute the XSwap prior for every potential edge in the network. Uses
    degree-grouping to maximize the effective number of permutations for each
    node pair. That is, node pairs with the same source and target degrees can
    be grouped when computing the XSwap prior, allowing there to be more
    permutations for some node pairs than `n_permutations`.

    Note that the mechanics of this function are separated to minimize memory use.

    Parameters
    ----------
    edge_list : List[Tuple[int, int]]
        Edge list representing the graph whose XSwap edge priors are to be
        computed. Tuples contain integer values representing nodes. No value
        should be greater than C++'s `INT_MAX`, in this case 2_147_483_647.
        An adjacency matrix will be created assuming that a node's value is its
        index in the matrix. If not, map edges (identifiers can be string or
        otherwise) using `xswap.preprocessing.map_str_edges`.
    n_permutations : int
        The number of permuted networks used to compute the empirical XSwap prior
    shape : Tuple[int, int]
        The shape of the matrix to be returned. In other words, a tuple of the
        number of source and target nodes.
    allow_self_loops : bool
        Whether to allow edges like (0, 0). In the case of bipartite graphs,
        such an edge represents a connection between two distinct nodes, while
        in other graphs it may represent an edge from a node to itself, in which
        case an edge may or may not be meaningful depending on context.
    allow_antiparallel : bool
        Whether to allow simultaneous edges like (0, 1) and (1, 0). In the case
        of bipartite graphs, these edges represent two connections between four
        distinct nodes, while for other graphs, these may be connections between
        the same two nodes.
    swap_multiplier : float
        The number of edge swap attempts is determined by the product of the
        number of existing edges and multiplier. For example, if five edges are
        passed and multiplier is set to 10, 50 swaps will be attempted. Non-integer
        products will be rounded down to the nearest integer.
    initial_seed : int
        Random seed that will be passed to the C++ Mersenne Twister 19937 random
        number generator. `initial_seed` will be used for the first permutation,
        and the seed used for each subsequent permutation will be incremented by
        one. For example, if `initial_seed` is 0 and `n_permutations` is 2, then
        the two permutations will pass seeds 0 and 1, respectively.
    max_malloc : int (`unsigned long long int` in C)
        The maximum amount of memory to be allocated using `malloc` when making
        a bitset to hold edges. An uncompressed bitset is implemented for
        holding edges that is significantly faster than alternatives. However,
        it is memory-inefficient and will not be used if more memory is required
        than `max_malloc`. Above the threshold, a Roaring bitset will be used.
    dtypes : dict
        Dictionary mapping returned column types to dtypes. Keys should be
        `'id'`, `'degree'`, `'edge'`, and `'xswap_prior'`. `dtype` need only
        be changed from its defaults if the values of `id` or `degree` are
        greater than the maxima in the default dtypes, or in cases where greater
        precision is desired. (`numpy.uint16` has a maximum value of 65535.)

    Returns
    -------
    prior_df : pandas.DataFrame
        Columns are the following:
        [source_id, target_id, edge, source_degree, target_degree, xswap_prior]
    """
    # Compute the adjacency matrix of the original (unpermuted) network
    original_edges = xswap.network_formats.edges_to_matrix(
        edge_list, add_reverse_edges=(not allow_antiparallel), shape=shape,
        dtype=dtypes['edge'], sparse=True)

    # Setup DataFrame for recording prior data
    prior_df = pandas.DataFrame({
        'source_id': numpy.repeat(numpy.arange(shape[0], dtype=dtypes['id']), shape[1]),
        'target_id': numpy.tile(numpy.arange(shape[1], dtype=dtypes['id']), shape[0]),
        'edge': original_edges.toarray().flatten(),
    })
    del original_edges

    prior_df['source_degree'] = (prior_df
                                 .groupby('source_id')
                                 .transform(sum)['edge']
                                 .astype(dtypes['degree']))
    del prior_df['source_id']

    prior_df['target_degree'] = (prior_df
                                 .groupby('target_id')
                                 .transform(sum)['edge']
                                 .astype(dtypes['degree']))
    del prior_df['target_id']

    # Compute the number of occurrences of each edge across permutations
    edge_counter = compute_xswap_occurrence_matrix(
        edge_list=edge_list, n_permutations=n_permutations, shape=shape,
        allow_self_loops=allow_self_loops, allow_antiparallel=allow_antiparallel,
        swap_multiplier=swap_multiplier, initial_seed=initial_seed,
        max_malloc=max_malloc)

    prior_df['num_permuted_edges'] = edge_counter.astype(dtypes['degree']).toarray().flatten()
    del edge_counter

    # The number of edges that occurred across all node pairs with the same
    # `source_degree` and `target_degree`
    dgp_edge_count = (
        prior_df
        .groupby(['source_degree', 'target_degree'])
        .transform(sum)['num_permuted_edges']
        .values
        .astype(dtypes['degree'])
    )
    del prior_df['num_permuted_edges']

    # The effective number of permutations for every node pair, incorporating
    # degree-grouping
    num_dgp = (
        n_permutations * prior_df.groupby(['source_degree', 'target_degree'])
                                 .transform(len)['edge']
                                 .values
                                 .astype(dtypes['degree'])
    )
    xswap_prior = (dgp_edge_count / num_dgp).astype(dtypes['xswap_prior'])
    del dgp_edge_count, num_dgp

    prior_df['xswap_prior'] = xswap_prior
    del xswap_prior

    prior_df = (
        prior_df
        .assign(
            source_id=numpy.repeat(numpy.arange(shape[0], dtype=dtypes['id']), shape[1]),
            target_id=numpy.tile(numpy.arange(shape[1], dtype=dtypes['id']), shape[0]),
        )
        .filter(items=['source_id', 'target_id', 'edge', 'source_degree',
                       'target_degree', 'xswap_prior'])
    )
    return prior_df


def approximate_xswap_prior(source_degree, target_degree, num_edges):
    """
    Approximate the XSwap prior by assuming that the XSwap Markov Chain is stationary.
    While this is not the case in reality, some networks' priors can be estimated
    very well using this equation.

    Parameters
    ----------
    source_degree : int, float, numpy.array, or pandas.Series
        The source degree for a single node pair or a number of source degrees.
        The type of object passed should match `target_degree`.
    target_degree : int, float, numpy.array, or pandas.Series
        The target degree for a single node pair or a number of target degrees.
        The type of object passed should match `source_degree`.
    num_edges : int or float
        The total number of edges in the network

    Returns
    -------
    approximate_prior : float, numpy.array, or pandas.Series
        Output type matches the types of `source_degree` and `target_degree`.
    """
    return source_degree * target_degree / (
        source_degree * target_degree + num_edges - source_degree - target_degree + 1
    )
