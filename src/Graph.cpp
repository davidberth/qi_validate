#include "../include/Graph.h"
#include <fstream>
#include <iostream>

Graph::Graph() {
    adjMatrix_ = nullptr;
    num_vertices = 0;
    critical_k = 0;
}

void Graph::init(int vertices) {
    this->num_vertices = vertices;
    adjMatrix_ = new int[vertices * vertices];
    
    // initialize matrix to zero
    for (int i = 0; i < vertices * vertices; ++i) {
        adjMatrix_[i] = 0;
    }
}

Graph::~Graph() {
    if (adjMatrix_) {
        delete[] adjMatrix_;
        adjMatrix_ = nullptr;
    }
}

void Graph::addEdge(int src, int dest) {
    addConnection(src, dest);
    addConnection(dest, src);
}

void Graph::addConnection(int src, int dest) {
    if (src < 0 || src >= num_vertices || dest < 0 || dest >= num_vertices) {
        return;
    }
    adjMatrix_[src * num_vertices + dest] = 1;
    adjMatrix_[dest * num_vertices + src] = 1; 
}

bool Graph::hasEdge(int src, int dest) const {
    return (adjMatrix_[src * num_vertices + dest] > 0);
}

int Graph::getEdgeCount() const {
    int edgeCount = 0;
    for (int i = 0; i < num_vertices; ++i) {
        for (int j = i + 1; j < num_vertices; ++j) {
            if (hasEdge(i, j)) {
                edgeCount++;
            }
        }
    }
    return edgeCount;
}

bool Graph::loadFromFile(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    int n;
    file >> n;
    
    if (n <= 0 || n > 99999) {
        std::cout << "Error: Invalid number of vertices: " << n << std::endl;
        file.close();
        return false;
    }
    
    // initialize graph with n vertices
    init(n);
    
    // read edges
    int src, dest;
    while (file >> src >> dest) {
        if (src >= 0 && src < n && dest >= 0 && dest < n && src != dest) {
            addEdge(src, dest);
        } else {
            std::cout << "Warning: Invalid edge (" << src << ", " << dest << ") ignored" << std::endl;
        }
    }
    
    file.close();
    return true;
}