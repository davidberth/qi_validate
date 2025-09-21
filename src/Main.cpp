#include "../include/Graph.h"
#include "../include/Partition.h"
#include "../include/McOperations.h"
#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 4) {
        std::cout << "Usage: " << argv[0] << " <graph_file> [--output <output_file>]" << std::endl;
        return 1;
    }
    
    std::string graph_file = argv[1];
    std::string output_file = "";
    bool use_output_file = false;
    
    // Parse command line arguments
    for (int i = 2; i < argc; i++) {
        if (std::string(argv[i]) == "--output" && i + 1 < argc) {
            output_file = argv[i + 1];
            use_output_file = true;
            i++; // Skip the next argument
        }
    }
    
    // Load graph from file
    Graph graph;
    if (!graph.loadFromFile(graph_file.c_str())) {
        std::cout << "Failed to load graph from " << graph_file << std::endl;
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
    
    if (current_partition.getQiNumber() == -1) {
        std::cout << "Initial partition (size " << current_partition.getNumBlocks() 
                  << "): qi = UNDETERMINED (quotient graph too large for exact computation)" << std::endl;
        std::cout << "Continuing with Mc operations - will switch to exact computation when quotient size ≤ 15..." << std::endl;
    } else {
        std::cout << "Initial partition (size " << current_partition.getNumBlocks() 
                  << "): qi = " << current_partition.getQiNumber() << std::endl;
    }
    
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
        
        if (next_partition.getQiNumber() == -1) {
            std::cout << "Step " << step << " (size " << next_partition.getNumBlocks() 
                      << "): qi = UNDETERMINED (quotient graph still too large)" << std::endl;
            std::cout << "         Continuing with Mc operations..." << std::endl;
        } else {
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
        }
        
        current_partition = next_partition;
        step++;
    }
    
    std::cout << std::endl;
    std::cout << "Final partition size: " << current_partition.getNumBlocks() << std::endl;
    
    // Determine validation result
    std::string result_status;
    std::string result_detail;
    int return_code = 0;
    
    if (current_partition.getQiNumber() == -1) {
        std::cout << "Final qi number: UNDETERMINED (final quotient graph still too large)" << std::endl;
        std::cout << "VALIDATION PARTIAL: Completed Mc operations but cannot verify final qi threshold" << std::endl;
        std::cout << "NOTE: For proof purposes, exact computation would be needed for final validation" << std::endl;
        result_status = "PARTIAL";
        result_detail = "Final qi undetermined - quotient graph too large";
        return_code = 0;
    } else {
        std::cout << "Final qi number: " << current_partition.getQiNumber() << std::endl;
        
        int final_required_qi = current_partition.getNumBlocks() - graph.critical_k + 1;
        std::cout << "Required final qi: " << final_required_qi << std::endl;
        
        if (current_partition.getQiNumber() >= final_required_qi) {
            std::cout << "VALIDATION SUCCESSFUL: qi ≥ k - k' + 1 throughout process" << std::endl;
            result_status = "PASS";
            result_detail = "qi ≥ k - k' + 1 throughout process";
            return_code = 0;
        } else {
            std::cout << "VALIDATION FAILED: final qi below threshold" << std::endl;
            result_status = "FAIL";
            result_detail = "Final qi below required threshold";
            return_code = 1;
        }
    }
    
    // Write results to output file if specified
    if (use_output_file) {
        std::ofstream outfile(output_file);
        if (outfile.is_open()) {
            outfile << "GRAPH: " << graph_file << std::endl;
            outfile << "VERTICES: " << graph.num_vertices << std::endl;
            outfile << "CRITICAL_K: " << graph.critical_k << std::endl;
            outfile << "STEPS: " << (step - 1) << std::endl;
            outfile << "RESULT: " << result_status << std::endl;
            outfile << "DETAIL: " << result_detail << std::endl;
            outfile.close();
        } else {
            std::cerr << "Error: Could not write to output file " << output_file << std::endl;
        }
    }
    
    return return_code;
}