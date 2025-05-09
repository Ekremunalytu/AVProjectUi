#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../Widgets/Dashboard/DashboardWidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // Pencereyi tam ekran olarak ayarla
    showFullScreen();
    
    // Dashboard widget'ını oluştur ve dashboardPage'e ekle
    DashboardWidget *dashboardWidget = new DashboardWidget(this);
    QVBoxLayout *dashboardLayout = new QVBoxLayout(ui->dashboardPage);
    dashboardLayout->addWidget(dashboardWidget);
    ui->dashboardPage->setLayout(dashboardLayout);
    
    // Navigation butonları için bağlantıları kur
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
    
    // Başlangıçta dashboard sayfasını göster
    ui->contentStackedWidget->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}
