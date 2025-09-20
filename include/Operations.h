#pragma once

#include "Partition.h"
#include "Graph.h"
#include <vector>
#include <utility>
#include <functional>

// result of a single partition operation
struct OperationResult {
    bool success;
    Partition result_partition;
    std::string operation_description;
    int interior_edge_change; // positive means increase, negative means decrease
    
    // metadata for tracking
    int affected_block_1;
    int affected_block_2;
    std::vector<int> moved_vertices;
};

class PartitionOperations {
public:
    // sc operations: split connected components within blocks
    static std::vector<OperationResult> findAllScOperations(const Partition& partition, const Graph& graph);
    static OperationResult performScOperation(const Partition& partition, const Graph& graph, int block_id, int component_index = -1);
    
    // su operations: split unconnected blocks into separate components
    static std::vector<OperationResult> findAllSuOperations(const Partition& partition, const Graph& graph);
    static OperationResult performSuOperation(const Partition& partition, const Graph& graph, int block_id);
    
    // mu operations: merge unconnected blocks
    static std::vector<OperationResult> findAllMuOperations(const Partition& partition, const Graph& graph);
    static OperationResult performMuOperation(const Partition& partition, const Graph& graph, int block_id_1, int block_id_2);
    
    // mc operations: merge connected blocks
    static std::vector<OperationResult> findAllMcOperations(const Partition& partition, const Graph& graph);
    static OperationResult performMcOperation(const Partition& partition, const Graph& graph, int block_id_1, int block_id_2);
    
    // composite operations
    // scmu: sc followed by optimal mu selection using qi-diagram degrees
    static OperationResult performScMuOperation(const Partition& partition, const Graph& graph);
    
    // sumc: su followed by optimal mc selection
    static OperationResult performSuMcOperation(const Partition& partition, const Graph& graph);
    
    // utility functions for operation analysis
    static std::vector<std::pair<int, int>> findQiPairs(const Partition& partition, const Graph& graph);
    static std::vector<std::pair<int, int>> findConnectedBlockPairs(const Partition& partition, const Graph& graph);
    static bool areBlocksConnectedInQuotient(const Partition& partition, const Graph& graph, int block_id_1, int block_id_2);
    
    // greedy selection methods (moved from private for ScMuAlgorithm separate step visualization)
    static OperationResult selectOptimalScOperation(const std::vector<OperationResult>& sc_options, const Graph& graph);
    static std::pair<int, int> selectOptimalMuPair(const std::vector<std::pair<int, int>>& qi_pairs, const Partition& partition, const Graph& graph);
    
private:
    // helper structures for internal operations
    struct SplittableComponent {
        int block_id;
        std::vector<int> component_vertices;
        int component_index;
    };
    
    // internal helper methods
    static std::vector<SplittableComponent> findSplittableComponents(const Partition& partition, const Graph& graph);
    static std::vector<int> findLeafVerticesInSpanningTree(const std::vector<int>& component_vertices, const Graph& graph);
    static std::vector<std::vector<int>> findComponentsInBlock(const Partition& partition, const Graph& graph, int block_id);
    static int calculateBlockAntiDegree(const std::vector<std::pair<int, int>>& qi_pairs, int block_id);
};