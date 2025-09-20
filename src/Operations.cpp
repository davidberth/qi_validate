#include "../include/Operations.h"
#include <algorithm>
#include <queue>
#include <map>
#include <set>
#include <random>
#include <functional>
#include <iostream>

// static random number generator for operation selection
static std::random_device rd;
static std::mt19937 gen(rd());

std::vector<OperationResult> PartitionOperations::findAllScOperations(const Partition& partition, const Graph& graph) {
    std::vector<OperationResult> results;
    std::vector<SplittableComponent> components = findSplittableComponents(partition, graph);
    
    for (const auto& component : components) {
        OperationResult result = performScOperation(partition, graph, component.block_id, component.component_index);
        if (result.success) {
            results.push_back(result);
        }
    }
    
    return results;
}

OperationResult PartitionOperations::performScOperation(const Partition& partition, const Graph& graph, int block_id, int component_index) {
    OperationResult result;
    result.success = false;
    result.interior_edge_change = 0;
    result.affected_block_1 = block_id;
    result.affected_block_2 = -1;
    
    // get all components within the specified block
    std::vector<std::vector<int>> components = findComponentsInBlock(partition, graph, block_id);
    
    // find splittable component (has >= 2 vertices)
    std::vector<int> target_component;
    for (size_t i = 0; i < components.size(); i++) {
        if (components[i].size() >= 2 && (component_index == -1 || (int)i == component_index)) {
            target_component = components[i];
            break;
        }
    }
    
    if (target_component.size() < 2) {
        result.operation_description = "Sc failed: no splittable component found";
        return result;
    }
    
    // create new partition by splitting the component
    result.result_partition = partition;
    
    // find leaf vertices in spanning tree for the component
    std::vector<int> leaf_vertices = findLeafVerticesInSpanningTree(target_component, graph);
    
    // select random leaf vertex to move to new block
    if (leaf_vertices.empty()) {
        // fallback: select random vertex from component
        std::uniform_int_distribution<> dis(0, target_component.size() - 1);
        int selected_vertex = target_component[dis(gen)];
        leaf_vertices.push_back(selected_vertex);
    }
    
    // randomly select one leaf vertex
    std::uniform_int_distribution<> dis(0, leaf_vertices.size() - 1);
    int vertex_to_move = leaf_vertices[dis(gen)];
    
    // find next available block id
    std::set<int> used_labels = result.result_partition.getUsedLabels();
    int new_block_id = *used_labels.rbegin() + 1;
    
    // move the selected vertex to new block
    result.result_partition.setLabel(vertex_to_move, new_block_id);
    result.moved_vertices.push_back(vertex_to_move);
    
    // calculate properties
    result.result_partition.calculateProperties(graph);
    
    // sc operations typically don't change interior edge count initially
    // (the change comes from subsequent mu operation)
    result.interior_edge_change = result.result_partition.getInteriorEdgeCount() - partition.getInteriorEdgeCount();
    
    result.success = true;
    result.operation_description = "Sc: split block " + std::to_string(block_id) + " (moved vertex " + std::to_string(vertex_to_move) + ")";
    
    return result;
}

std::vector<OperationResult> PartitionOperations::findAllSuOperations(const Partition& partition, const Graph& graph) {
    std::vector<OperationResult> results;
    auto blocks = partition.getBlocks();
    
    for (const auto& block_pair : blocks) {
        int block_id = block_pair.first;
        if (!partition.isBlockConnected(graph, block_id)) {
            OperationResult result = performSuOperation(partition, graph, block_id);
            if (result.success) {
                results.push_back(result);
            }
        }
    }
    
    return results;
}

OperationResult PartitionOperations::performSuOperation(const Partition& partition, const Graph& graph, int block_id) {
    OperationResult result;
    result.success = false;
    result.affected_block_1 = block_id;
    result.affected_block_2 = -1;
    
    // check if block is already connected
    if (partition.isBlockConnected(graph, block_id)) {
        result.operation_description = "Su failed: block " + std::to_string(block_id) + " is already connected";
        return result;
    }
    
    // get all components within this block
    std::vector<std::vector<int>> components = findComponentsInBlock(partition, graph, block_id);
    
    if (components.size() <= 1) {
        result.operation_description = "Su failed: block " + std::to_string(block_id) + " has only one component";
        return result;
    }
    
    // create new partition by assigning new block ids to components
    result.result_partition = partition;
    std::set<int> used_labels = result.result_partition.getUsedLabels();
    int next_block_id = *used_labels.rbegin() + 1;
    
    // keep first component in original block, move others to new blocks
    for (size_t comp_idx = 1; comp_idx < components.size(); comp_idx++) {
        for (int vertex : components[comp_idx]) {
            result.result_partition.setLabel(vertex, next_block_id);
            result.moved_vertices.push_back(vertex);
        }
        next_block_id++;
    }
    
    // calculate properties
    result.result_partition.calculateProperties(graph);
    result.interior_edge_change = result.result_partition.getInteriorEdgeCount() - partition.getInteriorEdgeCount();
    
    result.success = true;
    result.operation_description = "Su: split unconnected block " + std::to_string(block_id) + " into " + std::to_string(components.size()) + " blocks";
    
    return result;
}

std::vector<OperationResult> PartitionOperations::findAllMuOperations(const Partition& partition, const Graph& graph) {
    std::vector<OperationResult> results;
    std::vector<std::pair<int, int>> qi_pairs = findQiPairs(partition, graph);
    
    for (const auto& qi_pair : qi_pairs) {
        OperationResult result = performMuOperation(partition, graph, qi_pair.first, qi_pair.second);
        if (result.success) {
            results.push_back(result);
        }
    }
    
    return results;
}

OperationResult PartitionOperations::performMuOperation(const Partition& partition, const Graph& graph, int block_id_1, int block_id_2) {
    OperationResult result;
    result.success = false;
    result.affected_block_1 = block_id_1;
    result.affected_block_2 = block_id_2;
    
    // verify blocks are unconnected in quotient graph
    if (areBlocksConnectedInQuotient(partition, graph, block_id_1, block_id_2)) {
        result.operation_description = "Mu failed: blocks " + std::to_string(block_id_1) + " and " + std::to_string(block_id_2) + " are connected";
        return result;
    }
    
    // create new partition by merging the two blocks
    result.result_partition = partition;
    std::vector<int> block_2_vertices = partition.getBlockVertices(block_id_2);
    
    // move all vertices from block_2 to block_1
    for (int vertex : block_2_vertices) {
        result.result_partition.setLabel(vertex, block_id_1);
        result.moved_vertices.push_back(vertex);
    }
    
    // renormalize labels to ensure consecutive numbering
    result.result_partition.renormalizeLabels();
    
    // calculate properties
    result.result_partition.calculateProperties(graph);
    result.interior_edge_change = result.result_partition.getInteriorEdgeCount() - partition.getInteriorEdgeCount();
    
    result.success = true;
    result.operation_description = "Mu: merged blocks " + std::to_string(block_id_1) + " and " + std::to_string(block_id_2);
    
    return result;
}

std::vector<OperationResult> PartitionOperations::findAllMcOperations(const Partition& partition, const Graph& graph) {
    std::vector<OperationResult> results;
    std::vector<std::pair<int, int>> connected_pairs = findConnectedBlockPairs(partition, graph);
    
    for (const auto& connected_pair : connected_pairs) {
        OperationResult result = performMcOperation(partition, graph, connected_pair.first, connected_pair.second);
        if (result.success) {
            results.push_back(result);
        }
    }
    
    return results;
}

OperationResult PartitionOperations::performMcOperation(const Partition& partition, const Graph& graph, int block_id_1, int block_id_2) {
    OperationResult result;
    result.success = false;
    result.affected_block_1 = block_id_1;
    result.affected_block_2 = block_id_2;
    
    // verify blocks are connected in quotient graph
    if (!areBlocksConnectedInQuotient(partition, graph, block_id_1, block_id_2)) {
        result.operation_description = "Mc failed: blocks " + std::to_string(block_id_1) + " and " + std::to_string(block_id_2) + " are not connected";
        return result;
    }
    
    // create new partition by merging the two blocks
    result.result_partition = partition;
    std::vector<int> block_2_vertices = partition.getBlockVertices(block_id_2);
    
    // move all vertices from block_2 to block_1
    for (int vertex : block_2_vertices) {
        result.result_partition.setLabel(vertex, block_id_1);
        result.moved_vertices.push_back(vertex);
    }
    
    // renormalize labels to ensure consecutive numbering
    result.result_partition.renormalizeLabels();
    
    // calculate properties
    result.result_partition.calculateProperties(graph);
    result.interior_edge_change = result.result_partition.getInteriorEdgeCount() - partition.getInteriorEdgeCount();
    
    result.success = true;
    result.operation_description = "Mc: merged connected blocks " + std::to_string(block_id_1) + " and " + std::to_string(block_id_2);
    
    return result;
}

OperationResult PartitionOperations::performScMuOperation(const Partition& partition, const Graph& graph) {
    // first, perform sc operation
    std::vector<OperationResult> sc_options = findAllScOperations(partition, graph);
    
    if (sc_options.empty()) {
        OperationResult result;
        result.success = false;
        result.operation_description = "ScMu failed: no valid Sc operations available";
        return result;
    }
    
    // greedy select sc operation: prefer qi=2 with maximal qi-pairs
    OperationResult sc_result = selectOptimalScOperation(sc_options, graph);
    
    // then, perform optimal mu operation on the result
    std::vector<std::pair<int, int>> qi_pairs = findQiPairs(sc_result.result_partition, graph);
    
    if (qi_pairs.empty()) {
        OperationResult result;
        result.success = false;
        result.operation_description = "ScMu failed: no valid Mu operations available after Sc";
        return result;
    }
    
    // select optimal mu pair using qi-diagram degree strategy
    std::pair<int, int> optimal_pair = selectOptimalMuPair(qi_pairs, sc_result.result_partition, graph);
    OperationResult mu_result = performMuOperation(sc_result.result_partition, graph, optimal_pair.first, optimal_pair.second);
    
    if (!mu_result.success) {
        OperationResult result;
        result.success = false;
        result.operation_description = "ScMu failed: Mu operation failed after successful Sc";
        return result;
    }
    
    // combine results
    OperationResult combined_result = mu_result;
    combined_result.operation_description = "ScMu: " + sc_result.operation_description + " + " + mu_result.operation_description;
    combined_result.interior_edge_change = mu_result.result_partition.getInteriorEdgeCount() - partition.getInteriorEdgeCount();
    
    return combined_result;
}

OperationResult PartitionOperations::performSuMcOperation(const Partition& partition, const Graph& graph) {
    // first, perform su operation
    std::vector<OperationResult> su_options = findAllSuOperations(partition, graph);
    
    if (su_options.empty()) {
        OperationResult result;
        result.success = false;
        result.operation_description = "SuMc failed: no valid Su operations available";
        return result;
    }
    
    // select first available su operation
    OperationResult su_result = su_options[0];
    
    // then, perform mc operation on the result
    std::vector<std::pair<int, int>> connected_pairs = findConnectedBlockPairs(su_result.result_partition, graph);
    
    if (connected_pairs.empty()) {
        OperationResult result;
        result.success = false;
        result.operation_description = "SuMc failed: no valid Mc operations available after Su";
        return result;
    }
    
    // select first available connected pair
    OperationResult mc_result = performMcOperation(su_result.result_partition, graph, connected_pairs[0].first, connected_pairs[0].second);
    
    if (!mc_result.success) {
        OperationResult result;
        result.success = false;
        result.operation_description = "SuMc failed: Mc operation failed after successful Su";
        return result;
    }
    
    // combine results
    OperationResult combined_result = mc_result;
    combined_result.operation_description = "SuMc: " + su_result.operation_description + " + " + mc_result.operation_description;
    combined_result.interior_edge_change = mc_result.result_partition.getInteriorEdgeCount() - partition.getInteriorEdgeCount();
    
    return combined_result;
}

std::vector<std::pair<int, int>> PartitionOperations::findQiPairs(const Partition& partition, const Graph& graph) {
    std::vector<std::pair<int, int>> qi_pairs;
    auto blocks = partition.getBlocks();
    std::vector<int> block_ids;
    
    for (const auto& block_pair : blocks) {
        block_ids.push_back(block_pair.first);
    }
    
    // check all pairs of blocks for connectivity in quotient graph
    for (size_t i = 0; i < block_ids.size(); i++) {
        for (size_t j = i + 1; j < block_ids.size(); j++) {
            int block_1 = block_ids[i];
            int block_2 = block_ids[j];
            
            if (!areBlocksConnectedInQuotient(partition, graph, block_1, block_2)) {
                qi_pairs.push_back({block_1, block_2});
            }
        }
    }
    
    return qi_pairs;
}

std::vector<std::pair<int, int>> PartitionOperations::findConnectedBlockPairs(const Partition& partition, const Graph& graph) {
    std::vector<std::pair<int, int>> connected_pairs;
    auto blocks = partition.getBlocks();
    std::vector<int> block_ids;
    
    for (const auto& block_pair : blocks) {
        block_ids.push_back(block_pair.first);
    }
    
    // check all pairs of blocks for connectivity in quotient graph
    for (size_t i = 0; i < block_ids.size(); i++) {
        for (size_t j = i + 1; j < block_ids.size(); j++) {
            int block_1 = block_ids[i];
            int block_2 = block_ids[j];
            
            if (areBlocksConnectedInQuotient(partition, graph, block_1, block_2)) {
                connected_pairs.push_back({block_1, block_2});
            }
        }
    }
    return connected_pairs;
}

bool PartitionOperations::areBlocksConnectedInQuotient(const Partition& partition, const Graph& graph, int block_id_1, int block_id_2) {
    std::vector<int> block_1_vertices = partition.getBlockVertices(block_id_1);
    std::vector<int> block_2_vertices = partition.getBlockVertices(block_id_2);
    
    const int* adj_matrix = graph.getAdjMatrix();
    
    // check if there's any edge between vertices of the two blocks
    for (int v1 : block_1_vertices) {
        for (int v2 : block_2_vertices) {
            if (adj_matrix[v1 * graph.num_vertices + v2]) {
                return true;
            }
        }
    }
    
    return false;
}

std::vector<PartitionOperations::SplittableComponent> PartitionOperations::findSplittableComponents(const Partition& partition, const Graph& graph) {
    std::vector<SplittableComponent> splittable_components;
    auto blocks = partition.getBlocks();
    
    for (const auto& block_pair : blocks) {
        int block_id = block_pair.first;
        const std::vector<int>& block_vertices = block_pair.second;
        
        // find all components within this block
        std::vector<std::vector<int>> components = findComponentsInBlock(partition, graph, block_id);
        
        for (size_t comp_idx = 0; comp_idx < components.size(); comp_idx++) {
            // component is splittable if it has >= 2 vertices
            if (components[comp_idx].size() >= 2) {
                SplittableComponent sc;
                sc.block_id = block_id;
                sc.component_vertices = components[comp_idx];
                sc.component_index = comp_idx;
                splittable_components.push_back(sc);
            }
        }
    }
    
    return splittable_components;
}

std::vector<int> PartitionOperations::findLeafVerticesInSpanningTree(const std::vector<int>& component_vertices, const Graph& graph) {
    if (component_vertices.size() < 2) {
        return component_vertices; // single vertex is trivially a "leaf"
    }
    
    // build spanning tree within this component using dfs
    std::vector<std::vector<bool>> spanning_tree(graph.num_vertices, std::vector<bool>(graph.num_vertices, false));
    std::vector<bool> comp_visited(graph.num_vertices, false);
    
    std::function<void(int)> dfs = [&](int u) {
        comp_visited[u] = true;
        for (int v : component_vertices) {
            if (v != u && !comp_visited[v] && graph.getAdjMatrix()[u * graph.num_vertices + v]) {
                spanning_tree[u][v] = spanning_tree[v][u] = true;
                dfs(v);
            }
        }
    };
    
    dfs(component_vertices[0]);
    
    // find leaf vertices in the component's spanning tree
    std::vector<int> leaf_vertices;
    for (int v : component_vertices) {
        int degree = 0;
        for (int u : component_vertices) {
            if (u != v && spanning_tree[v][u]) {
                degree++;
            }
        }
        if (degree == 1) {
            leaf_vertices.push_back(v);
        }
    }
    
    return leaf_vertices;
}

std::vector<std::vector<int>> PartitionOperations::findComponentsInBlock(const Partition& partition, const Graph& graph, int block_id) {
    std::vector<int> block_vertices = partition.getBlockVertices(block_id);
    std::vector<std::vector<int>> components;
    
    if (block_vertices.empty()) {
        return components;
    }
    
    const int* adj_matrix = graph.getAdjMatrix();
    std::vector<bool> visited(graph.num_vertices, false);
    
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
                
                // check all other vertices in the same block
                for (int v : block_vertices) {
                    if (!visited[v] && adj_matrix[current * graph.num_vertices + v]) {
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

OperationResult PartitionOperations::selectOptimalScOperation(const std::vector<OperationResult>& sc_options, const Graph& graph) {
    if (sc_options.empty()) {
        OperationResult result;
        result.success = false;
        result.operation_description = "No Sc operations available";
        return result;
    }
    
    if (sc_options.size() == 1) {
        return sc_options[0];
    }
    
    // Greedy selection strategy:
    // 1. Prefer candidates with qi = 2 (exactly 2 Mu operations needed)
    // 2. Among qi=2 candidates, prefer maximal qi-pairs (maximal Mu choices)
    // 3. Fallback to any valid candidate
    
    std::vector<OperationResult> qi2_candidates;
    std::vector<OperationResult> other_candidates;
    
    for (const auto& sc_option : sc_options) {
        int qi_number = sc_option.result_partition.getQiNumber();
        if (qi_number == 2) {
            qi2_candidates.push_back(sc_option);
        } else if (qi_number > 0) { // still q-incomplete
            other_candidates.push_back(sc_option);
        }
    }
    
    // Prefer qi=2 candidates
    if (!qi2_candidates.empty()) {
        // Among qi=2 candidates, select one with maximal qi-pairs
        OperationResult best_candidate = qi2_candidates[0];
        int max_qi_pairs = findQiPairs(best_candidate.result_partition, graph).size();
        
        for (const auto& candidate : qi2_candidates) {
            int qi_pairs_count = findQiPairs(candidate.result_partition, graph).size();
            if (qi_pairs_count > max_qi_pairs) {
                max_qi_pairs = qi_pairs_count;
                best_candidate = candidate;
            }
        }
        
        return best_candidate;
    }
    
    // Fallback to other q-incomplete candidates
    if (!other_candidates.empty()) {
        return other_candidates[0]; // Just return first available
    }
    
    // Last resort: return first option (might be q-complete)
    return sc_options[0];
}

std::pair<int, int> PartitionOperations::selectOptimalMuPair(const std::vector<std::pair<int, int>>& qi_pairs, const Partition& partition, const Graph& graph) {
    if (qi_pairs.empty()) {
        return {-1, -1};
    }
    
    if (qi_pairs.size() == 1) {
        return qi_pairs[0];
    }
    
    // use qi-diagram degree-based selection strategy
    // count degree in qi-diagram (each block's degree = number of qi-pairs it participates in)
    std::map<int, int> anti_degree;
    auto blocks = partition.getBlocks();
    
    for (const auto& block_pair : blocks) {
        int block_id = block_pair.first;
        anti_degree[block_id] = 0; // will count qi-pair participations (degree in qi-diagram)
    }
    
    for (const auto& qi_pair : qi_pairs) {
        anti_degree[qi_pair.first]++;
        anti_degree[qi_pair.second]++;
    }
    
    // find qi-pair with minimum total degree (in qi-diagram)
    std::pair<int, int> selected_pair = qi_pairs[0];
    int min_total_anti_degree = anti_degree[selected_pair.first] + anti_degree[selected_pair.second];
    
    for (const auto& qi_pair : qi_pairs) {
        int total_anti_degree = anti_degree[qi_pair.first] + anti_degree[qi_pair.second];
        if (total_anti_degree < min_total_anti_degree) {
            min_total_anti_degree = total_anti_degree;
            selected_pair = qi_pair;
        }
    }
    
    return selected_pair;
}

int PartitionOperations::calculateBlockAntiDegree(const std::vector<std::pair<int, int>>& qi_pairs, int block_id) {
    int count = 0;
    for (const auto& qi_pair : qi_pairs) {
        if (qi_pair.first == block_id || qi_pair.second == block_id) {
            count++;
        }
    }
    return count;
}