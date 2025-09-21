#!/usr/bin/env python3
"""
Main script for qi_harness - Graph generator for qi validation testing.
"""

from qi_harness.robertson import GraphGenerator


def generate_graphs():
    """Generate test graphs for qi validation."""
    print("qi_harness: Generating organized test graphs for Hadwiger's conjecture validation")
    
    generator = GraphGenerator()
    generator.generate_test_graphs("graphs")
    generator.generate_robertson_placeholders("graphs")
    
    print("\nGenerated organized test graph structure:")
    print("  graphs/special/        - Special graphs (Petersen, K5, etc.)")
    print("  graphs/procedural/     - Procedurally generated graphs")
    print("  graphs/robertson/      - Robertson configurations (placeholder)")
    print("\nExample usage:")
    print("  ./build/Debug/qi_validate.exe graphs/special/petersen.txt")
    print("  ./build/Debug/qi_validate.exe graphs/procedural/cycles/cycle_9.txt")
    print("  ./build/Debug/qi_validate.exe graphs/test.txt  # Quick test")

if __name__ == "__main__":
    generate_graphs()
