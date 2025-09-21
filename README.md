# qi_validate: Computational Validation for Hadwiger's Conjecture
## David Berthiaume
A computational framework to validate key properties in a proof related to Hadwiger's conjecture. This tool verifies that **qi e k - k' + 1** throughout chains of Mc operations (merging connected blocks in graph partitions) from P* to the critical region at k'.

## Quick Start

### 1. Build the C++ Validator

**Requirements:**
- CMake 3.20+
- C++20 compatible compiler
- Git (for external dependencies)

**Build:**
```bash
# Configure and build
cmake -B out/build/x64-Release -S .
cmake --build out/build/x64-Release
```

### 2. Generate Test Graphs

**Requirements:**
- Python 3.8+
- uv package manager (`pip install uv`)

**Generate all graphs (including 633 Robertson configurations):**
```bash
python generate_graphs.py
```

This creates:
- `graphs/special/` - Classic extremal graphs (Petersen, platonic solids, etc.)
- `graphs/procedural/` - Graph families (cycles, wheels)  
- `graphs/robertson/` - 633 Robertson configurations (config_001.txt - config_633.txt)

### 3. Run a single validation

**Single graph validation:**
```bash
./out/build/x64-Release/qi_validate.exe graphs/special/petersen.txt
./out/build/x64-Release/qi_validate.exe graphs/robertson/config_001.txt
```

**Comprehensive test suite:**
```bash
python test_runner.py
```

This runs validation on all graphs and generates a summary report.

## What It Validates

The framework validates the theoretical guarantee:
- **qi e k - k' + 1** at each step of Mc operations
- **qi-number**: Maximum sum of disjoint independent block sets in quotient graph
- **k**: Current number of blocks in partition
- **k'**: Critical threshold where all connected partitions become q-incomplete

## Algorithm Features

- **Fast computation**: Uses DSATUR chromatic number algorithm for large graphs (>15 blocks)
- **Exact computation**: Uses exhaustive backtracking for small graphs (d15 blocks)  
- **Graceful handling**: Returns "UNDETERMINED" for computationally intensive cases
- **Early stopping**: Optimized to stop when qi threshold is met
- **Safety limits**: Automatically skips graphs >30 vertices to prevent crashes

## Test Results

The test runner categorizes results as:
- **PASS**: qi e k - k' + 1 throughout the entire process
- **PARTIAL**: Validation completed but some qi values undetermined  
- **FAIL**: qi fell below required threshold
- **SKIPPED**: Graph too large (>30 vertices)
- **TIMEOUT**: Validation exceeded 60-second limit
- **ERROR**: Unexpected failure
