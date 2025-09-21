#!/usr/bin/env python3
"""
Main script for qi_harness - Graph generator for qi validation testing.
"""

from qi_harness.graphgen import GraphGenerator


def generate_graphs():
    """Generate test graphs for qi validation."""
    print("qi_harness: Generating organized test graphs for Hadwiger's conjecture validation")
    
    generator = GraphGenerator()
    
    # Generate the full test suite including Robertson configurations
    generator.generate_full_test_suite("graphs", include_robertson=True)
    
    print("\nExample usage:")
    print("  ./build/Debug/qi_validate.exe graphs/special/petersen.txt")
    print("  ./build/Debug/qi_validate.exe graphs/procedural/cycles/cycle_9.txt")
    print("  ./build/Debug/qi_validate.exe graphs/robertson/config_001.txt")

if __name__ == "__main__":
    generate_graphs()
