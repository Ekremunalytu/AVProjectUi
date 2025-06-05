# Infrastructure Layer - External services and infrastructure
add_library(Infrastructure STATIC
    # Docker operations
    docker/BaseManager.cpp
    docker/DockerManager.cpp
    docker/DockerManager.h
    docker/BaseManager.h
    docker/DockerExceptions.h
    docker/DockerTypes.h
    
    # Network services
    network/VirusTotal/VirusTotalManager.cpp
    network/VirusTotal/VirusTotalManager.h
    network/Monitor/NetworkMonitor.cpp
    network/Monitor/NetworkMonitor.h
)

target_include_directories(Infrastructure
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(Infrastructure
    PUBLIC 
        Core
        Qt::Core
        Qt::Network
        Qt::Widgets
)
