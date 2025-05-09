cmake_minimum_required(VERSION 3.19)
project(AvProjectUi LANGUAGES CXX)

set (CMAKE_PREFIX_PATH "/Volumes/Crucial/QT/6.10.0/macos")

find_package(Qt6 6.10.0 REQUIRED COMPONENTS Core Widgets )
set (CMAKE_AUTOUIC ON)
set (CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/UI/Forms)
qt_standard_project_setup()

qt_add_executable(AvProjectUi
    WIN32 MACOSX_BUNDLE
        main.cpp
        UI/Src/mainwindow.cpp
        UI/Headers/mainwindow.h
        UI/Forms/mainwindow.ui
)

# Tüm proje dizinlerinin include yollarını ekleyin
target_include_directories(AvProjectUi PRIVATE
    ${CMAKE_SOURCE_DIR}              # Ana proje dizini
    ${CMAKE_SOURCE_DIR}/UI/Headers           # UI ana dizini
    ${CMAKE_SOURCE_DIR}/UI/Forms   # Headers dizini
    ${CMAKE_SOURCE_DIR}/UI/Src       # Src dizini
    ${CMAKE_SOURCE_DIR}/Core         # Core dizini (varsa)
    ${CMAKE_BINARY_DIR}              # Build dizini (ui_*.h dosyaları için)
)


target_link_libraries(AvProjectUi
    PRIVATE
        Qt::Core
        Qt::Widgets
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