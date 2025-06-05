# Presentation Layer - User Interface components
qt_add_library(Presentation STATIC
    # Main application window
    application/mainwindow.cpp
    application/mainwindow.h
    application/mainwindow.ui
    
    # Dashboard widgets
    widgets/Dashboard/DashboardWidget.cpp
    widgets/Dashboard/DashboardWidget.h
    widgets/Dashboard/DashboardWidget.ui
    
    # Settings widgets
    widgets/Settings/SettingsDialog.cpp
    widgets/Settings/SettingsDialog.h
    widgets/Settings/SettingsWidget.cpp
    widgets/Settings/SettingsWidget.h
    widgets/Settings/SettingsWidget.ui
    
    # History widgets
    widgets/History/HistoryWidget.cpp
    widgets/History/HistoryWidget.h
    widgets/History/HistoryWidget.ui
    
    # Service status widgets
    widgets/ServiceStatus/ServiceStatusWidget.cpp
    widgets/ServiceStatus/ServiceStatusWidget.h
    widgets/ServiceStatus/ServiceStatusWidget.ui
    
    # Resources
    resources/resources.qrc
)

target_include_directories(Presentation
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(Presentation
    PUBLIC 
        Core
        Storage
        Security
        Infrastructure
        Application
        Qt::Core
        Qt::Widgets
)
