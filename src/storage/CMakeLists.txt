# Storage Layer - Database and filesystem operations
qt_add_library(Storage STATIC
    database/DbManager/DbManager.cpp
    database/DbManager/DbManager.h
    database/DatabaseService/DatabaseService.cpp
    database/DatabaseService/DatabaseService.h
    database/Models/ScanHistoryModel.cpp
    database/Models/ScanHistoryModel.h
)

target_include_directories(Storage
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(Storage
    PUBLIC 
        Core
        Qt::Core 
        Qt::Sql
        Qt::Gui
        Qt::Widgets
)
