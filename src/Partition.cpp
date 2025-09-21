#include "../include/Partition.h"
#include "../include/Graph.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <vector>
#include <exception>
#include <map>
#include <string>
#include "dsatur.hpp"

Partition::Partition() : num_vertices_(0), qi_calculated_(false) {
    std::fill(partition_, partition_ + MAX_VERTICES, 0);
}

Partition::Partition(const int* partition_array, int num_vertices) 
    : num_vertices_(num_vertices), qi_calculated_(false) {
    assert(num_vertices <= MAX_VERTICES);
    std::copy(partition_array, partition_array + num_vertices, partition_);
    std::fill(partition_ + num_vertices, partition_ + MAX_VERTICES, 0);
}


Partition::Partition(const Partition& other) {
    copyFrom(other);
}

Partition& Partition::operator=(const Partition& other) {
    if (this != &other) {
        copyFrom(other);
    }
    return *this;
}

void Partition::copyFrom(const Partition& other) {
    num_vertices_ = other.num_vertices_;
    std::copy(other.partition_, other.partition_ + MAX_VERTICES, partition_);
    qi_calculated_ = other.qi_calculated_;
    qi_number_ = other.qi_number_;
}

void Partition::invalidateQiCache() {
    qi_calculated_ = false;
}

int Partition::getLabel(int vertex) const {
    assert(vertex >= 0 && vertex < num_vertices_);
    return partition_[vertex];
}

void Partition::setLabel(int vertex, int label) {
    assert(vertex >= 0 && vertex < num_vertices_);
    if (partition_[vertex] != label) {
        partition_[vertex] = label;
        invalidateQiCache();
    }
}

int Partition::getNumBlocks() const {
    bool used[MAX_VERTICES] = {false};
    for (int v = 0; v < num_vertices_; v++) {
        used[partition_[v]] = true;
    }
    int count = 0;
    for (int i = 0; i < MAX_VERTICES; i++) {
        if (used[i]) count++;
    }
    return count;
}

void Partition::getBlockVertices(int block_label, int* vertices, int& count) const {
    count = 0;
    for (int v = 0; v < num_vertices_; v++) {
        if (partition_[v] == block_label) {
            vertices[count++] = v;
        }
    }
}

void Partition::calculateQiNumber(const Graph& graph) {
    if (qi_calculated_) return;
    if (VERBOSE_QI_DEBUG) {
        printf("Entering calculateQiNumber() with %d blocks...\n", getNumBlocks());
        fflush(stdout);
    }
    qi_number_ = calculateQiNumberInternal(graph);
    qi_calculated_ = true;
}

void Partition::calculateQiNumber(const Graph& graph, int min_required_qi) {
    if (qi_calculated_) return;
    if (VERBOSE_QI_DEBUG) {
        printf("Entering calculateQiNumber() with early stopping: %d blocks, min_required=%d...\n", getNumBlocks(), min_required_qi);
        fflush(stdout);
    }
    qi_number_ = calculateQiNumberInternal(graph, min_required_qi);
    qi_calculated_ = true;
}

bool Partition::areBlocksConnectedInQuotient(const Graph& graph, int block1, int block2) const {
    if (block1 == block2) return false;
    
    const int* adj_matrix = graph.getAdjMatrix();
    
    // Check if any vertex in block1 is connected to any vertex in block2
    for (int u = 0; u < num_vertices_; u++) {
        if (partition_[u] != block1) continue;
        for (int v = 0; v < num_vertices_; v++) {
            if (partition_[v] != block2) continue;
            if (adj_matrix[u * num_vertices_ + v]) {
                return true;
            }
        }
    }
    return false;
}

void Partition::mergeBlocks(int block1, int block2) {
    if (block1 == block2) return;
    
    // Merge block2 into block1
    for (int v = 0; v < num_vertices_; v++) {
        if (partition_[v] == block2) {
            partition_[v] = block1;
        }
    }
    invalidateQiCache();
}

int Partition::calculateQiNumberInternal(const Graph& graph) const {
    int k = getNumBlocks();
    
    if (VERBOSE_QI_DEBUG) {
        printf("=== QI CALCULATION (k=%d) ===\n", k);
        printf("Algorithm selection: %s\n", (k <= 16) ? "EXACT (exhaustive)" : "FAST (chromatic number)");
        fflush(stdout);
    }
    
    if (k == 1) return 0; // Single block is q-complete
    
    // Build quotient graph for SmallGraph
    // Get list of used block labels (may not be consecutive)
    int block_labels[MAX_VERTICES];
    int label_count = 0;
    bool seen[MAX_VERTICES] = {false};
    
    for (int v = 0; v < num_vertices_; v++) {
        int label = partition_[v];
        if (!seen[label]) {
            seen[label] = true;
            block_labels[label_count++] = label;
        }
    }
    
    // Create mapping from block labels to consecutive indices for SmallGraph
    int label_to_index[MAX_VERTICES];
    for (int i = 0; i < MAX_VERTICES; i++) {
        label_to_index[i] = -1;
    }
    for (int i = 0; i < label_count; i++) {
        label_to_index[block_labels[i]] = i;
    }
    
    // Build quotient graph adjacency matrix with consecutive indices
    std::vector<std::vector<bool>> quotient_adj(label_count, std::vector<bool>(label_count, false));
    
    // Check all edges in original graph to build quotient graph
    const int* adj_matrix = graph.getAdjMatrix();
    
    if (VERBOSE_QI_DEBUG) {
        printf("\n=== QI CALCULATION DEBUG (ChromaticNumber) ===\n");
        printf("Partition blocks (%d total):\n", label_count);
        for (int i = 0; i < label_count; i++) {
            int label = block_labels[i];
            printf("  Block %d: vertices ", label);
            for (int v = 0; v < num_vertices_; v++) {
                if (partition_[v] == label) {
                    printf("%d ", v);
                }
            }
            printf("\n");
        }
    }
    
    for (int u = 0; u < num_vertices_; u++) {
        for (int v = u + 1; v < num_vertices_; v++) {
            if (adj_matrix[u * num_vertices_ + v] == 1) {
                int block_u = partition_[u];
                int block_v = partition_[v];
                
                if (block_u != block_v) {
                    int idx_u = label_to_index[block_u];
                    int idx_v = label_to_index[block_v];
                    quotient_adj[idx_u][idx_v] = true;
                    quotient_adj[idx_v][idx_u] = true;
                }
            }
        }
    }
    
    if (VERBOSE_QI_DEBUG) {
        printf("Quotient graph edges:\n");
        int edge_count = 0;
        for (int i = 0; i < label_count; i++) {
            for (int j = i + 1; j < label_count; j++) {
                if (quotient_adj[i][j]) {
                    printf("  Block %d -- Block %d\n", block_labels[i], block_labels[j]);
                    edge_count++;
                }
            }
        }
        printf("Quotient graph has %d vertices and %d edges\n", label_count, edge_count);
    }
    
    // Convert to graph-coloring library format and calculate chromatic number
    try {
        // Build graph in format expected by graph-coloring library
        std::map<std::string, std::vector<std::string>> coloring_graph;
        
        // Initialize all vertices
        for (int i = 0; i < label_count; i++) {
            coloring_graph[std::to_string(i)] = std::vector<std::string>();
        }
        
        // Add edges to graph
        for (int i = 0; i < label_count; i++) {
            for (int j = i + 1; j < label_count; j++) {
                if (quotient_adj[i][j]) {
                    coloring_graph[std::to_string(i)].push_back(std::to_string(j));
                    coloring_graph[std::to_string(j)].push_back(std::to_string(i));
                }
            }
        }
        
        // Use DSATUR algorithm to find chromatic number
        GraphColoring::Dsatur dsatur(coloring_graph);
        dsatur.color();
        
        // Get chromatic number
        int chromatic_number = dsatur.get_num_colors();
        
        // qi = k - chromatic_number  
        int qi = k - chromatic_number;
        
        if (VERBOSE_QI_DEBUG) {
            printf("Chromatic number (DSATUR): %d\n", chromatic_number);
            printf("qi = k - chromatic_number = %d - %d = %d\n", k, chromatic_number, qi);
            printf("=== END QI DEBUG ===\n\n");
        }
        
        return qi;
        
    } catch (const std::exception& e) {
        if (VERBOSE_QI_DEBUG) {
            printf("Graph coloring error: %s\n", e.what());
            printf("Falling back to exhaustive calculation...\n");
        }
        
        // Fallback to exhaustive calculation if coloring fails
        return calculateQiNumberInternalExhaustive(graph);
    }
}

int Partition::calculateQiNumberInternalExhaustive(const Graph& graph) const {
    int k = getNumBlocks();
    
    if (k == 1) return 0; // Single block is q-complete
    
    // Build quotient graph adjacency matrix
    bool quotient_adj[MAX_VERTICES][MAX_VERTICES];
    for (int i = 0; i < MAX_VERTICES; i++) {
        for (int j = 0; j < MAX_VERTICES; j++) {
            quotient_adj[i][j] = false;
        }
    }
    
    // Get list of used block labels (may not be consecutive)
    int block_labels[MAX_VERTICES];
    int label_count = 0;
    bool seen[MAX_VERTICES] = {false};
    
    for (int v = 0; v < num_vertices_; v++) {
        int label = partition_[v];
        if (!seen[label]) {
            seen[label] = true;
            block_labels[label_count++] = label;
        }
    }
    
    // Check all edges in original graph to build quotient graph
    const int* adj_matrix = graph.getAdjMatrix();
    
    for (int u = 0; u < num_vertices_; u++) {
        for (int v = u + 1; v < num_vertices_; v++) {
            if (adj_matrix[u * num_vertices_ + v] == 1) {
                int block_u = partition_[u];
                int block_v = partition_[v];
                
                if (block_u != block_v) {
                    quotient_adj[block_u][block_v] = true;
                    quotient_adj[block_v][block_u] = true;
                }
            }
        }
    }
    
    // Exact algorithm: try all possible ways to partition blocks into disjoint independent sets
    int max_qi = 0;
    
    // Use recursive backtracking to find optimal partition
    bool used[MAX_VERTICES] = {false};
    
    if (VERBOSE_QI_DEBUG) {
        printf("Starting exhaustive search for optimal qi...\n");
    }
    
    findOptimalQi(block_labels, label_count, quotient_adj, used, 0, 0, max_qi);
    
    if (VERBOSE_QI_DEBUG) {
        printf("Exhaustive fallback result: qi = %d\n", max_qi);
    }
    
    return max_qi;
}

int Partition::calculateQiNumberInternal(const Graph& graph, int min_required_qi) const {
    int k = getNumBlocks();
    
    if (VERBOSE_QI_DEBUG) {
        printf("=== QI CALCULATION WITH EARLY STOPPING (k=%d, min_required=%d) ===\n", k, min_required_qi);
    }
    
    if (k == 1) return 0; // Single block is q-complete
    
    // For larger graphs, try chromatic number approach first
    if (k > 15) {
        if (VERBOSE_QI_DEBUG) {
            printf("Algorithm: FAST (DSATUR chromatic number) - attempting early exit\n");
        }
        try {
            // Build quotient graph for SmallGraph (same as in main method)
            int block_labels[MAX_VERTICES];
            int label_count = 0;
            bool seen[MAX_VERTICES] = {false};
            
            for (int v = 0; v < num_vertices_; v++) {
                int label = partition_[v];
                if (!seen[label]) {
                    seen[label] = true;
                    block_labels[label_count++] = label;
                }
            }
            
            // Create mapping from block labels to consecutive indices
            int label_to_index[MAX_VERTICES];
            for (int i = 0; i < MAX_VERTICES; i++) {
                label_to_index[i] = -1;
            }
            for (int i = 0; i < label_count; i++) {
                label_to_index[block_labels[i]] = i;
            }
            
            // Build quotient graph adjacency matrix
            std::vector<std::vector<bool>> quotient_adj(label_count, std::vector<bool>(label_count, false));
            const int* adj_matrix = graph.getAdjMatrix();
            
            for (int u = 0; u < num_vertices_; u++) {
                for (int v = u + 1; v < num_vertices_; v++) {
                    if (adj_matrix[u * num_vertices_ + v] == 1) {
                        int block_u = partition_[u];
                        int block_v = partition_[v];
                        
                        if (block_u != block_v) {
                            int idx_u = label_to_index[block_u];
                            int idx_v = label_to_index[block_v];
                            quotient_adj[idx_u][idx_v] = true;
                            quotient_adj[idx_v][idx_u] = true;
                        }
                    }
                }
            }
            
            // Convert to graph-coloring library format
            std::map<std::string, std::vector<std::string>> coloring_graph;
            
            // Initialize all vertices
            for (int i = 0; i < label_count; i++) {
                coloring_graph[std::to_string(i)] = std::vector<std::string>();
            }
            
            // Add edges
            for (int i = 0; i < label_count; i++) {
                for (int j = i + 1; j < label_count; j++) {
                    if (quotient_adj[i][j]) {
                        coloring_graph[std::to_string(i)].push_back(std::to_string(j));
                        coloring_graph[std::to_string(j)].push_back(std::to_string(i));
                    }
                }
            }
            
            // Use DSATUR to find chromatic number
            GraphColoring::Dsatur dsatur(coloring_graph);
            dsatur.color();
            int chromatic_number = dsatur.get_num_colors();
            int qi = k - chromatic_number;
            
            if (VERBOSE_QI_DEBUG) {
                printf("Fast chromatic calculation (DSATUR): qi = %d - %d = %d (required >= %d)\n", 
                       k, chromatic_number, qi, min_required_qi);
            }
            
            // If this satisfies our requirement, return it
            if (qi >= min_required_qi) {
                return qi;
            }
            
        } catch (const std::exception& e) {
            if (VERBOSE_QI_DEBUG) {
                printf("Graph coloring error in fast path: %s\n", e.what());
                printf("Returning UNDETERMINED for large graph\n");
            }
            return -1; // Return special value for undetermined
        }
        
        // If we reach here, fast algorithm didn't satisfy threshold
        if (VERBOSE_QI_DEBUG) {
            printf("Fast algorithm result insufficient (required=%d)\n", min_required_qi);
            printf("Returning UNDETERMINED for large graph to avoid expensive computation\n");
        }
        return -1; // Return special value for undetermined
    }
    
    // Fall back to exhaustive early-stopping search for small graphs only
    if (VERBOSE_QI_DEBUG) {
        printf("Algorithm: EXACT (exhaustive backtracking with early stopping)\n");
    }
    
    // Early exit: if we only need qi >= min_required_qi, we can stop early
    if (min_required_qi <= 0) return calculateQiNumberInternal(graph);
       
    // Build quotient graph adjacency matrix
    bool quotient_adj[MAX_VERTICES][MAX_VERTICES];
    for (int i = 0; i < MAX_VERTICES; i++) {
        for (int j = 0; j < MAX_VERTICES; j++) {
            quotient_adj[i][j] = false;
        }
    }
    
    // Get list of used block labels (may not be consecutive)
    int block_labels[MAX_VERTICES];
    int label_count = 0;
    bool seen[MAX_VERTICES] = {false};
    
    for (int v = 0; v < num_vertices_; v++) {
        int label = partition_[v];
        if (!seen[label]) {
            seen[label] = true;
            block_labels[label_count++] = label;
        }
    }
    
    // Check all edges in original graph to build quotient graph
    const int* adj_matrix = graph.getAdjMatrix();
    
    for (int u = 0; u < num_vertices_; u++) {
        for (int v = u + 1; v < num_vertices_; v++) {
            if (adj_matrix[u * num_vertices_ + v] == 1) {
                int block_u = partition_[u];
                int block_v = partition_[v];
                
                if (block_u != block_v) {
                    quotient_adj[block_u][block_v] = true;
                    quotient_adj[block_v][block_u] = true;
                }
            }
        }
    }
    
    // Use recursive backtracking with early stopping
    bool used[MAX_VERTICES] = {false};
    int max_qi = 0;
    
    if (VERBOSE_QI_DEBUG) {
        printf("Starting exhaustive search with early stopping (min_required: %d)...\n", min_required_qi);
    }
    
    findOptimalQi(block_labels, label_count, quotient_adj, used, 0, 0, max_qi, min_required_qi);
    
    if (VERBOSE_QI_DEBUG) {
        printf("Early stopping search result: qi = %d (required >= %d)\n", max_qi, min_required_qi);
    }
    
    return max_qi;
}

// Helper function for exact qi calculation using exhaustive backtracking
void Partition::findOptimalQi(const int* block_labels, int label_count, 
                             const bool quotient_adj[MAX_VERTICES][MAX_VERTICES],
                             bool* used, int start_idx, int current_qi, int& max_qi) const {
    
    // Base case: no more unused blocks
    bool has_unused = false;
    for (int i = 0; i < label_count; i++) {
        if (!used[block_labels[i]]) {
            has_unused = true;
            break;
        }
    }
    
    if (!has_unused) {
        if (current_qi > max_qi) {
            max_qi = current_qi;
        }
        return;
    }
    
    // Find first unused block
    int first_unused = -1;
    for (int i = 0; i < label_count; i++) {
        if (!used[block_labels[i]]) {
            first_unused = i;
            break;
        }
    }
    
    if (first_unused == -1) {
        if (current_qi > max_qi) {
            max_qi = current_qi;
        }
        return;
    }
    
    int start_block = block_labels[first_unused];
    
    // Try all possible independent sets that include start_block
    // Use bitmask to enumerate all subsets of remaining unused blocks
    int unused_blocks[MAX_VERTICES];
    int unused_count = 0;
    
    for (int i = first_unused + 1; i < label_count; i++) {
        if (!used[block_labels[i]]) {
            unused_blocks[unused_count++] = block_labels[i];
        }
    }
    
    // Try all subsets of unused_blocks (2^unused_count possibilities)
    int max_subset = 1 << unused_count;
    
    for (int subset = 0; subset < max_subset; subset++) {
        // Build independent set starting with start_block
        int independent_set[MAX_VERTICES];
        int set_size = 1;
        independent_set[0] = start_block;
        
        // Add blocks from subset if they form an independent set
        bool valid_set = true;
        for (int bit = 0; bit < unused_count && valid_set; bit++) {
            if (subset & (1 << bit)) {
                int candidate = unused_blocks[bit];
                
                // Check if candidate is independent of all blocks in current set
                for (int j = 0; j < set_size; j++) {
                    if (quotient_adj[candidate][independent_set[j]]) {
                        valid_set = false;
                        break;
                    }
                }
                
                if (valid_set) {
                    independent_set[set_size++] = candidate;
                }
            }
        }
        
        if (valid_set) {
            if (VERBOSE_QI_DEBUG) {
                // DEBUG: Print found independent set
                printf("Found independent set (size %d, contributes %d): {", set_size, set_size - 1);
                for (int j = 0; j < set_size; j++) {
                    printf("%d", independent_set[j]);
                    if (j < set_size - 1) printf(", ");
                }
                printf("}\n");
            }
            
            // Mark blocks in this independent set as used
            bool temp_used[MAX_VERTICES];
            for (int j = 0; j < MAX_VERTICES; j++) {
                temp_used[j] = used[j];
            }
            
            for (int j = 0; j < set_size; j++) {
                temp_used[independent_set[j]] = true;
            }
            
            // Contribution is (set_size - 1), but ignore single blocks (contribute 0)
            int contribution = (set_size > 1) ? set_size - 1 : 0;
            
            // Recursively solve for remaining blocks
            findOptimalQi(block_labels, label_count, quotient_adj, temp_used, 
                         0, current_qi + contribution, max_qi);
        }
    }
}

// Helper function with early stopping for qi calculation
void Partition::findOptimalQi(const int* block_labels, int label_count, 
                             const bool quotient_adj[MAX_VERTICES][MAX_VERTICES],
                             bool* used, int start_idx, int current_qi, int& max_qi,
                             int min_required_qi) const {
    
    // Early stopping: if we've already found a sufficient qi, stop searching
    if (max_qi >= min_required_qi) {
        return;
    }
    
    // Base case: no more unused blocks
    bool has_unused = false;
    for (int i = 0; i < label_count; i++) {
        if (!used[block_labels[i]]) {
            has_unused = true;
            break;
        }
    }
    
    if (!has_unused) {
        if (current_qi > max_qi) {
            max_qi = current_qi;
        }
        return;
    }
    
    // Find first unused block
    int first_unused = -1;
    for (int i = 0; i < label_count; i++) {
        if (!used[block_labels[i]]) {
            first_unused = i;
            break;
        }
    }
    
    if (first_unused == -1) {
        if (current_qi > max_qi) {
            max_qi = current_qi;
        }
        return;
    }
    
    int start_block = block_labels[first_unused];
    
    // Try all possible independent sets that include start_block
    // Use bitmask to enumerate all subsets of remaining unused blocks
    int unused_blocks[MAX_VERTICES];
    int unused_count = 0;
    
    for (int i = first_unused + 1; i < label_count; i++) {
        if (!used[block_labels[i]]) {
            unused_blocks[unused_count++] = block_labels[i];
        }
    }
    
    // Try all subsets of unused_blocks (2^unused_count possibilities)
    int max_subset = 1 << unused_count;
    
    for (int subset = 0; subset < max_subset; subset++) {
        // Early stopping check
        if (max_qi >= min_required_qi) {
            return;
        }
        
        // Build independent set starting with start_block
        int independent_set[MAX_VERTICES];
        int set_size = 1;
        independent_set[0] = start_block;
        
        // Add blocks from subset if they form an independent set
        bool valid_set = true;
        for (int bit = 0; bit < unused_count && valid_set; bit++) {
            if (subset & (1 << bit)) {
                int candidate = unused_blocks[bit];
                
                // Check if candidate is independent of all blocks in current set
                for (int j = 0; j < set_size; j++) {
                    if (quotient_adj[candidate][independent_set[j]]) {
                        valid_set = false;
                        break;
                    }
                }
                
                if (valid_set) {
                    independent_set[set_size++] = candidate;
                }
            }
        }
        
        if (valid_set) {
            // Mark blocks in this independent set as used
            bool temp_used[MAX_VERTICES];
            for (int j = 0; j < MAX_VERTICES; j++) {
                temp_used[j] = used[j];
            }
            
            for (int j = 0; j < set_size; j++) {
                temp_used[independent_set[j]] = true;
            }
            
            // Contribution is (set_size - 1)
            int contribution = (set_size > 1) ? set_size - 1 : 0;
            
            // Recursively solve for remaining blocks
            findOptimalQi(block_labels, label_count, quotient_adj, temp_used, 
                         0, current_qi + contribution, max_qi, min_required_qi);
        }
    }
}

