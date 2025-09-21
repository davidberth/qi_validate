#!/usr/bin/env python3
"""
Parse Robertson configuration files and convert to simple graph format.
"""

import os
import re

def parse_robertson_file(filename):
    """Parse the Robertson source file and extract configurations."""
    configurations = []
    
    with open(filename, 'r') as f:
        lines = f.readlines()
    
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        
        # Skip empty lines
        if not line:
            i += 1
            continue
        
        # Try to identify configuration start by looking for the pattern:
        # Line 1: name (could be numeric like "0.7322" or "2.122")
        # Line 2: n r a b format (4 integers)
        # Line 3: edge data (starts with number or " 0")
        
        # Check if this could be a configuration name
        # Look ahead to see if next line has 4 integers (n r a b format)
        if i + 1 < len(lines):
            next_line = lines[i + 1].strip()
            params = next_line.split()
            
            if len(params) >= 4:
                try:
                    # Try to parse as n r a b
                    n = int(params[0])  # number of vertices
                    r = int(params[1])  # ring size  
                    a = int(params[2])  # cardinality of C
                    b = int(params[3])  # cardinality of C'
                    
                    # This looks like a valid configuration
                    name = line
                    i += 2  # Skip name and params lines
                    
                    # Next line has additional edges: count followed by vertex pairs
                    if i >= len(lines):
                        break
                        
                    additional_edges_line = lines[i].strip()
                    i += 1
                    
                    # Parse additional edges (may be "0" if none)
                    additional_edges = list()
                    if additional_edges_line and additional_edges_line != '0':
                        parts = additional_edges_line.split()
                        if len(parts) > 0:
                            edge_count = int(parts[0])
                            # Parse edge pairs: v1 v2 v3 v4 ... (pairs)
                            for j in range(1, len(parts), 2):
                                if j + 1 < len(parts):
                                    v1 = int(parts[j]) - 1    # Convert to 0-based
                                    v2 = int(parts[j + 1]) - 1  # Convert to 0-based
                                    # Add in canonical form
                                    if v1 < v2:
                                        additional_edges.append((v1, v2))
                                    else:
                                        additional_edges.append((v2, v1))
                    
                    # Now parse adjacency lists until we hit big numbers (coordinates)
                    edges = list()  # Use set to avoid duplicate edges
                    
                    while i < len(lines):
                        line = lines[i].strip()
                        if not line:
                            break
                            
                        # Parse adjacency list line: "ignore_index source_vertex endpoint1 endpoint2 ..."
                        parts = line.split()
                        if len(parts) >= 3:  # Need at least ignore, source, and one endpoint
                            try:
                                # Skip first number (index), get source vertex  
                                source_vertex = int(parts[0])
                                ignore_index = int(parts[1])
                                
                                # Check if we hit the coordinate section (big numbers > 10000)
                                if ignore_index > 10000 or source_vertex > 10000:
                                    break
                                    
                                # Extract endpoints (all parts after source vertex)
                                endpoints = []
                                for j in range(2, len(parts)):
                                    endpoint = int(parts[j])
                                    if endpoint > 10000:  # Hit coordinates, stop
                                        break
                                    endpoints.append(endpoint)
                                
                                # Add edges (convert from 1-based to 0-based)
                                for endpoint in endpoints:
                                    if source_vertex <= n and endpoint <= n:  # Valid vertices within vertex count
                                        v1 = source_vertex - 1  # Convert to 0-based
                                        v2 = endpoint - 1  # Convert to 0-based
                                        # Debug: print problematic edges
                                        if v1 < 0 or v1 >= n or v2 < 0 or v2 >= n:
                                            print(f"WARNING: Invalid edge ({source_vertex}->{endpoint}) converts to ({v1}->{v2}) with n={n}")
                                        # Skip self-loops
                                        elif v1 != v2:
                                            # Add edge in canonical form (smaller vertex first)
                                            if v1 < v2:
                                                edges.append((v1, v2))
                                            else:
                                                edges.append((v2, v1))
                                            
                            except ValueError:
                                # Hit non-numeric data, probably coordinates
                                break
                        else:
                            break
                        
                        i += 1
                    
                    # Combine adjacency list edges with additional edges
                    edges.extend(additional_edges)
                    edges = list(set(edges))
                    
                    
                    configurations.append({
                        'name': name,
                        'vertices': n,
                        'edges': edges
                    })
                    
                    # Skip ahead to next configuration (past adjacency lists and coordinates)
                    i += 1
                    while i < len(lines):
                        line = lines[i].strip()
                        if not line:
                            # Found empty line, next configuration starts after this
                            i += 1
                            break
                        i += 1
                    
                except ValueError:
                    # Not a valid parameter line, skip
                    i += 1
            else:
                i += 1
        else:
            i += 1
    
    return configurations

def write_graph_file(config, output_dir):
    """Write a single configuration to a graph file."""
    filename = os.path.join(output_dir, f"{config['name']}.txt")
    
    with open(filename, 'w') as f:
        # First line: number of vertices
        f.write(f"{config['vertices']}\n")
        
        # Each subsequent line: one edge as two 0-based indices
        for v1, v2 in config['edges']:
            f.write(f"{v1} {v2}\n")
    
    print(f"Written {config['name']}.txt: {config['vertices']} vertices, {len(config['edges'])} edges")

def main():
    source_file = "robertson/source.txt"
    output_dir = "robertson/graphs"
    
    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    print("Parsing source file...")
    configurations = parse_robertson_file(source_file)
    
    print(f"Found {len(configurations)} configurations")
    
    # Process ALL configurations 
    for i, config in enumerate(configurations):
        if i < 10:  # Show progress for first 10
            print(f"Processing config {i+1}: {config['name']}")
        write_graph_file(config, output_dir)
    
    print(f"Processed all {len(configurations)} configurations")

if __name__ == "__main__":
    main()