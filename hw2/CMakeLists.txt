cmake_minimum_required(VERSION 2.8)

# ALL OF THE SPECIFIED NAMES IN THIS CMAKE ARE REQUIRED.
# Set the project name variable to hw2.
# THIS IS REQUIRED

project(hw2)

# Add your CMAKE_C_FLAGS AND CMAKE_CXX_FLAGS here
# Make SURE to use -std=c11 and -std=c++11 where relevant.
# The options specified below are required, but additional options
#  may be used.

set(CMAKE_C_FLAGS "-std=c11 -g -Wall -Wextra -Wshadow -Werror")
set(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall -Wextra -Wshadow -Werror")

# Add our include directory to CMake's search paths
# THIS IS REQUIRED

include_directories(${CMAKE_SOURCE_DIR}/include)

# Create library from dyn_array so we can use it later

add_library(dyn_array src/dyn_array.c)

# Compile the analysis executable

add_executable(analysis_code src/analysis.c ../src/process_scheduling.c ../src/dyn_array.c)

# link the dyn_array library we compiled against our analysis executable

target_link_libraries(analysis_code dyn_array)

# Compile the tester executable
add_executable(hw2_test test/tests.cpp ../src/process_scheduling.c ../src/dyn_array.c)

target_compile_definitions(hw2_test PRIVATE)

# Link ${PROJECT_NAME}_test with dyn_array and gtest and pthread libraries
target_link_libraries(hw2_test gtest pthread dyn_array)
