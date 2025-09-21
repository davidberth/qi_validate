#pragma once

// forward declaration
class Graph;

class Partition {
public:
    static const int MAX_VERTICES = 100;
    
    // constructors
    Partition();
    Partition(const int* partition_array, int num_vertices);
    Partition(const Partition& other);
    
    // assignment operator
    Partition& operator=(const Partition& other);
    
    // basic accessors
    int getLabel(int vertex) const;
    void setLabel(int vertex, int label);
    int getNumVertices() const { return num_vertices_; }
    const int* getPartitionArray() const { return partition_; }
    
    // essential block operations (array-based)
    int getNumBlocks() const;
    void getBlockVertices(int block_label, int* vertices, int& count) const;
    
    // property calculations require graph
    void calculateQiNumber(const Graph& graph);
    void calculateQiNumber(const Graph& graph, int min_required_qi);
    int getQiNumber() const { return qi_number_; }
    
    // essential for Mc operations - check if blocks are connected in quotient
    bool areBlocksConnectedInQuotient(const Graph& graph, int block1, int block2) const;
    
    // merge two blocks (Mc operation)
    void mergeBlocks(int block1, int block2);

private:
    int partition_[MAX_VERTICES];
    int num_vertices_;
    
    // cached qi number
    mutable bool qi_calculated_;
    mutable int qi_number_;
    
    // helper methods
    void copyFrom(const Partition& other);
    void invalidateQiCache();
    int calculateQiNumberInternal(const Graph& graph) const;
    int calculateQiNumberInternal(const Graph& graph, int min_required_qi) const;
    int calculateQiNumberInternalExhaustive(const Graph& graph) const;
    void findOptimalQi(const int* block_labels, int label_count, 
                      const bool quotient_adj[MAX_VERTICES][MAX_VERTICES],
                      bool* used, int start_idx, int current_qi, int& max_qi) const;
    void findOptimalQi(const int* block_labels, int label_count, 
                      const bool quotient_adj[MAX_VERTICES][MAX_VERTICES],
                      bool* used, int start_idx, int current_qi, int& max_qi, 
                      int min_required_qi) const;
};