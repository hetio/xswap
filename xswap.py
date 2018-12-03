import subprocess

# import numpy
# import scipy.sparse


# def matrix_to_edges(matrix):
#     sp = scipy.sparse.coo_matrix(matrix)
#     edges = list(zip(sp.row, sp.col))
#     return edges


def write_edges(edges, pipe):
    for edge in edges:
        b = bytes(f'{edge[0]} {edge[1]}\n', 'utf-8')
        pipe.stdin.write(b)
    pipe.stdin.flush()


def read_edges(pipe):
    edges = list()
    while True:
        next_line = pipe.stdout.readline().strip().decode("utf-8")
        source, target = [int(i) for i in next_line.split(" ")]
        if (source, target) == (-1, -1):
            return edges
        edges.append((source, target))


def permute_pair_list(pair_list, directed=False, multiplier=10, excluded_pair_set=set(), seed=0, log=False):
    num_swaps = int(multiplier * len(pair_list))
    d = 1 if directed else 0
    max_source = max([int(i[0]) for i in pair_list])
    max_target = max([int(i[1]) for i in pair_list])
    # TODO: Add static path reference to the xswap binary.
    command = f'./xswap {len(pair_list)} {len(excluded_pair_set)} {num_swaps} {seed} {max_source} {max_target} {d}'
    p = subprocess.Popen([command], shell=True,
                         stdout=subprocess.PIPE, stdin=subprocess.PIPE)
    # Send edges
    write_edges(pair_list, p)

    # Send excluded edges
    write_edges(excluded_pair_set, p)

    # Read back edges from xswap.cpp's stdout stream
    new_edges = read_edges(p)
    assert len(new_edges) == len(pair_list)

    info_dict = dict()
    for i in ["same_edge", "self_loop", "duplicate", "undir_duplicate", "excluded"]:
        info_dict[i] = int(p.stdout.readline().strip().decode("utf-8"))
    return new_edges, info_dict


# def permute_matrix(adjacency_matrix, directed=False, multiplier=10, excluded_pair_set=set(), seed=0, log=False):
#     edges = matrix_to_edges(adjacency_matrix)

#     new_edges, info_dict = permute_pair_list(edges, directed=directed, multiplier=multiplier,
#                                              excluded_pair_set=excluded_pair_set, seed=seed, log=log)

#     row_inds, col_inds = list(zip(*new_edges))
#     sparse_permuted = scipy.sparse.coo_matrix(
#         (numpy.ones(len(edges), int), (row_inds, col_inds)),
#         shape=adjacency_matrix.shape)
#     return sparse_permuted.toarray(), info_dict
