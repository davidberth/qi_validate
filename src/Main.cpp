#include "../include/Graph.h"
#include "../include/Partition.h"
#include "../include/McOperations.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <graph_file>" << std::endl;
        return 1;
    }
    
    // Load graph from file
    Graph graph;
    if (!graph.loadFromFile(argv[1])) {
        std::cout << "Failed to load graph from " << argv[1] << std::endl;
        return 1;
    }
    
    std::cout << "Loaded graph with " << graph.num_vertices << " vertices, k'=" << graph.critical_k << std::endl;
    
    // Create initial partition P* (each vertex in its own block)
    int initial_partition[Partition::MAX_VERTICES];
    for (int i = 0; i < graph.num_vertices; i++) {
        initial_partition[i] = i;
    }
    
    Partition current_partition(initial_partition, graph.num_vertices);
    
    std::cout << "Starting qi validation:" << std::endl;
    std::cout << "Target partition size: " << graph.critical_k << std::endl;
    std::cout << std::endl;
    
    // Calculate and output initial qi number (with early stopping)
    int initial_required_qi = current_partition.getNumBlocks() - graph.critical_k + 1;
    current_partition.calculateQiNumber(graph, initial_required_qi);
    std::cout << "Initial partition (size " << current_partition.getNumBlocks() 
              << "): qi = " << current_partition.getQiNumber() << std::endl;
    
    int step = 1;
    
    // Perform Mc operations until we reach target size
    while (current_partition.getNumBlocks() > graph.critical_k) {
        Partition next_partition = McOperations::performRandomMcOperation(current_partition, graph);
        
        // Check if we made progress
        if (next_partition.getNumBlocks() == current_partition.getNumBlocks()) {
            std::cout << "No more Mc operations available. Stopping at size " 
                      << current_partition.getNumBlocks() << std::endl;
            break;
        }
        
        // Use early stopping - only need qi >= threshold
        int required_qi = next_partition.getNumBlocks() - graph.critical_k + 1;
        next_partition.calculateQiNumber(graph, required_qi);
        
        std::cout << "Step " << step << " (size " << next_partition.getNumBlocks() 
                  << "): qi = " << next_partition.getQiNumber();
        
        // Check if qi meets threshold (qi >= k - k' + 1)
        std::cout << " (qi >= " << required_qi << " required)";
        
        if (next_partition.getQiNumber() < required_qi) {
            std::cout << " ERROR: qi below required threshold!" << std::endl;
            return 1;
        } else {
            std::cout << " PASS" << std::endl;
        }
        
        current_partition = next_partition;
        step++;
    }
    
    std::cout << std::endl;
    std::cout << "Final partition size: " << current_partition.getNumBlocks() << std::endl;
    std::cout << "Final qi number: " << current_partition.getQiNumber() << std::endl;
    
    int final_required_qi = current_partition.getNumBlocks() - graph.critical_k + 1;
    std::cout << "Required final qi: " << final_required_qi << std::endl;
    
    if (current_partition.getQiNumber() >= final_required_qi) {
        std::cout << "VALIDATION SUCCESSFUL: qi > k - k' throughout process" << std::endl;
    } else {
        std::cout << "VALIDATION FAILED: final qi below threshold" << std::endl;
        return 1;
    }
    
    return 0;
}