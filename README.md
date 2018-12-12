# xswap

## Usage examples

## Choice of parameters

### `directed` and `bipartite`

The `bipartite` argument determines the meaning of a node's value.
A bipartite graph is a graph in which nodes can be divided into disjoint sets with connections exclusively between sets.
For example, consider the graph shown in the figure below:

<a href="#bipartite_graph"><img src="docs/img/bipartite_graph.png" alt="Image of bipartite graph" width="50%" id="bipartite_graph"></a>

The adjacency matrix corresponding to a bipartite graph can be broken into four blocks.

<a href="#bipartite_adj"><img src="https://latex.codecogs.com/gif.latex?A&=\begin{bmatrix}0&B\\B^T&0\end{bmatrix}" title="Bipartite graph adjacency matrix" id="bipartite_adj" /></a>

The two diagonal blocks are entirely zero and the two off-diagonal blocks are the biadjacency matrix and its transpose.

<a href="#biadj"><img src="https://latex.codecogs.com/gif.latex?B&=\begin{bmatrix}1&0&0&0\\0&1&1&0\\0&0&0&1\end{bmatrix}" title="Bipartite graph biadjacency matrix" id="biadj" /></a>

The biadjacency matrix is, in general, non-square.
This means that the edge (0, 0) is not a self loop, as a 0 in the first position refers to a different node than a 0 in the second position.
