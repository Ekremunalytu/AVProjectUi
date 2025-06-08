# Security Layer - All security operations (CDR, Scanning, Sandbox)
qt_add_library(Security STATIC
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
      # YARA Integration - use built-in YARA library
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

# Use built-in YARA library
set(ENABLE_YARA_SUPPORT ON)
message(STATUS "Using built-in YARA library from external/yara")

target_include_directories(Security
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# Include built-in YARA headers
if(ENABLE_YARA_SUPPORT)
    target_include_directories(Security PRIVATE ${CMAKE_SOURCE_DIR}/external/yara/include)
    target_compile_definitions(Security PRIVATE ENABLE_YARA_SUPPORT=1)
else()
    target_compile_definitions(Security PRIVATE ENABLE_YARA_SUPPORT=0)
endif()

target_link_libraries(Security
    PUBLIC 
        Core
        Storage
        Qt::Core
)

# Link built-in YARA library
if(ENABLE_YARA_SUPPORT)
    target_link_libraries(Security PRIVATE yara)
    
    # On Windows, we need additional libraries for YARA
    if(WIN32)# Find OpenSSL - Windows specific paths for MSYS2/MinGW
        find_package(OpenSSL QUIET)
        if(OpenSSL_FOUND)
            target_link_libraries(Security PRIVATE OpenSSL::SSL OpenSSL::Crypto)
            message(STATUS "Found OpenSSL package: ${OPENSSL_VERSION}")
        else()
            # Try to find OpenSSL manually for MinGW/MSYS2
            find_library(SSL_LIBRARY 
                NAMES ssl libssl ssleay32 ssl-3 libssl-3
                PATHS 
                    C:/msys64/ucrt64/lib
                    C:/msys64/mingw64/lib
                    C:/Qt/Tools/mingw*/lib
                NO_DEFAULT_PATH
            )
            find_library(CRYPTO_LIBRARY 
                NAMES crypto libcrypto libeay32 crypto-3 libcrypto-3
                PATHS 
                    C:/msys64/ucrt64/lib
                    C:/msys64/mingw64/lib
                    C:/Qt/Tools/mingw*/lib
                NO_DEFAULT_PATH
            )
            
            # Also try to find OpenSSL headers
            find_path(OPENSSL_INCLUDE_DIR 
                NAMES openssl/ssl.h
                PATHS 
                    C:/msys64/ucrt64/include
                    C:/msys64/mingw64/include
                    C:/Qt/Tools/mingw*/include
                NO_DEFAULT_PATH
            )
            
            if(SSL_LIBRARY AND CRYPTO_LIBRARY)
                target_link_libraries(Security PRIVATE ${SSL_LIBRARY} ${CRYPTO_LIBRARY})
                message(STATUS "Found OpenSSL libraries: ${SSL_LIBRARY}, ${CRYPTO_LIBRARY}")
                if(OPENSSL_INCLUDE_DIR)
                    target_include_directories(Security PRIVATE ${OPENSSL_INCLUDE_DIR})
                    message(STATUS "Found OpenSSL headers: ${OPENSSL_INCLUDE_DIR}")
                endif()
            else()
                message(WARNING "OpenSSL libraries not found. YARA may not link properly.")
                message(STATUS "Searched paths: C:/msys64/ucrt64/lib, C:/msys64/mingw64/lib")
            endif()
        endif()
          # Find and link TLSH library if available
        find_library(TLSH_LIBRARY 
            NAMES tlsh libtlsh tlsh-static libtlsh-static
            PATHS 
                C:/msys64/ucrt64/lib
                C:/msys64/mingw64/lib
                ${CMAKE_BINARY_DIR}/lib
                ${CMAKE_SOURCE_DIR}/lib
            NO_DEFAULT_PATH
        )
        
        # Also look for TLSH headers
        find_path(TLSH_INCLUDE_DIR
            NAMES tlsh.h
            PATHS
                C:/msys64/ucrt64/include
                C:/msys64/mingw64/include
                ${CMAKE_SOURCE_DIR}/include
            NO_DEFAULT_PATH
        )
        
        if(TLSH_LIBRARY)
            target_link_libraries(Security PRIVATE ${TLSH_LIBRARY})
            message(STATUS "Found TLSH library: ${TLSH_LIBRARY}")
            if(TLSH_INCLUDE_DIR)
                target_include_directories(Security PRIVATE ${TLSH_INCLUDE_DIR})
                message(STATUS "Found TLSH headers: ${TLSH_INCLUDE_DIR}")
            endif()
        else()
            message(WARNING "TLSH library not found. Some YARA features may not work.")
            message(STATUS "Searched paths: C:/msys64/ucrt64/lib, C:/msys64/mingw64/lib")
        endif()
          # Windows system libraries required by YARA (basic set)
        target_link_libraries(Security PRIVATE 
            ws2_32 
            crypt32 
        )
    endif()
endif()

# YARA Rules Integration - Only if YARA is available
if(ENABLE_YARA_SUPPORT)
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
else()
    message(STATUS "YARA support disabled - skipping YARA rules processing")
endif()
