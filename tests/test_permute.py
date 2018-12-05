import pytest

import xswap


@pytest.mark.parametrize('edges,permutable', [
    ([(0, 0), (1, 1), (1, 2), (2, 3)], True),
    ([(0, 0)], False),
    # (),
])
def test_xswap_changes_edges(edges, permutable):
    """
    Check that XSwap returns a different set of edges than the ones given if the edges
    are permutable. Check that XSwap does not modify edges in place.
    """
    edges_copy = edges.copy()
    new_edges, stats = xswap.permute_edge_list(
        edges, directed=False, multiplier=10, excluded_pair_set=set(), seed=0, log=False)
    assert edges == edges_copy
    if permutable:
        assert new_edges != edges
    else:
        assert new_edges == edges
