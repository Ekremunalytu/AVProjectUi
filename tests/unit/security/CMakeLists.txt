# Security Unit Tests

add_executable(security_tests
    YaraRuleManagerTest.cpp
    SandboxManagerTest.cpp
)

target_link_libraries(security_tests
    PRIVATE
        Security
        Core
        gtest
        gtest_main
        gmock
        gmock_main
)

target_include_directories(security_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests
)

# Add the test to CTest
add_test(
    NAME SecurityTests
    COMMAND security_tests
)

# Set test properties
set_tests_properties(SecurityTests PROPERTIES
    TIMEOUT 30
    LABELS "unit;security"
)
