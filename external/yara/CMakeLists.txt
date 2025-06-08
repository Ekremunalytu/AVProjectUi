cmake_minimum_required(VERSION 3.16)
project(libyara)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)
include_directories(.)

# Define preprocessor macros for Windows
add_definitions(-DYARA_STATIC)
add_definitions(-D_WIN32_WINNT=0x0600)
add_definitions(-DWIN32_LEAN_AND_MEAN)

# Define TLSH configuration macros
add_definitions(-DBUCKETS_256)
add_definitions(-DCHECKSUM_3B)

# Enable crypto support for Windows
add_definitions(-DHAVE_WINCRYPT_H)

# Enable Windows process support
if(WIN32)
    add_definitions(-DUSE_WINDOWS_PROC)
endif()

# Collect all C source files
file(GLOB YARA_SOURCES
    "*.c"
    "tlshc/*.c"
)

# Add module source files explicitly
file(GLOB MODULE_SOURCES
    "modules/tests/*.c"
    "modules/time/*.c"
    "modules/string/*.c"
    "modules/math/*.c"
    "modules/pe/*.c"
    # PE authenticode parser disabled due to OpenSSL dependency
    # "modules/pe/authenticode-parser/*.c"
    "modules/elf/*.c"
    "modules/console/*.c"
    "modules/hash/*.c"
    "modules/macho/*.c"
    "modules/dotnet/*.c"
    "modules/dex/*.c"
    "modules/demo/*.c"
    # Cuckoo module disabled due to jansson dependency
    # "modules/cuckoo/*.c"
    # Magic module disabled due to libmagic dependency
    # "modules/magic/*.c"
)

# Add process-related source files
if(WIN32)
    file(GLOB PROC_SOURCES "proc/windows.c")
else()
    file(GLOB PROC_SOURCES "proc/none.c")
endif()

# Combine all sources
list(APPEND YARA_SOURCES ${MODULE_SOURCES} ${PROC_SOURCES})

# Exclude specific files that might cause issues
list(REMOVE_ITEM YARA_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/main.c"
)

# Create static library
add_library(yara STATIC ${YARA_SOURCES})

# Link with Windows libraries
if(WIN32)
    target_link_libraries(yara
        advapi32
        crypt32
        ws2_32
        user32
        kernel32
    )
endif()

# Set public headers
target_include_directories(yara PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Installation rules
install(TARGETS yara
    EXPORT yaraTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/yara
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

install(FILES include/yara.h
    DESTINATION include
)
