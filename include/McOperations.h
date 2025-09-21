#pragma once

#include "Partition.h"
#include "Graph.h"

class McOperations {
public:
    // find all possible Mc operations (merge connected blocks)
    static int findAllMcOperations(const Partition& partition, const Graph& graph, 
                                   int* block1_array, int* block2_array);
    
    // perform Mc operation - merge two connected blocks
    static Partition performMcOperation(const Partition& partition, int block1, int block2);
    
    // choose random Mc operation from available options
    static Partition performRandomMcOperation(const Partition& partition, const Graph& graph);
};