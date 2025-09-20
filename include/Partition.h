#pragma once

#include <vector>
#include <map>
#include <set>
#include <string>

// forward declaration
class Graph;

class Partition {
public:
    static const int MAX_VERTICES = 100;
    
    // constructors
    Partition();
    Partition(const int* partition_array, int num_vertices);
    Partition(const std::vector<int>& partition_vector);
    Partition(const Partition& other);
    
    // assignment operator
    Partition& operator=(const Partition& other);
    
    // basic accessors
    int getLabel(int vertex) const;
    void setLabel(int vertex, int label);
    int getNumVertices() const { return num_vertices_; }
    const int* getPartitionArray() const { return partition_; }
    
    // block operations
    std::map<int, std::vector<int>> getBlocks() const;
    std::set<int> getUsedLabels() const;
    int getNumBlocks() const;
    std::vector<int> getBlockVertices(int block_label) const;
    int getBlockSize(int block_label) const;
    
    // property calculations require graph
    void calculateProperties(const Graph& graph);
    bool isConnectedPartition() const { return is_connected_; }
    bool isIndependentPartition() const { return is_independent_; }
    int getInteriorEdgeCount() const { return interior_edges_; }
    int getQiNumber() const { return qi_number_; }
    
    // block-level properties require graph
    bool isBlockConnected(const Graph& graph, int block_label) const;
    bool isBlockIndependent(const Graph& graph, int block_label) const;
    std::vector<std::vector<int>> getBlockComponents(const Graph& graph, int block_label) const;
    
    // utility functions
    // ensure consecutive labels starting from 0
    void renormalizeLabels(); 
    // uses all labels from 0 to k-1
    bool isNonDegenerate() const; 
    // first occurrence order: 0, then 1, then 2, etc
    bool isCanonical() const; 
    
    // comparison and hashing
    bool operator==(const Partition& other) const;
    bool operator!=(const Partition& other) const;
    size_t hash() const;
    
    // string representation
    std::string toString() const;
    // with properties
    std::string toDebugString() const; 
    
    // metadata for algorithm tracking
    void setOriginalIndex(long long index) { original_index_ = index; }
    long long getOriginalIndex() const { return original_index_; }
    void setOperation(const std::string& operation);
    std::string getOperation() const { return operation_; }

private:
    int partition_[MAX_VERTICES];
    int num_vertices_;
    
    // cached properties calculated when needed
    mutable bool properties_calculated_;
    mutable bool is_connected_;
    mutable bool is_independent_;
    mutable int interior_edges_;
    mutable int qi_number_;
    
    // metadata
    long long original_index_;
    std::string operation_;
    
    // helper methods
    void copyFrom(const Partition& other);
    void invalidateCache();
    int calculateQiNumberInternal(const Graph& graph) const;
};

// hash function for use in unordered containers
namespace std {
    template<>
    struct hash<Partition> {
        size_t operator()(const Partition& p) const {
            return p.hash();
        }
    };
}