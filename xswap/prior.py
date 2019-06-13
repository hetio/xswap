import collections
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
                                    initial_seed: int = 0):
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

    # Set max_source and max_target to their mutual maximum to ensure enough
    # space is allocated for the bitset
    max_source = max_target = max(map(max, edge_list))

    edge_counter = scipy.sparse.csc_matrix(shape, dtype=int)

    for i in range(n_permutations):
        permuted_edges, stats = xswap._xswap_backend._xswap(
            edge_list, [], max_source, max_target, allow_self_loops,
            allow_antiparallel, num_swaps, initial_seed + i)
        permuted_matrix = xswap.network_formats.edges_to_matrix(
            permuted_edges, add_reverse_edges=(not allow_antiparallel),
            shape=shape, dtype=int, sparse=True)
        edge_counter += permuted_matrix

    return edge_counter


def compute_xswap_priors(edge_list: List[Tuple[int, int]], n_permutations: int,
                         shape: Tuple[int, int], allow_self_loops: bool=False,
                         allow_antiparallel: bool=False, swap_multiplier: int=10,
                         initial_seed: int=0):
    """
    Compute the XSwap prior for every potential edge in the network. Uses
    degree-grouping to maximize the effective number of permutations for each
    node pair.

    Note that the mechanics of this function are separated to minimize memory use.
    """
    edge_counter = compute_xswap_occurrence_matrix(
        edge_list=edge_list, n_permutations=n_permutations, shape=shape,
        allow_self_loops=allow_self_loops, allow_antiparallel=allow_antiparallel,
        swap_multiplier=swap_multiplier, initial_seed=initial_seed)

    # Setup
    prior_df = pd.DataFrame({
        'source_id': np.repeat(np.arange(shape[0], dtype=np.uint16), shape[1]),
        'target_id': np.tile(np.arange(shape[1], dtype=np.uint16), shape[0]),
        'num_permuted_edges': edge_counter.toarray().flatten(),
    })
    del edge_counter

    # Implement degree grouping to increase effective number of permutations
    prior_df = (
        prior_df
        .assign(
            # Presence of an edge in the original network
            edge=xswap.network_formats.edges_to_matrix(
                edge_list, add_reverse_edges=(not allow_antiparallel), shape=shape,
                dtype=bool, sparse=True).toarray().flatten(),
            source_degree=lambda df: df.groupby('source_id').transform(sum)['edge'],
            target_degree=lambda df: df.groupby('target_id').transform(sum)['edge'],
            # The effective number of permutations after degree-grouping
            num_dgp=lambda df: n_permutations * df.groupby(['source_id', 'target_id'])
                                                  .transform(len)['edge'],
            # The number of edges found in permuted networks (using DGP)
            dgp_edge_count=lambda df: df.groupby(['source_degree', 'target_degree'])
                                        .transform(sum)['num_permuted_edges'],
            xswap_prior=lambda df: df['dgp_edge_count'] / df['num_dgp']
        )
        .filter(items=['source_id', 'target_id', 'source_degree',
                       'target_degree', 'edge', 'num_dgp', 'xswap_prior'])
    )
    return prior_df
