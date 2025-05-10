cmake_minimum_required(VERSION 3.19)
project(AvProjectUi LANGUAGES CXX)

# C++ standard settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Disable internationalization, forcing English - remove tr() function effects
add_definitions(-DQT_NO_TRANSLATION)

# Enable CTest support
include(CTest)
enable_testing()

# Qt path configuration - adjust this according to your Qt installation
set(CMAKE_PREFIX_PATH "/Volumes/Crucial/QT/6.10.0/macos")

# Find required Qt packages with proper version
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Network Test)

# Enable automatic handling of Qt-specific files
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/UI)

# Global include directories
include_directories(
    ${CMAKE_SOURCE_DIR}          # Root directory for absolute includes
    ${CMAKE_BINARY_DIR}          # Generated files directory (ui_*.h, etc.)
)

# Add resource file properly
set(PROJECT_RESOURCES ${CMAKE_SOURCE_DIR}/resources.qrc)

qt_standard_project_setup()

qt_add_executable(AvProjectUi
    WIN32 MACOSX_BUNDLE
        main.cpp

        UI/Mainwindow/mainwindow.cpp
        UI/Mainwindow/mainwindow.h
        UI/Mainwindow/mainwindow.ui

        UI/Widgets/Dashboard/DashboardWidget.cpp
        UI/Widgets/Dashboard/DashboardWidget.h
        UI/Widgets/Dashboard/DashboardWidget.ui

        ${PROJECT_RESOURCES}

        Database/DbManager/DbManager.cpp
        Database/DbManager/DbManager.h
        Database/DatabaseService/DatabaseService.cpp
        Database/DatabaseService/DatabaseService.h
        
        Scanner/Dashboard/BasicScanner/BasicScanner.cpp
        Scanner/Dashboard/BasicScanner/BasicScanner.h
        
        Core/AppConfig.h
        Network/VirusTotal/VirusTotalManager.cpp
        Network/VirusTotal/VirusTotalManager.h
)

# Include directories with proper Qt paths
target_include_directories(AvProjectUi PRIVATE
    ${CMAKE_SOURCE_DIR}                # Project root for absolute includes
    ${CMAKE_BINARY_DIR}                # Generated files (ui_*.h, etc.)
    ${Qt6Core_INCLUDE_DIRS}           # Qt Core includes
    ${Qt6Widgets_INCLUDE_DIRS}        # Qt Widgets includes
    ${Qt6Sql_INCLUDE_DIRS}            # Qt SQL includes
)

# Add compile definitions to ensure proper Qt handling
target_compile_definitions(AvProjectUi PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
)

target_link_libraries(AvProjectUi
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
        Qt::Network
)

include(GNUInstallDirs)

install(TARGETS AvProjectUi
    BUNDLE  DESTINATION AvProjectUi
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET AvProjectUi
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

# Add tests directory
add_subdirectory(tests)

# Add custom target: build_and_test
add_custom_target(build_and_test
    COMMENT "Build all targets and run tests"
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)