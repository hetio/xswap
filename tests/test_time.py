import os
import time

import requests

import xswap

test_directory = os.path.dirname(os.path.realpath(__file__)) + '/'


def load_edges():
    edges_url = "https://github.com/greenelab/xswap/raw/{}/{}".format(
        "51cad9392880db08a4c5870f08a670c52877e78f", "graphs/GiG_edges.txt")
    response = requests.get(edges_url)
    edges = list()
    for edge in response.iter_lines():
        edge = str(edge, 'utf-8')
        source, target = edge.split(',')
        edges.append((int(source), int(target)))
    return edges


def test_time():
    print("Loading edges")
    edges = load_edges()
    print("Edges are loaded. Starting permutation.")
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
        f.write('Runtime: {:.3f} sec. {:.3f} percent unchanged of {} total edges after '
                '{} swap attempts\n'.format(t2 - t1, p_unch, len(edges), 10*len(edges)))
