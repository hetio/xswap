import numpy
import pandas
import pytest

import xswap


@pytest.mark.parametrize('edges,true_prior,num_swaps,shape', [
    ([(0, 0), (1, 1)], 0.5 * numpy.ones((2, 2)), 10000, (2, 2)),
    ([(0, 1), (1, 0)], 0.5 * numpy.ones((2, 2)), 10000, (2, 2)),
    ([(0, 0)], numpy.ones((1, 1)), 10, (1, 1)),
    ([(0, 1), (1, 2), (3, 4), (1, 0)], numpy.zeros((5, 5)), 0, (5, 5)),
    ([(0, 1), (1, 2), (3, 4), (1, 0)], numpy.zeros((4, 5)), 0, (4, 5)),
])
def test_prior_matrix(edges, true_prior, num_swaps, shape):
    """
    Check that `xswap.prior.compute_xswap_occurrence_matrix` is returning
    reasonable results for very small networks where the correct prior is obvious.
    """
    occurrence_matrix = xswap.prior.compute_xswap_occurrence_matrix(
        edges, n_permutations=num_swaps, shape=shape, allow_self_loops=True,
        allow_antiparallel=True)
    if num_swaps:
        edge_prior = (occurrence_matrix / num_swaps).toarray()
    else:
        edge_prior = occurrence_matrix.toarray()
    assert numpy.abs(edge_prior - true_prior).max() == pytest.approx(0, abs=0.01)


@pytest.mark.parametrize('edges,dtypes,source_degrees,target_degrees,shape,allow_antiparallel', [
    (
        [(0, 2), (0, 3), (1, 2), (2, 3), (3, 4)],
        {'id': numpy.uint16, 'edge': bool, 'degree': numpy.uint32, 'xswap_prior': float},
        {0: 2, 1: 1, 2: 3, 3: 3, 4: 1}, {0: 2, 1: 1, 2: 3, 3: 3, 4: 1}, (5, 5), False
    ),
    (
        [(0, 2), (0, 3), (1, 2), (2, 3), (3, 4)],
        {'id': numpy.int8, 'edge': int, 'degree': numpy.float, 'xswap_prior': numpy.float64},
        {0: 2, 1: 1, 2: 3, 3: 3, 4: 1}, {0: 2, 1: 1, 2: 3, 3: 3, 4: 1}, (5, 5), False
    ),
    (
        [(0, 2), (0, 3), (1, 2), (1, 3)],
        {'id': numpy.float16, 'edge': float, 'degree': float, 'xswap_prior': numpy.float32},
        {0: 2, 1: 2, 2: 0, 3: 0}, {0: 0, 1: 0, 2: 2, 3: 2}, (4, 4), True
    ),
])
def test_prior_dataframe(edges, dtypes, source_degrees, target_degrees, shape, allow_antiparallel):
    """
    Check that the `xswap.prior.compute_xswap_priors` performs correctly
    """
    prior_df = xswap.prior.compute_xswap_priors(edges, n_permutations=1000,
        shape=shape, allow_self_loops=False, allow_antiparallel=allow_antiparallel, dtypes=dtypes)

    assert isinstance(prior_df, pandas.DataFrame)
    assert list(prior_df.columns) == ['source_id', 'target_id', 'edge', 'source_degree',
                                      'target_degree', 'xswap_prior']
    assert dict(prior_df.dtypes) == {
        'source_id': dtypes['id'], 'target_id': dtypes['id'], 'edge': dtypes['edge'],
        'source_degree': dtypes['degree'], 'target_degree': dtypes['degree'],
        'xswap_prior': dtypes['xswap_prior']
    }

    assert prior_df.set_index('source_id')['source_degree'].to_dict() == source_degrees
    assert prior_df.set_index('target_id')['target_degree'].to_dict() == target_degrees

    # Ensure that all the edges are accounted for in the dataframe
    for edge in edges:
        assert prior_df.query('source_id == {} & target_id == {}'.format(*edge))['edge'].values[0]

    # Whether directed-ness is correctly propagated through the pipeline
    if allow_antiparallel:
        assert prior_df['edge'].sum() == len(edges)
    else:
        assert prior_df['edge'].sum() == len(edges) * 2
