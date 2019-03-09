import os
import time

import requests

import xswap

test_directory = os.path.dirname(os.path.realpath(__file__)) + '/'


def load_edges():
    edges_url = "https://github.com/greenelab/xswap/raw/{}/{}".format(
        "8c31b4cbdbbf2cfa5018b1277bbd0e9f6263e573", "graphs/GiG_edges_reduced.txt")
    response = requests.get(edges_url)
    edges = list()
    for edge in response.iter_lines():
        edge = str(edge, 'utf-8')
        source, target = edge.split(',')
        edges.append((int(source), int(target)))
    return edges


def test_time():
    edges = load_edges()
    t1 = time.time()
    new_edges, stats = xswap.permute_edge_list(edges)
    t2 = time.time()
    time_diff = t2 - t1
    print("{:.4f}  seconds elapsed.".format(time_diff))
    assert edges != new_edges
    assert time_diff < 5

    num_repeats = 0
    old_set = set(edges)
    new_set = set(new_edges)
    for edge in old_set:
        if edge in new_set:
            num_repeats += 1
    p_unch = num_repeats / len(edges)
    with open(test_directory + 'permutation_stats.txt', 'w') as f:
        f.write('Runtime: {:.3f} sec. {:.3f} percent unchanged of {} total edges after '
                '{} swap attempts\n'.format(time_diff, p_unch, len(edges), 10*len(edges)))
