#include "../include/Partition.h"
#include "../include/Graph.h"
#include <algorithm>
#include <queue>
#include <sstream>
#include <cassert>
#include <cstring>
#include <cstdio>

Partition::Partition() : num_vertices_(0), properties_calculated_(false), original_index_(-1) {
    std::fill(partition_, partition_ + MAX_VERTICES, 0);
}

Partition::Partition(const int* partition_array, int num_vertices) 
    : num_vertices_(num_vertices), properties_calculated_(false), original_index_(-1) {
    assert(num_vertices <= MAX_VERTICES);
    std::copy(partition_array, partition_array + num_vertices, partition_);
    std::fill(partition_ + num_vertices, partition_ + MAX_VERTICES, 0);
}

Partition::Partition(const std::vector<int>& partition_vector) 
    : num_vertices_(partition_vector.size()), properties_calculated_(false), original_index_(-1) {
    assert(partition_vector.size() <= MAX_VERTICES);
    std::copy(partition_vector.begin(), partition_vector.end(), partition_);
    std::fill(partition_ + num_vertices_, partition_ + MAX_VERTICES, 0);
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
    properties_calculated_ = other.properties_calculated_;
    is_connected_ = other.is_connected_;
    is_independent_ = other.is_independent_;
    interior_edges_ = other.interior_edges_;
    qi_number_ = other.qi_number_;
    original_index_ = other.original_index_;
    operation_ = other.operation_;
}

void Partition::invalidateCache() {
    properties_calculated_ = false;
}

int Partition::getLabel(int vertex) const {
    assert(vertex >= 0 && vertex < num_vertices_);
    return partition_[vertex];
}

void Partition::setLabel(int vertex, int label) {
    assert(vertex >= 0 && vertex < num_vertices_);
    if (partition_[vertex] != label) {
        partition_[vertex] = label;
        invalidateCache();
    }
}

std::map<int, std::vector<int>> Partition::getBlocks() const {
    std::map<int, std::vector<int>> blocks;
    for (int v = 0; v < num_vertices_; v++) {
        blocks[partition_[v]].push_back(v);
    }
    return blocks;
}

std::set<int> Partition::getUsedLabels() const {
    std::set<int> labels;
    for (int v = 0; v < num_vertices_; v++) {
        labels.insert(partition_[v]);
    }
    return labels;
}

int Partition::getNumBlocks() const {
    return getUsedLabels().size();
}

std::vector<int> Partition::getBlockVertices(int block_label) const {
    std::vector<int> vertices;
    for (int v = 0; v < num_vertices_; v++) {
        if (partition_[v] == block_label) {
            vertices.push_back(v);
        }
    }
    return vertices;
}

int Partition::getBlockSize(int block_label) const {
    int count = 0;
    for (int v = 0; v < num_vertices_; v++) {
        if (partition_[v] == block_label) {
            count++;
        }
    }
    return count;
}

void Partition::calculateProperties(const Graph& graph) {
    if (properties_calculated_) return;
    
    // Calculate interior edge count
    interior_edges_ = 0;
    const int* adj_matrix = graph.getAdjMatrix();
    for (int i = 0; i < num_vertices_; i++) {
        for (int j = i + 1; j < num_vertices_; j++) {
            if (adj_matrix[i * num_vertices_ + j] && partition_[i] == partition_[j]) {
                interior_edges_++;
            }
        }
    }
    
    // Check if partition is independent (no interior edges)
    is_independent_ = (interior_edges_ == 0);
    
    // Check if partition is connected (all blocks are connected)
    is_connected_ = true;
    auto blocks = getBlocks();
    for (const auto& block_pair : blocks) {
        if (!isBlockConnected(graph, block_pair.first)) {
            is_connected_ = false;
            break;
        }
    }
    
    // Calculate qi number
    qi_number_ = calculateQiNumberInternal(graph);
    
    properties_calculated_ = true;
}

bool Partition::isBlockConnected(const Graph& graph, int block_label) const {
    std::vector<int> block_vertices = getBlockVertices(block_label);
    
    if (block_vertices.size() <= 1) {
        return true; // Single vertex or empty block is trivially connected
    }
    
    // BFS to check connectivity within the block
    const int* adj_matrix = graph.getAdjMatrix();
    std::vector<bool> visited(num_vertices_, false);
    std::queue<int> queue;
    
    // Start BFS from first vertex in block
    queue.push(block_vertices[0]);
    visited[block_vertices[0]] = true;
    int visited_count = 1;
    
    while (!queue.empty() && visited_count < (int)block_vertices.size()) {
        int current = queue.front();
        queue.pop();
        
        // Check all other vertices in the same block
        for (int v : block_vertices) {
            if (!visited[v] && adj_matrix[current * num_vertices_ + v]) {
                visited[v] = true;
                queue.push(v);
                visited_count++;
            }
        }
    }
    
    return visited_count == (int)block_vertices.size();
}

bool Partition::isBlockIndependent(const Graph& graph, int block_label) const {
    std::vector<int> block_vertices = getBlockVertices(block_label);
    
    if (block_vertices.size() <= 1) {
        return true; // Single vertex is trivially independent
    }
    
    const int* adj_matrix = graph.getAdjMatrix();
    for (size_t i = 0; i < block_vertices.size(); i++) {
        for (size_t j = i + 1; j < block_vertices.size(); j++) {
            int u = block_vertices[i];
            int v = block_vertices[j];
            if (adj_matrix[u * num_vertices_ + v]) {
                return false; // Found an edge within the block
            }
        }
    }
    
    return true;
}

std::vector<std::vector<int>> Partition::getBlockComponents(const Graph& graph, int block_label) const {
    std::vector<int> block_vertices = getBlockVertices(block_label);
    std::vector<std::vector<int>> components;
    
    if (block_vertices.empty()) {
        return components;
    }
    
    const int* adj_matrix = graph.getAdjMatrix();
    std::vector<bool> visited(num_vertices_, false);
    
    for (int start_vertex : block_vertices) {
        if (!visited[start_vertex]) {
            std::vector<int> component;
            std::queue<int> queue;
            
            queue.push(start_vertex);
            visited[start_vertex] = true;
            component.push_back(start_vertex);
            
            while (!queue.empty()) {
                int current = queue.front();
                queue.pop();
                
                // Check all other vertices in the same block
                for (int v : block_vertices) {
                    if (!visited[v] && adj_matrix[current * num_vertices_ + v]) {
                        visited[v] = true;
                        queue.push(v);
                        component.push_back(v);
                    }
                }
            }
            
            components.push_back(component);
        }
    }
    
    return components;
}

int Partition::calculateQiNumberInternal(const Graph& graph) const {
    int k = getNumBlocks();
    
    if (k == 1) return 0; // Single block is q-complete
    
    // Build quotient graph adjacency matrix
    bool quotient_adj[MAX_VERTICES][MAX_VERTICES];
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            quotient_adj[i][j] = false;
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
    
    // Use greedy algorithm to compute qi
    bool used[MAX_VERTICES] = {false};
    int qi = 0;
    
    while (true) {
        // Find largest independent set in remaining quotient graph
        std::vector<int> available_blocks;
        for (int i = 0; i < k; i++) {
            if (!used[i]) {
                available_blocks.push_back(i);
            }
        }
        
        if (available_blocks.empty()) break;
        
        // Greedy: start with block that has minimum connections to other available blocks
        int best_start = -1;
        int min_connections = k + 1;
        for (int block : available_blocks) {
            int connections = 0;
            for (int other : available_blocks) {
                if (block != other && quotient_adj[block][other]) {
                    connections++;
                }
            }
            if (connections < min_connections) {
                min_connections = connections;
                best_start = block;
            }
        }
        
        // Build maximal independent set starting with best_start
        std::vector<int> independent_set = {best_start};
        used[best_start] = true;
        
        for (int candidate : available_blocks) {
            if (candidate == best_start) continue;
            
            // Check if candidate is independent of all blocks in current set
            bool is_independent_of_set = true;
            for (int block_in_set : independent_set) {
                if (quotient_adj[candidate][block_in_set]) {
                    is_independent_of_set = false;
                    break;
                }
            }
            
            if (is_independent_of_set) {
                independent_set.push_back(candidate);
                used[candidate] = true;
            }
        }
        
        // Add (size - 1) to qi for this independent set
        qi += (int)independent_set.size() - 1;
    }
    
    return qi;
}

void Partition::renormalizeLabels() {
    std::map<int, int> old_to_new_id;
    std::set<int> used_blocks = getUsedLabels();
    
    int new_id = 0;
    for (int old_id : used_blocks) {
        old_to_new_id[old_id] = new_id++;
    }
    
    for (int v = 0; v < num_vertices_; v++) {
        partition_[v] = old_to_new_id[partition_[v]];
    }
    
    invalidateCache();
}

bool Partition::isNonDegenerate() const {
    int max_label = *std::max_element(partition_, partition_ + num_vertices_);
    std::set<int> used_labels = getUsedLabels();
    
    // Should use labels 0, 1, 2, ..., max_label exactly
    return (int)used_labels.size() == max_label + 1 && 
           *used_labels.begin() == 0 && 
           *used_labels.rbegin() == max_label;
}

bool Partition::isCanonical() const {
    std::vector<int> first_occurrence(getNumBlocks(), -1);
    
    for (int v = 0; v < num_vertices_; v++) {
        int label = partition_[v];
        if (first_occurrence[label] == -1) {
            first_occurrence[label] = v;
        }
    }
    
    // Check that labels appear in order of first occurrence
    for (int label = 1; label < getNumBlocks(); label++) {
        if (first_occurrence[label] <= first_occurrence[label - 1]) {
            return false;
        }
    }
    
    return true;
}

bool Partition::operator==(const Partition& other) const {
    if (num_vertices_ != other.num_vertices_) return false;
    return std::equal(partition_, partition_ + num_vertices_, other.partition_);
}

bool Partition::operator!=(const Partition& other) const {
    return !(*this == other);
}

size_t Partition::hash() const {
    size_t hash_value = 0;
    for (int i = 0; i < num_vertices_; i++) {
        hash_value = hash_value * 31 + partition_[i];
    }
    return hash_value;
}

std::string Partition::toString() const {
    std::ostringstream oss;
    oss << "[";
    for (int i = 0; i < num_vertices_; i++) {
        oss << partition_[i];
        if (i < num_vertices_ - 1) oss << "-";
    }
    oss << "]";
    return oss.str();
}

std::string Partition::toDebugString() const {
    std::ostringstream oss;
    oss << toString();
    if (properties_calculated_) {
        oss << " (Blocks: " << getNumBlocks() 
            << ", Interior: " << interior_edges_ 
            << ", qi: " << qi_number_
            << ", Connected: " << (is_connected_ ? "Y" : "N")
            << ", Independent: " << (is_independent_ ? "Y" : "N")
            << ")";
    }
    if (!operation_.empty()) {
        oss << " [" << operation_ << "]";
    }
    return oss.str();
}

void Partition::setOperation(const std::string& operation) {
    operation_ = operation;
}