"""
Graph generator for qi validation testing.
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
            # For cycles of length ≥ 5, critical k = 4
            # (cycles are 3-colorable for odd n, 2-colorable for even n, 
            # but in Hadwiger context k' = 4 for cycles ≥ 5)
            critical_k = 4 if n >= 5 else 3
            self.save_as_qi_format(cycle, cycle_path, critical_k)
        
        # Generate other graph families
        # Wheel graphs
        wheel_dir = os.path.join(families_dir, "wheels")
        os.makedirs(wheel_dir, exist_ok=True)
        for n in [6, 8, 10]:
            wheel = nx.wheel_graph(n)
            wheel_path = os.path.join(wheel_dir, f"wheel_{n}.txt")
            # Wheel graph with n vertices has k' = n 
            # (merging center with any rim vertex gives K_{n-1} minor)
            self.save_as_qi_format(wheel, wheel_path, n)
    
    def generate_full_test_suite(self, base_dir: str = "graphs", include_robertson: bool = True):
        """
        Generate the complete test suite including Robertson configurations.
        
        Args:
            base_dir: Base directory for graph organization
            include_robertson: Whether to generate Robertson configurations
        """
        print("Generating complete test suite...")
        
        # Generate the standard test graphs
        self.generate_test_graphs(base_dir)
        
        # Generate Robertson configurations if requested
        if include_robertson:
            print("\nGenerating Robertson configurations...")
            self.generate_robertson_from_source(base_dir=base_dir)
        
        print("\nTest suite generation complete!")
    
    def generate_robertson_placeholders(self, base_dir: str = "graphs"):
        """
        Generate placeholder structure for Robertson configurations.
        
        Args:
            base_dir: Base directory for graph organization
        """
        robertson_dir = os.path.join(base_dir, "robertson")
        os.makedirs(robertson_dir, exist_ok=True)
        
       
    
    def generate_robertson_from_source(self, source_file: str = "robertson/source.txt", base_dir: str = "graphs"):
        """
        Generate Robertson configurations from the source file using parse_robertson.py.
        
        Args:
            source_file: Path to the Robertson configurations source file
            base_dir: Base directory for graph organization
        """
        from .parse_robertson import parse_robertson_file
        
        robertson_dir = os.path.join(base_dir, "robertson")
        os.makedirs(robertson_dir, exist_ok=True)
        
        if not os.path.exists(source_file):
            print(f"Warning: Robertson source file not found at {source_file}")
            print("Please place the Robertson source file at robertson/source.txt")
            print(f"Current working directory: {os.getcwd()}")
            print(f"Looking for: {os.path.abspath(source_file)}")
            return
        
        print(f"Parsing Robertson configurations from {source_file}...")
        
        try:
            # Parse all configurations from source file
            configurations = parse_robertson_file(source_file)
            
            print(f"Found {len(configurations)} Robertson configurations")
            
            if len(configurations) == 0:
                print("ERROR: No configurations were parsed from the source file")
                return
            
            # Generate graph files for each configuration
            for i, config in enumerate(configurations, 1):
                config_name = f"config_{i:03d}"
                config_file = os.path.join(robertson_dir, f"{config_name}.txt")
                
                # Convert configuration to NetworkX graph
                graph = self._robertson_config_to_graph(config)
                
                # Save with k'=5 as specified
                self.save_as_qi_format(graph, config_file, critical_k=5)
            
            print(f"Successfully generated {len(configurations)} Robertson configuration files")
            
        except Exception as e:
            print(f"Error processing Robertson configurations: {e}")
            print("Please check the source file format and parse_robertson.py implementation")
    
    def _robertson_config_to_graph(self, config):
        """Convert a Robertson configuration dict to a NetworkX graph."""
        import networkx as nx
        
        # Create graph with specified number of vertices
        # Check both possible keys for vertex count
        n = config.get('vertices', config.get('n', 0))
        graph = nx.Graph()
        graph.add_nodes_from(range(n))
        
        # Add edges from the configuration
        edges = config.get('edges', [])
        for edge in edges:
            if len(edge) == 2:
                u, v = edge
                if 0 <= u < n and 0 <= v < n and u != v:
                    graph.add_edge(u, v)
        
        return graph
    
    def _generate_classic_graphs(self, special_dir: str):
        """Generate classic extremal and coloring graphs."""
        
        # Classic extremal graphs
        graphs_to_generate = [
            # Petersen and related
            ("petersen", nx.petersen_graph(), 6, "Petersen graph (classic counterexample)"),
            
            # Platonic solids
            ("octahedral", nx.octahedral_graph(), 5, "Octahedral graph (3-regular, 6 vertices)"),
            ("icosahedral", nx.icosahedral_graph(), 6, "Icosahedral graph (5-regular, 12 vertices)"),
            ("dodecahedral", nx.dodecahedral_graph(), 5, "Dodecahedral graph (3-regular, 20 vertices)"),
            
            
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