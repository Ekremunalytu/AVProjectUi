#include "DashboardWidget.h"
#include "ui_dashboardwidget.h"
#include <QDebug>

DashboardWidget::DashboardWidget(QWidget *parent):
    QWidget(parent),
    ui(new Ui::DashboardWidget)
{
    ui->setupUi(this);
    
    // Tüm butonlara tıklama olaylarını bağla
    connect(ui->basicScanButton_dashboard, &QPushButton::clicked, this, &DashboardWidget::onBasicScanClicked);
    connect(ui->advancedScanButton_dashboard, &QPushButton::clicked, this, &DashboardWidget::onAdvancedScanClicked);
    connect(ui->cdrScanButton_dashboard, &QPushButton::clicked, this, &DashboardWidget::onCdrScanClicked);
    connect(ui->sandboxScanButton_dashboard, &QPushButton::clicked, this, &DashboardWidget::onSandboxScanClicked);
}

DashboardWidget::~DashboardWidget() {
    delete ui;
}

void DashboardWidget::onBasicScanClicked() {
    qDebug() << "Basic Scan başlatılıyor...";
    ui->scanResultsTextEdit_dashboard->append("Basic Scan başlatılıyor...");
}

void DashboardWidget::onAdvancedScanClicked() {
    qDebug() << "Advanced Scan başlatılıyor...";
    ui->scanResultsTextEdit_dashboard->append("Advanced Scan başlatılıyor...");
}

void DashboardWidget::onCdrScanClicked() {
    qDebug() << "CDR Scan başlatılıyor...";
    ui->scanResultsTextEdit_dashboard->append("CDR Scan başlatılıyor...");
}

void DashboardWidget::onSandboxScanClicked() {
    qDebug() << "Sandbox Scan başlatılıyor...";
    ui->scanResultsTextEdit_dashboard->append("Sandbox Scan başlatılıyor...");
}
