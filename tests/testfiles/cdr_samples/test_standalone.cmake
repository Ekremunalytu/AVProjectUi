cmake_minimum_required(VERSION 3.16)

# Add the test executable
add_executable(test_cdr_standalone test_cdr_standalone.cpp)

# Link Qt libraries
target_link_libraries(test_cdr_standalone 
    Qt6::Core 
    Qt6::Widgets 
    Security
    Core
    Infrastructure
)

# Set C++ standard
target_compile_features(test_cdr_standalone PRIVATE cxx_std_17)

# Include directories
target_include_directories(test_cdr_standalone PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)
