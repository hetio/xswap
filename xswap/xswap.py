import xswap._xswap_backend


def permute_edge_list(edge_list, directed=False, multiplier=10, excluded_pair_set=set(), seed=0, log=False):
    max_source = max([i[0] for i in edge_list])
    max_target = max([i[1] for i in edge_list])
    num_swaps = int(10 * len(edge_list))
    return xswap._xswap_backend._xswap(edge_list, list(excluded_pair_set),
                                       max_source, max_target, directed,
                                       num_swaps, seed)
