import xswap


if __name__ == '__main__':
    edges = [(0, 0), (1, 1), (1, 2), (2, 3)]
    new_edges, stats = xswap.permute_edge_list(
        edges, directed=False, multiplier=10, excluded_pair_set=set(), seed=0, log=False)
    print(new_edges)
