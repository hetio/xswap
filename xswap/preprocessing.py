import pathlib as _pathlib


def load_str_edges(filename, node_delim=',', edge_delim='\n'):
    """
    Load edges from file into memory. Store edges as a list and store each edge
    as Tuple[str, str]. Used to load edges for preprocessing.
    """
    file_path = _pathlib.Path(filename)
    if not file_path.is_file():
        raise FileNotFoundError

    with open(file_path, 'r') as f:
        graph_string = f.read()
    str_edges = [
        edge.split(node_delim) for edge in graph_string.split(edge_delim) if edge != ''
    ]
    return str_edges


def load_processed_edges(filename):
    """
    Load processed edges from a file. Processed means that edges are guaranteed
    to be integers ranging from zero to the number of unique nodes.
    """
    str_edges = load_str_edges(filename)
    edges = [
        (int(edge[0]), int(edge[1])) for edge in str_edges
    ]
    return edges


def _map_nodes_to_int(nodes):
    """
    Return a dict mapping a list of nodes to their sorted indices. Nodes should
    be a list of strings.

    Returns:
    --------
    Dict[str, int]
    """
    name_to_id = dict()
    sorted_node_set = sorted(list(set(nodes)))
    for i, name in enumerate(sorted_node_set):
        name_to_id[name] = i
    return name_to_id


def _apply_map(edges, mapping):
    """
    Maps edges according to a node mapping. If `mapping` contains two dicts,
    `_apply_map` maps source and target nodes separately. Else, if `mapping`
    contains only one dict, then source and target nodes are mapped according
    to the same mapping.
    """
    source_nodes = [edge[0] for edge in edges]
    target_nodes = [edge[1] for edge in edges]

    if len(mapping) == 1:
        mapped_nodes = [
            map(mapping[0].get, source_nodes),
            map(mapping[0].get, target_nodes),
        ]
    else:
        mapped_nodes = [
            map(mapping[0].get, source_nodes),
            map(mapping[1].get, target_nodes),
        ]
    return list(zip(*mapped_nodes))


def map_str_edges(edges, bipartite):
    """
    Maps a list of edge tuples containing strings to a minimal set of
    integer edges.

    edges : List[Tuple[str, str]]
    bipartite : bool
        Whether to map source and target nodes using the same mapping.
        For example, an edge like ('1', '1') may refer to a connection between
        separate nodes, or it may be a self-loop. If `bipartite=True`, the
        edge would be mapped like (0, 1), where the new node ids reflect the fact
        that the same names do not indicate the same nodes. To ensure that names
        are consistently mapped between source and target, put `bipartite=False`.

    Returns:
    --------
    Tuple[List[Tuple[int, int]], Dict[int, str]]

    Example:
    --------
    >>> map_str_edges([('a', 'b'), ('b', 'c')], bipartite=False)

    ([(0, 1), (1, 2)], {0: 'a', 1: 'b', 2: 'c'})
    """
    # Two separate mappings to be used for source and target nodes
    if bipartite:
        mappings = [_map_nodes_to_int(nodes) for nodes in zip(*edges)]

    # One single mapping to be used for both source and target nodes
    if not bipartite:
        nodes = list(set([node for nodelist in zip(*edges) for node in nodelist]))
        mappings = [_map_nodes_to_int(nodes)]

    mapped_edges = _apply_map(edges, mappings)
    return (mapped_edges, mappings)
