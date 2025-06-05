# Integration Tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_integration.cpp")
add_executable(IntegrationTests
    test_integration.cpp
)

set_target_properties(IntegrationTests PROPERTIES 
    LINKER_LANGUAGE CXX
)

target_link_libraries(IntegrationTests 
    PRIVATE 
        Core
        Storage
        Security
                Infrastructure
        Application
        Presentation
        Qt::Test 
        Qt::Core
        Qt::Widgets
)

add_test(NAME IntegrationTests COMMAND IntegrationTests)
endif()
