/**
 * @file mainwindow.cpp
 * @brief Implementation of the MainWindow class for the application
 * @author Ekrem Ünal
 * @date 9.05.2025
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../Widgets/Dashboard/DashboardWidget.h"

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
