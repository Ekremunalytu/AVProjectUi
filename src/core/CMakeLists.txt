# Core Layer - Foundation components
add_library(Core STATIC
    config/AppConfig.h
    config/config.ini
    config/SettingsManager.h
    config/SettingsManager.cpp
    interfaces/IDbManager.h
    interfaces/IDockerScanner.h
    interfaces/IScanner.h
    interfaces/IVirusTotalScanner.h
    interfaces/ScannerTypes.h
)

# Set as header-only library
set_target_properties(Core PROPERTIES 
    LINKER_LANGUAGE CXX
    INTERFACE_INCLUDE_DIRECTORIES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_include_directories(Core
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(Core
    PUBLIC Qt::Core
)
