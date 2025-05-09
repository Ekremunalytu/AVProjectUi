cmake_minimum_required(VERSION 3.19)
project(AvProjectUi LANGUAGES CXX)

set (CMAKE_PREFIX_PATH "/Volumes/Crucial/QT/6.10.0/macos")

find_package(Qt6 6.10.0 REQUIRED COMPONENTS Core Widgets Sql )
set (CMAKE_AUTOUIC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/UI)

# Resource dosyasını doğru şekilde ekleyin
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
)

# Tüm proje dizinlerinin include yollarını ekleyin
target_include_directories(AvProjectUi PRIVATE
    ${CMAKE_SOURCE_DIR}            # Tüm projeyi include edebilmek için
    ${CMAKE_SOURCE_DIR}/UI/Mainwindow
    ${CMAKE_SOURCE_DIR}/Core         # Core dizini (varsa)
    ${CMAKE_BINARY_DIR}              # Build dizini (ui_*.h dosyaları için)
)

target_link_libraries(AvProjectUi
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
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