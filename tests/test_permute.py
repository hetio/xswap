import tempfile

import pytest
import requests

import xswap


@pytest.mark.parametrize('edges,permutable', [
    ([(0, 0), (1, 1), (1, 2), (2, 3)], True),
    ([(0, 0)], False),
])
def test_xswap_changes_edges(edges, permutable):
    """
    Check that XSwap returns a different set of edges than the ones given if the edges
    are permutable. Check that XSwap does not modify edges in place.
    """
    edges_copy = edges.copy()
    new_edges, stats = xswap.permute_edge_list(
        edges, allow_self_loops=True, allow_antiparallel=True)
    assert edges == edges_copy
    if permutable:
        assert new_edges != edges
    else:
        assert new_edges == edges


def test_roaring_warning():
    """
    Check that a warning is given when using the much slower but far more general
    Roaring bitset rather than the faster fully uncompressed bitset.
    """
    edges_url = "https://github.com/greenelab/xswap/raw/{}/{}".format(
        "8c31b4cbdbbf2cfa5018b1277bbd0e9f6263e573", "graphs/GiG_edges_reduced.txt")
    response = requests.get(edges_url)
    with tempfile.NamedTemporaryFile() as tf:
        tf.write(response.content)
        edges = xswap.preprocessing.load_processed_edges(tf.name)

    with pytest.warns(None):
        permuted_edges, stats = xswap.permute_edge_list(edges, allow_self_loops=True,
            allow_antiparallel=False, multiplier=0.1, seed=0, max_malloc=4000000000)

    with pytest.warns(RuntimeWarning, match="Using Roaring bitset because of the large number of edges."):
        permuted_edges, stats = xswap.permute_edge_list(edges, allow_self_loops=True,
            allow_antiparallel=False, multiplier=0.1, seed=0, max_malloc=10)
