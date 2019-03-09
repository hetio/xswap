#include <iostream>
#include "../xswap/src/xswap.h"


main(int argc, char const *argv[])
{
    int counter, incorrect_contains, incorrect_doesnt_contain;

    // Create real edges to be added to the Roaring set
    int** real_edges = (int**)malloc(sizeof(int*) * 16);
    counter = 0;
    for (int i = 4; i < 8; i++) {
        for (int j = 4; j < 8; j++) {
            real_edges[counter] = (int*)malloc(sizeof(int) * 2);
            real_edges[counter][0] = i;
            real_edges[counter][1] = j;
            counter += 1;
        }
    }

    Edges edges;
    edges.edge_array = real_edges;
    edges.num_edges = 16;
    RoaringBitSet edges_set = RoaringBitSet(edges);

    // Check that edges added at the creation of the set are contained
    incorrect_doesnt_contain = 0;
    for (int i = 4; i < 8; i++) {
        for (int j = 4; j < 8; j++) {
            int edge[2] = {i, j};
            if (!edges_set.contains(edge)) {
                incorrect_doesnt_contain += 1;
            }
        }
    }

    // Create fake edges and check that they are not in the set
    counter = 0;
    incorrect_contains = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int fake_edge[2] = {i, j};
            // Check that this edge is not in the set
            if (edges_set.contains(fake_edge)) {
                incorrect_contains += 1;
            }
            // Add the edge and check that it was added
            edges_set.add(fake_edge);
            if (!edges_set.contains(fake_edge)) {
                incorrect_doesnt_contain += 1;
            }
            // Remove the edge and check that it is removed
            edges_set.remove(fake_edge);
            if (edges_set.contains(fake_edge)) {
                incorrect_contains += 1;
            }
            counter += 1;
        }
    }

    free(real_edges);
    if (incorrect_contains == 0 && incorrect_doesnt_contain == 0) {
        std::cout << "All tests passed" << "\n";
        return 0;
    } else {
        std::cout << "Tests failed " << incorrect_contains << " " << incorrect_doesnt_contain << "\n";
        return 1;
    }
}
