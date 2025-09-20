#pragma once

class Graph {
public:
    Graph();
    void init(int vertices);
    ~Graph();
    void addEdge(int src, int dest);
    void addConnection(int src, int dest);
    bool hasEdge(int src, int dest) const;
    int getEdgeCount() const;
    bool loadFromFile(const char* filename);
    int* getAdjMatrix() const { return adjMatrix_; }
    
    int num_vertices;
    int critical_k;

private:
    int* adjMatrix_;
};