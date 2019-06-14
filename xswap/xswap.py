from typing import List, Set, Tuple

import xswap._xswap_backend


def permute_edge_list(edge_list: List[Tuple[int, int]], allow_self_loops: bool = False,
                      allow_antiparallel: bool = False, multiplier: float = 10,
                      excluded_edges: Set[Tuple[int, int]] = set(), seed: int = 0,
                      max_malloc: int = 400000_000):
    """
    Permute the edges of a graph using the XSwap method given by Hanhijärvi,
    et al. (doi.org/f3mn58). XSwap is a degree-preserving network randomization
    technique that selects edges, checks the validity of the swap, and exchanges
    the target nodes between the edges. For information on what values to select
    for directed, please see README.md.

    Parameters
    ----------
    edge_list : List[Tuple[int, int]]
        Edge list representing the graph to be randomized. Tuples can contain
        integer values representing nodes. No value should be greater than C++'s
        `INT_MAX`, in this case 2_147_483_647.
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
    multiplier : float
        The number of edge swap attempts is determined by the product of the
        number of existing edges and multiplier. For example, if five edges are
        passed and multiplier is set to 10, 50 swaps will be attempted. Non-integer
        products will be rounded down to the nearest integer.
    excluded_edges : Set[Tuple[int, int]]
        Specific edges which should never be created by the network randomization
    seed : int
        Random seed that will be passed to the C++ Mersenne Twister 19937 random
        number generator.
    max_malloc : int (`unsigned long long int` in C)
        The maximum amount of memory to be allocated using `malloc` when making
        a bitset to hold edges. An uncompressed bitset is implemented for
        holding edges that is significantly faster than alternatives. However,
        it is memory-inefficient and will not be used if more memory is required
        than `max_malloc`. Above the threshold, a Roaring bitset will be used.

    Returns
    -------
    new_edges : List[Tuple[int, int]]
        Edge list of a permutation of the network given as `edge_list`
    stats : Dict[str, int]
        Information about the permutation performed. Gives the following information:
        `swap_attempts` - number of attempted swaps
        `same_edge` - number of swaps rejected because one edge was chosen twice
        `self_loop` - number of swaps rejected because new edge is a self-loop
        'duplicate` - number of swaps rejected because new edge already exists
        `undir_duplicate` - number of swaps rejected because the network is
            undirected and the reverse of the new edge already exists
        `excluded` - number of swaps rejected because new edge was among excluded
    """
    if len(edge_list) != len(set(edge_list)):
        raise ValueError("Edge list contained duplicate edges.")

    # Number of attempted XSwap swaps
    num_swaps = int(multiplier * len(edge_list))

    # Compute the maximum node ID (for creating the bitset)
    max_id = max(map(max, edge_list))

    new_edges, stats = xswap._xswap_backend._xswap(
        edge_list, list(excluded_edges), max_id, allow_self_loops,
        allow_antiparallel, num_swaps, seed, max_malloc)

    return new_edges, stats
