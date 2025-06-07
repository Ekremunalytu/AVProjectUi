# AVProjectUi - Layer-Based Architecture
cmake_minimum_required(VERSION 3.19)

if(APPLE)
  set(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

project(AVProjectUi VERSION 1.0.0 LANGUAGES CXX)

# C++ standard settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Disable internationalization, forcing English
add_definitions(-DQT_NO_TRANSLATION)

# Qt path configuration
if(APPLE)
  set(CMAKE_PREFIX_PATH "/Volumes/Crucial/QT/6.10.0/macos")
elseif(WIN32)
  set(CMAKE_PREFIX_PATH "C:/Qt/6.9.0/mingw_64")
endif()

# Find required Qt packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Network)

# Enable automatic handling of Qt-specific files
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/src/presentation)

qt_standard_project_setup()

# Global include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}
)

# Add subdirectories (Layer-based)
add_subdirectory(src/core)
add_subdirectory(src/storage)
add_subdirectory(src/security)
add_subdirectory(src/infrastructure)
add_subdirectory(src/application)
add_subdirectory(src/presentation)

# Main executable
qt_add_executable(AVProjectUi
    WIN32 MACOSX_BUNDLE
    src/presentation/application/main.cpp
)

# Link all layers
target_link_libraries(AVProjectUi
    PRIVATE
        Core
        Storage
        Security
        Infrastructure
        Application
        Presentation
        Qt::Core
        Qt::Widgets
        Qt::Sql
        Qt::Network
)

# Enable testing
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
include(GNUInstallDirs)
install(TARGETS AVProjectUi
    BUNDLE  DESTINATION AVProjectUi
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET AVProjectUi
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

# Copy config file
add_custom_command(TARGET AVProjectUi POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/src/core/config/config.ini
    $<TARGET_FILE_DIR:AVProjectUi>/config.ini
    COMMENT "Copying config.ini to executable directory"
)

# Copy YARA rules to executable directory
add_custom_command(TARGET AVProjectUi POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_BINARY_DIR}/yara_rules
    $<TARGET_FILE_DIR:AVProjectUi>/yara_rules
    COMMENT "Copying YARA rules to executable directory"
)

# Post-build automation: Documentation only (cleanup removed to preserve executable)
add_custom_command(TARGET AVProjectUi POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "📚 Generating documentation..."
    COMMAND bash ${CMAKE_SOURCE_DIR}/tools/scripts/docs.sh generate || echo "Documentation generation completed"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating project documentation"
)

# Documentation targets
find_program(DOXYGEN_EXECUTABLE doxygen)
find_program(PYTHON_EXECUTABLE python3)

if(DOXYGEN_EXECUTABLE)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/docs/Doxyfile.main
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
    
    if(PYTHON_EXECUTABLE)
        add_custom_target(serve-docs
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/scripts/serve_docs.py
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Starting documentation server"
            VERBATIM
        )
        
        add_custom_target(docs-serve
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/docs/Doxyfile.main
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/scripts/serve_docs.py
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs
            COMMENT "Generating documentation and starting server"
            VERBATIM
        )
    else()
        message(WARNING "Python3 not found. Documentation server targets will not be available.")
    endif()
else()
    message(WARNING "Doxygen not found. Documentation targets will not be available.")
endif()

# Note: test_text_sanitization.cpp is now part of the organized test structure in tests/unit/
