/**
 * @file mainwindow.cpp
 * @brief Implementation of the MainWindow class for the application
 * @author Ekrem Ünal
 * @date 9.05.2025
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../Widgets/Dashboard/DashboardWidget.h"
#include "../Widgets/History/HistoryWidget.h" // Added include for HistoryWidget
#include "../Widgets/ServiceStatus/ServiceStatusWidget.h" // Added include for ServiceStatusWidget
#include "Database/DatabaseService/DatabaseService.h" // Added include for DatabaseService

/**
 * @brief Constructs and initializes the main window.
 * 
 * This constructor sets up the UI, configures the window to be fullscreen,
 * initializes the dashboard and other content pages, and connects navigation
 * buttons to their respective actions.
 * 
 * @param parent Parent widget pointer
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // Set up the UI from the .ui file
    ui->setupUi(this);
    
    // Configure window to be fullscreen
    showFullScreen();
    
    // Create the dashboard widget and add it to the dashboard page
    DashboardWidget *dashboardWidget = new DashboardWidget(this);
    QVBoxLayout *dashboardLayout = new QVBoxLayout(ui->dashboardPage);
    dashboardLayout->addWidget(dashboardWidget);
    ui->dashboardPage->setLayout(dashboardLayout);

    // Create the history widget and add it to the history page
    auto& dbService = DatabaseService::getInstance();
    HistoryWidget *historyWidget = new HistoryWidget(dbService.getDbManager(), this);
    QVBoxLayout *historyLayout = new QVBoxLayout(ui->historyPage);
    historyLayout->addWidget(historyWidget);
    ui->historyPage->setLayout(historyLayout);

    // Create the service status widget and add it to the service status page
    ServiceStatusWidget *serviceStatusWidget = new ServiceStatusWidget(this); // Renamed variable
    QWidget *serviceStatusContainerPage = ui->contentStackedWidget->widget(1); // Target index 1 for Service Status

    if (serviceStatusContainerPage) {
        QVBoxLayout *serviceStatusLayout = new QVBoxLayout(serviceStatusContainerPage);
        serviceStatusLayout->addWidget(serviceStatusWidget);
        serviceStatusContainerPage->setLayout(serviceStatusLayout);
    } else {
        // If the page at index 1 (for Service Status) doesn't exist in the stacked widget, log a warning.
        qWarning("Service Status page (at index 1) not found in contentStackedWidget. ServiceStatusWidget will not be added.");
    }
    
    // Set up navigation button connections
    connect(ui->navDashbardButton, &QPushButton::clicked, this, [this]() {
        ui->contentStackedWidget->setCurrentIndex(0); // Dashboard page
    });
    
    connect(ui->navHistoryButton, &QPushButton::clicked, this, [this]() {
        ui->contentStackedWidget->setCurrentIndex(3); // History page
    });
    
    connect(ui->navServiceStatusButton, &QPushButton::clicked, this, [this]() {
        ui->contentStackedWidget->setCurrentIndex(1); // Service status page
    });
    
    connect(ui->navSettingsButton, &QPushButton::clicked, this, [this]() {
        ui->contentStackedWidget->setCurrentIndex(2); // Settings page
    });
    
    // Show the dashboard page by default
    ui->contentStackedWidget->setCurrentIndex(0);
}

/**
 * @brief Destroys the MainWindow object and cleans up resources.
 * 
 * This destructor ensures that UI resources are properly released.
 */
MainWindow::~MainWindow()
{
    delete ui;
}
