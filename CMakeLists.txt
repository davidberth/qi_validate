cmake_minimum_required(VERSION 3.20)
project(qi_validator)

# Configuration options
option(VERBOSE_QI_DEBUG "Enable detailed debugging output for qi calculations" ON)
option(VERBOSE_MC_OPERATIONS "Enable Mc operation debugging output" OFF)

# Fetch graph-coloring library for chromatic number computation
include(FetchContent)
FetchContent_Declare(
  graph_coloring
  GIT_REPOSITORY https://github.com/brrcrites/graph-coloring.git
  GIT_TAG        master
)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_GetProperties(graph_coloring)
if(NOT graph_coloring_POPULATED)
  FetchContent_Populate(graph_coloring)
  # Add the graph-coloring source files directly
  set(GRAPH_COLORING_SOURCES
    ${graph_coloring_SOURCE_DIR}/Source/coloring_algorithm.cpp
    ${graph_coloring_SOURCE_DIR}/Source/dsatur.cpp
  )
endif()

add_executable(qi_validate src/Partition.cpp src/Graph.cpp src/McOperations.cpp "src/Main.cpp" ${GRAPH_COLORING_SOURCES})
target_compile_features(qi_validate PRIVATE cxx_std_20)
target_include_directories(qi_validate PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${graph_coloring_SOURCE_DIR}/Header)

# Pass configuration options as compile definitions
if(VERBOSE_QI_DEBUG)
    target_compile_definitions(qi_validate PRIVATE VERBOSE_QI_DEBUG=1)
else()
    target_compile_definitions(qi_validate PRIVATE VERBOSE_QI_DEBUG=0)
endif()

if(VERBOSE_MC_OPERATIONS)
    target_compile_definitions(qi_validate PRIVATE VERBOSE_MC_OPERATIONS=1)
else()
    target_compile_definitions(qi_validate PRIVATE VERBOSE_MC_OPERATIONS=0)
endif()
