# Unit Tests
enable_testing()

# Core layer tests
add_executable(CoreTests
    core/test_api_key.cpp
    core/test_compile.cpp
)
target_link_libraries(CoreTests 
    PRIVATE Core Security Infrastructure Storage Qt::Test Qt::Core
)
add_test(NAME CoreTests COMMAND CoreTests)

# Security layer tests (former CDR tests)
file(GLOB SECURITY_TEST_FILES security/*.cpp)
if(SECURITY_TEST_FILES)
    add_executable(SecurityTests ${SECURITY_TEST_FILES})
    target_link_libraries(SecurityTests 
        PRIVATE Security Qt::Test Qt::Core
    )
    add_test(NAME SecurityTests COMMAND SecurityTests)
endif()

# Storage layer tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/storage/test_storage.cpp")
add_executable(StorageTests
    storage/test_storage.cpp
)
set_target_properties(StorageTests PROPERTIES 
    LINKER_LANGUAGE CXX
)
target_link_libraries(StorageTests 
    PRIVATE Storage Qt::Test Qt::Core
)
add_test(NAME StorageTests COMMAND StorageTests)
endif()

# Infrastructure layer tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/infrastructure/test_infrastructure.cpp")
add_executable(InfrastructureTests
    infrastructure/test_infrastructure.cpp
)
set_target_properties(InfrastructureTests PROPERTIES 
    LINKER_LANGUAGE CXX
)
target_link_libraries(InfrastructureTests 
    PRIVATE Infrastructure Qt::Test Qt::Core
)
add_test(NAME InfrastructureTests COMMAND InfrastructureTests)
endif()

# Presentation layer tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/presentation/test_presentation.cpp")
add_executable(PresentationTests
    presentation/test_presentation.cpp
)
set_target_properties(PresentationTests PROPERTIES 
    LINKER_LANGUAGE CXX
)
target_link_libraries(PresentationTests 
    PRIVATE Presentation Qt::Test Qt::Widgets
)
add_test(NAME PresentationTests COMMAND PresentationTests)
endif()
