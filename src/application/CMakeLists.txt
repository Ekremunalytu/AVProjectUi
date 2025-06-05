# Application Layer - Business logic and workflows
add_library(Application STATIC
    # This layer will contain business logic
    # For now, it's empty but ready for future expansion
    services/README.md
)

# Create placeholder files
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/services/README.md "# Application Services\n\nThis directory will contain business logic services.\n")

# Set as interface library for now
set_target_properties(Application PROPERTIES 
    LINKER_LANGUAGE CXX
)

target_include_directories(Application
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(Application
    PUBLIC 
        Core
        Storage
        Security
        Infrastructure
)
