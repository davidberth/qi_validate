#!/usr/bin/env python3
"""
Test Robertson configuration parsing.
"""

from parse_robertson import parse_robertson_file
import os

def test_robertson_parsing():
    source_file = "robertson/source.txt"
    
    if not os.path.exists(source_file):
        print(f"ERROR: Source file not found at {source_file}")
        return
    
    print(f"Testing Robertson parsing from {source_file}...")
    
    try:
        configurations = parse_robertson_file(source_file)
        print(f"Successfully parsed {len(configurations)} configurations")
        
        if configurations:
            # Show first few configurations
            for i, config in enumerate(configurations[:3]):
                print(f"Config {i+1}: {config['name']} - {config['vertices']} vertices, {len(config['edges'])} edges")
        
    except Exception as e:
        print(f"ERROR: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    test_robertson_parsing()