# Core Layer - Foundation components
qt_add_library(Core STATIC
    config/AppConfig.h
    config/config.ini
    config/SettingsManager.h
    config/SettingsManager.cpp
    session/SessionManager.h
    session/SessionManager.cpp
    interfaces/IDbManager.h
    interfaces/IDockerScanner.h
    interfaces/IScanner.h
    interfaces/IVirusTotalScanner.h
    interfaces/IYaraRuleManager.h
    interfaces/ScannerTypes.h
)

target_include_directories(Core
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(Core
    PUBLIC Qt::Core
)
