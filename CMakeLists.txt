cmake_minimum_required(VERSION 3.20)
project(qi_validator)

# Configuration options
option(VERBOSE_QI_DEBUG "Enable detailed debugging output for qi calculations" OFF)
option(VERBOSE_MC_OPERATIONS "Enable Mc operation debugging output" OFF)

add_executable(qi_validate src/Partition.cpp src/Graph.cpp src/McOperations.cpp "src/Main.cpp")
target_compile_features(qi_validate PRIVATE cxx_std_20)
target_include_directories(qi_validate PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

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
