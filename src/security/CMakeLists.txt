# Security Layer - All security operations (CDR, Scanning, Sandbox)
add_library(Security STATIC
    # CDR (Content Disarm & Reconstruction) - All CDR functionality together
    cdr/CdrManager.cpp
    cdr/CdrManager.h
    cdr/CdrSanitizer.cpp
    cdr/CdrSanitizer.h
    cdr/CdrTypes.h
    cdr/CDRScanner.cpp
    cdr/CDRScanner.h
    cdr/FileSanitizer.h
    cdr/FilesystemCompat.cpp
    cdr/FilesystemCompat.h
    
    # Monitoring
    monitoring/FileSystemWatcher.cpp
    monitoring/FileSystemWatcher.h
    
    # Scanning
    scanning/BasicScanner.cpp
    scanning/BasicScanner.h
    
    # YARA Integration
    scanning/yara/YaraRuleManager.cpp
    scanning/yara/YaraRuleManager.h
    
    # Sandbox
    sandbox/SandboxManager.cpp
    sandbox/SandboxManager.h
    sandbox/SandboxScanner.cpp
    sandbox/SandboxScanner.h
    sandbox/SandboxTypes.h
)

target_include_directories(Security
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# Find YARA library
find_library(YARA_LIBRARY NAMES yara PATHS /opt/homebrew/lib /usr/local/lib)
find_path(YARA_INCLUDE_DIR NAMES yara.h PATHS /opt/homebrew/include /usr/local/include)

if(NOT YARA_LIBRARY)
    message(FATAL_ERROR "YARA library not found")
endif()

if(NOT YARA_INCLUDE_DIR)
    message(FATAL_ERROR "YARA headers not found")
endif()

target_include_directories(Security
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${YARA_INCLUDE_DIR}
)

target_link_libraries(Security
    PUBLIC 
        Core
        Storage
        Qt::Core
    PRIVATE
        ${YARA_LIBRARY}
)

# YARA Rules Integration
set(YARA_RULES_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scanning/yara)
set(YARA_RULES_INSTALL_DIR ${CMAKE_BINARY_DIR}/yara_rules)

# Create yara rules directory in build
file(MAKE_DIRECTORY ${YARA_RULES_INSTALL_DIR})
file(MAKE_DIRECTORY ${YARA_RULES_INSTALL_DIR}/rules)
file(MAKE_DIRECTORY ${YARA_RULES_INSTALL_DIR}/maldocs)

# Collect all YARA rule files
file(GLOB_RECURSE YARA_RULE_FILES 
    ${YARA_RULES_SOURCE_DIR}/rules/*.yar
    ${YARA_RULES_SOURCE_DIR}/maldocs/*.yar
)

# Copy YARA rules to build directory
foreach(RULE_FILE ${YARA_RULE_FILES})
    # Get relative path from rules source directory
    file(RELATIVE_PATH REL_PATH ${YARA_RULES_SOURCE_DIR} ${RULE_FILE})
    
    # Create the target path in build directory
    set(TARGET_PATH ${YARA_RULES_INSTALL_DIR}/${REL_PATH})
    
    # Get the directory of the target file
    get_filename_component(TARGET_DIR ${TARGET_PATH} DIRECTORY)
    
    # Create target directory if it doesn't exist
    file(MAKE_DIRECTORY ${TARGET_DIR})
    
    # Add custom command to copy rule file
    add_custom_command(
        OUTPUT ${TARGET_PATH}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RULE_FILE} ${TARGET_PATH}
        DEPENDS ${RULE_FILE}
        COMMENT "Copying YARA rule: ${REL_PATH}"
    )
    
    # Add to list of generated files
    list(APPEND YARA_COPIED_FILES ${TARGET_PATH})
endforeach()

# Create custom target for YARA rules
add_custom_target(YaraRules ALL
    DEPENDS ${YARA_COPIED_FILES}
    COMMENT "Processing YARA rules for compilation"
)

# Make Security library depend on YARA rules
add_dependencies(Security YaraRules)

# Generate a C++ header with rule paths
set(YARA_CONFIG_HEADER ${CMAKE_CURRENT_BINARY_DIR}/YaraRulesPaths.h)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/scanning/yara/YaraRulesPaths.h.in
    ${YARA_CONFIG_HEADER}
    @ONLY
)

# Add the generated header to include directories
target_include_directories(Security PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
