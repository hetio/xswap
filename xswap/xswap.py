from typing import List, Set, Tuple

import xswap._xswap_backend


def permute_edge_list(edge_list: List[Tuple[int, int]], directed: bool = False,
                      bipartite: bool = True, multiplier: float = 10,
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
    directed : bool
        Whether (0, 1) and (1, 0) should be treated as different edges. `directed`
        should be `False` when (0, 1) is equivalent to (1, 0). In this case, the
        edges will be pruned so that only one copy of every edge is present.
        After XSwap is complete, then, copies of every new edge will be made so
        that the permuted edges have the same properties as the original edges.
    bipartite : bool
        Whether the set of source nodes is the same as the set of target nodes.
        If (0, 0) is a self loop, then set `bipartite=False`. To permute a
        bipartite graph, use only the edges going from one set of nodes to the
        other. That is, only use the edges found in the biadjacency matrix. For
        a more detailed explanation, see README.md.
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
    if directed and bipartite:
        raise NotImplementedError("Directed bipartite graphs are not supported.")
    if not directed and not bipartite:
        edge_list = [edge for edge in edge_list if edge[0] < edge[1]]

    max_source = max([i[0] for i in edge_list])
    max_target = max([i[1] for i in edge_list])
    num_swaps = int(multiplier * len(edge_list))
    new_edges, stats = xswap._xswap_backend._xswap(
        edge_list, list(excluded_edges), max_source, max_target, directed,
        num_swaps, seed)

    if not directed and not bipartite:
        reversed_edges = [(edge[1], edge[0]) for edge in new_edges]
        new_edges.extend(reversed_edges)

    return new_edges, stats
