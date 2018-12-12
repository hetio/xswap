from typing import List, Set, Tuple

import xswap._xswap_backend


def permute_edge_list(edge_list: List[Tuple[int, int]], allow_self_loops: bool = False,
                      allow_antiparallel: bool = False, multiplier: float = 10,
                      excluded_edges: Set[Tuple[int, int]] = set(), seed: int = 0):
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
    """
    max_source = max([i[0] for i in edge_list])
    max_target = max([i[1] for i in edge_list])
    num_swaps = int(multiplier * len(edge_list))
    new_edges, stats = xswap._xswap_backend._xswap(
        edge_list, list(excluded_edges), max_source, max_target, allow_self_loops,
        allow_antiparallel, num_swaps, seed)

    return new_edges, stats
