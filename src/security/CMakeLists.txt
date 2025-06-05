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

target_link_libraries(Security
    PUBLIC 
        Core
        Storage
        Qt::Core
)
