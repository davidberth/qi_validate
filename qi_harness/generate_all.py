#!/usr/bin/env python3
"""
Generate all test graphs including Robertson configurations.

This script generates:
1. Standard test graphs (cycles, wheels, classic graphs)
2. Robertson configurations (633 graphs from source file)

Usage:
    python generate_all.py [--no-robertson] [--base-dir DIRECTORY]
"""

import argparse
from graphgen import GraphGenerator


def main():
    parser = argparse.ArgumentParser(description="Generate all test graphs for qi validation")
    parser.add_argument("--no-robertson", action="store_true", 
                       help="Skip Robertson configuration generation")
    parser.add_argument("--base-dir", default="../graphs",
                       help="Base directory for graph files (default: ../graphs)")
    
    args = parser.parse_args()
    
    # Create generator
    generator = GraphGenerator()
    
    # Generate full test suite
    include_robertson = not args.no_robertson
    generator.generate_full_test_suite(
        base_dir=args.base_dir, 
        include_robertson=include_robertson
    )
    
    print(f"\nGraph generation complete!")
    if include_robertson:
        print("Generated standard test graphs + 633 Robertson configurations")
    else:
        print("Generated standard test graphs only (Robertson skipped)")


if __name__ == "__main__":
    main()