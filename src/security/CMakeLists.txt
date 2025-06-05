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
