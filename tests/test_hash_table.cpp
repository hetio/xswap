#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include "../xswap/src/xswap.h"

void handle_eptr(std::exception_ptr eptr) {
    try {
        if (eptr) {
            std::rethrow_exception(eptr);
        }
    } catch(const std::exception& e) {
        std::cout << "Unexpected exception while attempting bad element access " << e.what() << "\n";
    }
}

bool test_add(EdgeHashTable edges_set) {
    int edge_to_add[2] = {1, 1};
    edges_set.add(edge_to_add);
    int** fake_edges = (int**)malloc(sizeof(int*) * 16);
    int counter = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            fake_edges[counter] = (int*)malloc(sizeof(int) * 2);
            fake_edges[counter][0] = i;
            fake_edges[counter][1] = j;
            counter += 1;
        }
    }
    bool correctly_contains = edges_set.contains(edge_to_add);
    int num_incorrect = 0;
    for (int i = 0; i < 16; i++) {
        bool incorrectly_contains = edges_set.contains(fake_edges[i]);
        bool was_added = (fake_edges[i][0] == edge_to_add[0] && fake_edges[i][1] == edge_to_add[1]);
        if (incorrectly_contains and !was_added) {
            num_incorrect += 1;
            std::printf("Incorrectly contained: (%d, %d)\n", fake_edges[i][0], fake_edges[i][1]);
        }
    }
    free(fake_edges);
    if (num_incorrect == 0 && correctly_contains == true) {
        return true;
    } else {
        return false;
    }
}

bool test_remove(EdgeHashTable edges_set) {
    int edge_to_add[2] = {1, 1};
    edges_set.add(edge_to_add);
    bool was_added = edges_set.contains(edge_to_add);
    edges_set.remove(edge_to_add);
    bool was_removed = !edges_set.contains(edge_to_add);
    bool passed = was_added && was_removed;
    if (!was_added)
        std::printf("Did not add edge properly");
    if (!was_removed)
        std::printf("Did not remove edge properly");
    return passed;
}

bool test_oob_insert(EdgeHashTable edges_set) {

    int edge_to_add[2] = {4, 4};
    std::exception_ptr eptr;
    try {
        edges_set.add(edge_to_add);
    } catch(std::out_of_range) {
        return true;
    } catch(...) {
        eptr = std::current_exception();
        handle_eptr(eptr);
        return true;
    }
    std::printf("No exception on OOB insert\n");
    return false;
}

bool test_oob_access(EdgeHashTable edges_set) {
    int edge_to_access[2] = {4, 4};
    std::exception_ptr eptr;
    try {
        edges_set.add(edge_to_access);
    } catch(std::out_of_range) {
        return true;
    } catch(...) {
        eptr = std::current_exception();
        handle_eptr(eptr);
        return true;
    }
    std::printf("No exception on OOB access\n");
    return false;
}

bool test_oob_remove(EdgeHashTable edges_set) {
    int edge_to_access[2] = {4, 4};
    std::exception_ptr eptr;
    try {
        edges_set.add(edge_to_access);
    } catch(std::out_of_range) {
        return true;
    } catch(...) {
        eptr = std::current_exception();
        handle_eptr(eptr);
        return true;
    }
    std::printf("No exception on OOB removal\n");
    return false;
}

bool test_remove_nonexistent(EdgeHashTable edges_set) {
    int edge_to_access[2] = {2, 2};
    std::exception_ptr eptr;
    try {
        edges_set.remove(edge_to_access);
    } catch(std::logic_error) {
        return true;
    } catch(...) {
        eptr = std::current_exception();
        handle_eptr(eptr);
        return true;
    }
    std::printf("No exception on removal of nonexisting element\n");
    return false;
}

bool test_insert_existing(EdgeHashTable edges_set) {
    int edge_to_access[2] = {2, 2};
    edges_set.add(edge_to_access);
    std::exception_ptr eptr;
    try {
        edges_set.add(edge_to_access);
    } catch(std::logic_error) {
        return true;
    } catch(...) {
        eptr = std::current_exception();
        handle_eptr(eptr);
        return true;
    }
    std::printf("No exception on addition of existing element\n");
    return false;
}

main(int argc, char const *argv[]) {
    int num_tests = 7;
    bool test_passed[num_tests];

    EdgeHashTable edges_set = EdgeHashTable(3, 3);
    test_passed[0] = test_add(edges_set);
    edges_set = EdgeHashTable(3, 3);  // Reset so functions don't interfere
    test_passed[1] = test_remove(edges_set);
    test_passed[2] = test_oob_insert(edges_set);
    test_passed[3] = test_oob_access(edges_set);
    test_passed[4] = test_oob_remove(edges_set);
    edges_set = EdgeHashTable(3, 3);
    test_passed[5] = test_remove_nonexistent(edges_set);
    edges_set = EdgeHashTable(3, 3);
    test_passed[6] = test_insert_existing(edges_set);

    bool all_tests_passed = true;
    for (int i = 0; i < num_tests; i++) {
        all_tests_passed &= test_passed[i];
    }

    if (all_tests_passed) {
        std::printf("All tests passed\n");
        return 0;
    } else {
        std::printf("Test failure\n");
        return 1;
    }
    edges_set.free_table();
}
