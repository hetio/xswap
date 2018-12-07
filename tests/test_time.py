import os
import time

import xswap

test_directory = os.path.dirname(os.path.realpath(__file__)) + '/'


def load_edges():
    edges_file = test_directory + 'GiG_edges.txt'
    with open(edges_file) as f:
        string_edges = f.readlines()
    edges = list()
    for edge in string_edges:
        source, target = edge.split(',')
        edges.append((int(source), int(target)))
    return edges


def test_time():
    edges = load_edges()
    t1 = time.time()
    new_edges, stats = xswap.permute_edge_list(edges)
    t2 = time.time()
    print(str(t2 - t1) + "  seconds elapsed.")
    assert edges != new_edges
    assert t2 - t1 < 5

    num_repeats = 0
    old_set = set(edges)
    new_set = set(new_edges)
    for edge in old_set:
        if edge in new_set:
            num_repeats += 1
    p_unch = num_repeats / len(edges)
    with open(test_directory + 'permutation_stats.txt', 'w') as f:
        f.write('{0:.3f} percent unchanged of {1} total edges after '
                '{2} swap attempts\n'.format(p_unch, len(edges), 10*len(edges)))
