import csv


def load_str_edges(filename, node_delim=',', edge_delim='\n'):
    """
    Load edges from file into memory. Store edges as a list and store each edge
    as Tuple[str, str]. Used to load edges for preprocessing.
    """
    with open(filename, 'r', newline='') as f:
        reader = csv.reader(f, delimiter=node_delim, lineterminator=edge_delim)
        str_edges = [tuple(row) for row in reader if len(row) > 1]
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


def write_edges(filename, edges, node_delim=',', edge_delim='\n'):
    with open(filename, 'w', newline='') as f:
        writer = csv.writer(f, delimiter=node_delim, lineterminator=edge_delim)
        writer.writerows(edges)


def write_mapping(filename, mapping, delimiter=','):
    with open(filename, 'w', newline='') as f:
        writer = csv.writer(f, delimiter=delimiter)
        writer.writerow(['original', 'mapped'])
        for original, mapped in mapping.items():
            writer.writerow([original, mapped])


def _map_nodes_to_int(nodes):
    """
    Return a dict mapping a list of nodes to their sorted indices. Nodes should
    be a list of strings.

    Returns:
    --------
    Dict[str, int]
    """
    sorted_node_set = sorted(set(nodes))
    name_to_id = {name: i for i, name in enumerate(sorted_node_set)}
    return name_to_id


def _apply_map(edges, source_mapping, target_mapping):
    """
    Maps edges according to new node names specified by source and target maps.

    edges : List[Tuple[str, str]]
    source_mapping : Dict[str, int]
    target_mapping : Dict[str, int]
    """
    source_nodes = [edge[0] for edge in edges]
    target_nodes = [edge[1] for edge in edges]
    mapped_nodes = [
        map(source_mapping.get, source_nodes),
        map(target_mapping.get, target_nodes),
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
    source_nodes = [edge[0] for edge in edges]
    target_nodes = [edge[1] for edge in edges]

    # Two separate mappings to be used for source and target nodes
    if bipartite:
        source_map = _map_nodes_to_int(source_nodes)
        target_map = _map_nodes_to_int(target_nodes)

    # One single mapping to be used for both source and target nodes
    if not bipartite:
        combined_nodes = list(set(source_nodes + target_nodes))
        source_map = target_map = _map_nodes_to_int(combined_nodes)

    mapped_edges = _apply_map(edges, source_map, target_map)
    return (mapped_edges, source_map, target_map)
