# Tests - Layer-based test organization
cmake_minimum_required(VERSION 3.19)

# Find Qt Test module
find_package(Qt6 REQUIRED COMPONENTS Test)

# Unit tests for each layer
add_subdirectory(unit)

# Integration tests
add_subdirectory(integration)

# Test data and fixtures
file(COPY fixtures/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/fixtures/)
