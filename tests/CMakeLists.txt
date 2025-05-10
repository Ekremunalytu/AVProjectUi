# Tests CMakeLists.txt
cmake_minimum_required(VERSION 3.19)

# C++ standard settings (same as main project)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Disable internationalization, forcing English - remove tr() function effects
add_definitions(-DQT_NO_TRANSLATION)

# CTest desteğini etkinleştir
include(CTest)
enable_testing()

# Qt Test paketini bulma
find_package(Qt6 REQUIRED COMPONENTS Test Sql Core Widgets)

# Test kaynağı dizinleri
set(TEST_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Ortak test kaynakları
set(COMMON_TEST_HEADERS
    ${TEST_SOURCES_DIR}/Database/MockDbManager.h
    ${TEST_SOURCES_DIR}/TestDataFactory.h
)

# Qt MOC, UIC, ve RCC için otomatik işleme
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Dashboard widget test projesi
set(DASHBOARD_TEST
    ${TEST_SOURCES_DIR}/UI/DashboardTest.cpp
    ${TEST_SOURCES_DIR}/UI/DashboardTest.h
    ${CMAKE_SOURCE_DIR}/UI/Widgets/Dashboard/DashboardWidget.cpp
    ${CMAKE_SOURCE_DIR}/UI/Widgets/Dashboard/DashboardWidget.h
    ${COMMON_TEST_HEADERS}
)

# Dashboard widget test executable
add_executable(DashboardTest ${DASHBOARD_TEST})
target_include_directories(DashboardTest PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${TEST_SOURCES_DIR}
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
    ${Qt6Test_INCLUDE_DIRS}
    ${Qt6Sql_INCLUDE_DIRS}
)
target_link_libraries(DashboardTest PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Test
    Qt::Sql
)

# MainWindow test projesi
set(MAINWINDOW_TEST
    ${TEST_SOURCES_DIR}/UI/MainWindowTest.cpp
    ${TEST_SOURCES_DIR}/UI/MainWindowTest.h
    ${CMAKE_SOURCE_DIR}/UI/Mainwindow/mainwindow.cpp
    ${CMAKE_SOURCE_DIR}/UI/Mainwindow/mainwindow.h
    ${CMAKE_SOURCE_DIR}/UI/Widgets/Dashboard/DashboardWidget.cpp
    ${CMAKE_SOURCE_DIR}/UI/Widgets/Dashboard/DashboardWidget.h
    ${COMMON_TEST_HEADERS}
)

# MainWindow test executable
add_executable(MainWindowTest ${MAINWINDOW_TEST})
target_include_directories(MainWindowTest PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${TEST_SOURCES_DIR}
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
    ${Qt6Test_INCLUDE_DIRS}
    ${Qt6Sql_INCLUDE_DIRS}
)
target_link_libraries(MainWindowTest PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Test
    Qt::Sql
)

# Database test projesi
set(DATABASE_TEST
    ${TEST_SOURCES_DIR}/Database/DbManagerTest.cpp
    ${TEST_SOURCES_DIR}/Database/DbManagerTest.h
    ${CMAKE_SOURCE_DIR}/Database/DbManager/DbManager.cpp
    ${CMAKE_SOURCE_DIR}/Database/DbManager/DbManager.h
    ${COMMON_TEST_HEADERS}
)

# Database test executable
add_executable(DbManagerTest ${DATABASE_TEST})
target_include_directories(DbManagerTest PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${TEST_SOURCES_DIR}
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
    ${Qt6Test_INCLUDE_DIRS}
    ${Qt6Sql_INCLUDE_DIRS}
)
target_link_libraries(DbManagerTest PRIVATE
    Qt::Core
    Qt::Sql
    Qt::Test
)

# Tüm testlerin listesi
set(TEST_EXECUTABLES
    DashboardTest
    DbManagerTest
    MainWindowTest
)

# Her test için add_test ekle
foreach(test_executable ${TEST_EXECUTABLES})
    add_test(
        NAME ${test_executable}
        COMMAND ${test_executable} -silent
    )
endforeach()

# Test coverage hedefi (opsiyonel)
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # Kapsam raporlaması için derleyici bayrakları
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage")
        
        # Kapsam raporu oluşturan özel hedef
        add_custom_target(coverage
            COMMAND lcov --capture --directory ${CMAKE_BINARY_DIR} --output-file ${CMAKE_BINARY_DIR}/coverage.info
            COMMAND genhtml ${CMAKE_BINARY_DIR}/coverage.info --output-directory ${CMAKE_BINARY_DIR}/coverage-report
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating coverage report"
        )
    endif()
endif()

# Testleri build sonrası otomatik çalıştırmak için özel hedef oluştur
add_custom_target(run_tests ALL
    DEPENDS ${TEST_EXECUTABLES}
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    COMMENT "Running all tests after build..."
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Ana hedef oluşturulduktan sonra testleri çalıştır
add_dependencies(run_tests ${TEST_EXECUTABLES}) 