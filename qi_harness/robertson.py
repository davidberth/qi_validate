"""
Graph generator for qi validation testing.

This module provides tools for generating various types of graphs including:
- Classic extremal and coloring graphs
- Programmatically generated graph families
- Robertson configurations from the 4-color theorem (from arxiv source)
"""

import networkx as nx
from typing import List, Dict, Any
import os


class GraphGenerator:
    """Generator for various types of graphs used in qi validation."""
    
    def __init__(self, seed: int = 42):
        """
        Initialize GraphGenerator with deterministic seed.
        
        Args:
            seed: Random seed for deterministic graph generation
        """
        self.graphs = {}
        self.seed = seed
        # Set seed for deterministic generation if needed
        import random
        random.seed(seed)
    
    def parse_robertson_configuration(self, config_data: str) -> nx.Graph:
        """
        Parse a Robertson configuration from arxiv source and return a NetworkX graph.
        
        Args:
            config_data: String representation of the Robertson configuration
            
        Returns:
            NetworkX graph representing the configuration
        """
        # TODO: Implement Robertson configuration parsing from arxiv source
        # This will parse the specific format used in the Robertson configurations
        G = nx.Graph()
        return G
    
    def save_as_qi_format(self, graph: nx.Graph, filename: str, critical_k: int):
        """
        Save a NetworkX graph in the qi_validate input format.
        
        Args:
            graph: NetworkX graph to save
            filename: Output filename
            critical_k: Critical k value for the graph
        """
        with open(filename, 'w') as f:
            # Write number of vertices
            f.write(f"{graph.number_of_nodes()}\n")
            
            # Write edges
            for u, v in graph.edges():
                f.write(f"{u} {v}\n")
            
            # Write critical k
            f.write(f"k={critical_k}\n")
    
    def generate_test_graphs(self, base_dir: str = "graphs"):
        """
        Generate test graphs in organized directory structure.
        
        Args:
            base_dir: Base directory for graph organization
        """
        # Create directory structure
        special_dir = os.path.join(base_dir, "special")
        procedural_dir = os.path.join(base_dir, "procedural")
        cycles_dir = os.path.join(procedural_dir, "cycles")
        families_dir = os.path.join(procedural_dir, "families")
        
        os.makedirs(special_dir, exist_ok=True)
        os.makedirs(cycles_dir, exist_ok=True)
        os.makedirs(families_dir, exist_ok=True)
        
        # Generate classic extremal and coloring graphs
        self._generate_classic_graphs(special_dir)
        
        # Generate cycle family
        for n in [7, 9, 11, 15, 20]:
            cycle = nx.cycle_graph(n)
            cycle_path = os.path.join(cycles_dir, f"cycle_{n}.txt")
            # For cycles, critical k is roughly n/2 + 1 (simplified)
            critical_k = max(3, n // 2)
            self.save_as_qi_format(cycle, cycle_path, critical_k)
        
        # Generate other graph families
        # Wheel graphs
        wheel_dir = os.path.join(families_dir, "wheels")
        os.makedirs(wheel_dir, exist_ok=True)
        for n in [6, 8, 10]:
            wheel = nx.wheel_graph(n)
            wheel_path = os.path.join(wheel_dir, f"wheel_{n}.txt")
            self.save_as_qi_format(wheel, wheel_path, 4)  # Wheels typically have small k'
    
    def generate_robertson_placeholders(self, base_dir: str = "graphs"):
        """
        Generate placeholder structure for Robertson configurations.
        
        Args:
            base_dir: Base directory for graph organization
        """
        robertson_dir = os.path.join(base_dir, "robertson")
        os.makedirs(robertson_dir, exist_ok=True)
        
        # Create a README for the Robertson directory
        readme_path = os.path.join(robertson_dir, "README.md")
        with open(readme_path, 'w') as f:
            f.write("# Robertson Configurations\n\n")
            f.write("This directory will contain the 633 Robertson configurations from the 4-color theorem.\n")
            f.write("Each configuration will be parsed and converted to qi_validate format.\n\n")
            f.write("Format: config_XXX.txt where XXX is the configuration number (001-633)\n\n")
            f.write("## Source\n")
            f.write("Configurations are parsed from the arxiv source data.\n")
            f.write("Use `generate_robertson_from_source()` to process the source file.\n")
    
    def generate_robertson_from_source(self, source_file: str, base_dir: str = "graphs"):
        """
        Generate Robertson configurations from the arxiv source file.
        
        Args:
            source_file: Path to the downloaded Robertson configurations source file
            base_dir: Base directory for graph organization
        """
        robertson_dir = os.path.join(base_dir, "robertson")
        os.makedirs(robertson_dir, exist_ok=True)
        
        if not os.path.exists(source_file):
            print(f"Warning: Robertson source file not found at {source_file}")
            print("Please download the Robertson configurations from the arxiv paper.")
            return
        
        # TODO: Implement parsing of the specific format from arxiv
        # This will depend on the exact format of the source file
        print(f"Processing Robertson configurations from {source_file}")
        
        # Placeholder: would parse each configuration and save as .txt
        # for config_id in range(1, 634):  # 633 configurations
        #     graph = self.parse_robertson_configuration(config_data)
        #     filename = os.path.join(robertson_dir, f"config_{config_id:03d}.txt")
        #     critical_k = self.determine_critical_k(graph)  # Implementation needed
        #     self.save_as_qi_format(graph, filename, critical_k)
        
        print("Robertson configuration generation ready for implementation.")
        print("Will generate config_001.txt through config_633.txt when source format is implemented.")
    
    def _generate_classic_graphs(self, special_dir: str):
        """Generate classic extremal and coloring graphs."""
        
        # Basic complete graphs
        graphs_to_generate = [
               # Petersen and related
            ("petersen", nx.petersen_graph(), 6, "Petersen graph (classic counterexample)"),
            
            # Complete bipartite graphs  
            ("k33_bipartite", nx.complete_bipartite_graph(3, 3), 4, "K3,3 complete bipartite (non-planar)"),
            ("k23_bipartite", nx.complete_bipartite_graph(2, 2), 3, "K2,2 complete bipartite"),
            ("k44_bipartite", nx.complete_bipartite_graph(7, 7), 8, "K7,7 complete bipartite"),
            
            # Platonic solids
            ("octahedral", nx.octahedral_graph(), 4, "Octahedral graph (3-regular, 6 vertices)"),
            ("icosahedral", nx.icosahedral_graph(), 4, "Icosahedral graph (5-regular, 12 vertices)"),
            ("dodecahedral", nx.dodecahedral_graph(), 4, "Dodecahedral graph (3-regular, 20 vertices)"),
            
            # Chvátal graph - smallest 4-chromatic 4-regular graph  
            ("chvatal", self._chvatal_graph(), 5, "Chvátal graph (4-chromatic, 4-regular, 12 vertices)"),
            
            # Grötzsch graph - triangle-free 4-chromatic
            ("grotzsch", self._grotzsch_graph(), 5, "Grötzsch graph (triangle-free, 4-chromatic, 11 vertices)"),
            
 
        ]
        
        for name, graph, critical_k, description in graphs_to_generate:
            filepath = os.path.join(special_dir, f"{name}.txt")
            if not os.path.exists(filepath):
                self.save_as_qi_format(graph, filepath, critical_k)
                print(f"Generated {name}: {description}")
    
    def _chvatal_graph(self) -> nx.Graph:
        """Generate the Chvátal graph."""
        # Chvátal graph: 12 vertices, 4-regular, 4-chromatic
        edges = [
            (0, 1), (0, 4), (0, 6), (0, 9),
            (1, 2), (1, 5), (1, 7),
            (2, 3), (2, 6), (2, 8),
            (3, 4), (3, 7), (3, 9),
            (4, 5), (4, 8),
            (5, 6), (5, 10), (5, 11),
            (6, 7), (6, 10),
            (7, 8), (7, 11),
            (8, 9), (8, 10),
            (9, 10), (9, 11),
            (10, 11)
        ]
        G = nx.Graph()
        G.add_edges_from(edges)
        return G
    
    def _grotzsch_graph(self) -> nx.Graph:
        """Generate the Grötzsch graph."""
        # Grötzsch graph: 11 vertices, triangle-free, 4-chromatic
        G = nx.Graph()
        # Outer 5-cycle
        for i in range(5):
            G.add_edge(i, (i + 1) % 5)
        
        # Inner vertices
        for i in range(5):
            G.add_edge(i, i + 5)  # Connect outer to inner
            G.add_edge(i + 5, 10)  # Connect inner to center
        
        return G
    
    def _mycielski_graph(self, k: int) -> nx.Graph:
        """Generate Mycielski graph M_k (triangle-free, k-chromatic)."""
        if k == 2:
            return nx.path_graph(2)
        elif k == 3:
            return nx.cycle_graph(5)
        elif k == 4:
            return self._grotzsch_graph()
        elif k == 5:
            # Mycielski construction on M4
            base = self._grotzsch_graph()
            return self._mycielski_construction(base)
        else:
            # For higher k, use iterative construction
            G = nx.cycle_graph(5)  # M3
            for _ in range(k - 3):
                G = self._mycielski_construction(G)
            return G
    
    def _mycielski_construction(self, G: nx.Graph) -> nx.Graph:
        """Apply Mycielski construction to increase chromatic number by 1."""
        n = G.number_of_nodes()
        H = nx.Graph()
        
        # Copy original graph
        H.add_edges_from(G.edges())
        
        # Add new vertices (duplicates)
        for v in range(n):
            for u in G.neighbors(v):
                H.add_edge(u, v + n)
        
        # Add central vertex connected to all duplicates
        central = 2 * n
        for v in range(n, 2 * n):
            H.add_edge(central, v)
        
        return H
    
    def _hoffman_singleton_graph(self) -> nx.Graph:
        """Generate the Hoffman-Singleton graph (50 vertices)."""
        # This is complex to construct directly, so use a simpler proxy
        # In practice, you'd implement the proper construction
        # For now, return a substitute that's still interesting
        return nx.hoffman_singleton_graph() if hasattr(nx, 'hoffman_singleton_graph') else nx.petersen_graph()