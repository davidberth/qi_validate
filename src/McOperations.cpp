#include "../include/McOperations.h"
#include <cstdlib>
#include <ctime>
#include <cstdio>

int McOperations::findAllMcOperations(const Partition& partition, const Graph& graph, 
                                      int* block1_array, int* block2_array) {
    int count = 0;
    int num_blocks = partition.getNumBlocks();
    
    // Get all used block labels
    bool used_labels[Partition::MAX_VERTICES] = {false};
    for (int v = 0; v < partition.getNumVertices(); v++) {
        used_labels[partition.getLabel(v)] = true;
    }
    
    // Find all pairs of blocks that are connected in quotient graph
    for (int b1 = 0; b1 < Partition::MAX_VERTICES; b1++) {
        if (!used_labels[b1]) continue;
        
        for (int b2 = b1 + 1; b2 < Partition::MAX_VERTICES; b2++) {
            if (!used_labels[b2]) continue;
            
            if (partition.areBlocksConnectedInQuotient(graph, b1, b2)) {
                block1_array[count] = b1;
                block2_array[count] = b2;
                count++;
            }
        }
    }
    
    return count;
}

Partition McOperations::performMcOperation(const Partition& partition, int block1, int block2) {
    Partition result = partition;
    result.mergeBlocks(block1, block2);
    return result;
}

Partition McOperations::performRandomMcOperation(const Partition& partition, const Graph& graph) {
    int block1_array[Partition::MAX_VERTICES];
    int block2_array[Partition::MAX_VERTICES];
    
    int num_operations = findAllMcOperations(partition, graph, block1_array, block2_array);
    
    if (num_operations == 0) {
        // No Mc operations available, return original partition
        return partition;
    }
    
    // Choose random operation
    static bool seeded = false;
    if (!seeded) {
        srand(time(nullptr));
        seeded = true;
    }
    
    int chosen_index = rand() % num_operations;
    int block1 = block1_array[chosen_index];
    int block2 = block2_array[chosen_index];
    
    if (VERBOSE_MC_OPERATIONS) {
        printf("Performing Mc operation: merging block %d with block %d\n", block1, block2);
    }
    
    return performMcOperation(partition, block1, block2);
}