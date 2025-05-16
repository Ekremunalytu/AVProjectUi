#include "NetworkMonitorWidget.h"
#include "ui_NetworkMonitorWidget.h"
#include <QDateTime>
#include <QScrollBar>
#include <QStringLiteral>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

using namespace Qt::StringLiterals;

NetworkMonitorWidget::NetworkMonitorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NetworkMonitorWidget),
    m_logCount(0),
    m_isActive(false)
{
    ui->setupUi(this);
    
    // Initialize the network monitor
    m_networkMonitor = new NetworkMonitor(this);
    
    // Initialize the log model
    m_logModel = new NetworkLogModel(this);
    
    // Setup table view
    setupTableView();
    
    // Initialize status update timer - daha sık güncelleme için 500 ms'ye düşürdük
    m_statusUpdateTimer = new QTimer(this);
    m_statusUpdateTimer->setInterval(500); // 1000'den 500'e düşürüldü
    connect(m_statusUpdateTimer, &QTimer::timeout, this, &NetworkMonitorWidget::updateStatusIndicator);
    m_statusUpdateTimer->start();
    
    // Initialize connection update timer
    m_connectionUpdateTimer = new QTimer(this);
    m_connectionUpdateTimer->setInterval(2000);
    connect(m_connectionUpdateTimer, &QTimer::timeout, this, &NetworkMonitorWidget::updateActiveConnections);
    m_connectionUpdateTimer->start();
    
    setupConnections();
    
    // Initial UI state
    ui->statusValueLabel->setText(u"Pasif"_s);
    ui->statusIndicator->setStyleSheet(u"background-color: rgba(231, 76, 60, 0.9); border-radius: 8px; border: 1px solid rgba(231, 76, 60, 0.7);"_s); // Red for inactive
    ui->logCountLabel->setText(u"0"_s);
    ui->activeConnectionsCountLabel->setText(u"0"_s);
}

NetworkMonitorWidget::~NetworkMonitorWidget()
{
    if (m_networkMonitor->isMonitoring()) {
        m_networkMonitor->stopMonitoring();
    }
    delete ui;
}

void NetworkMonitorWidget::setupConnections()
{
    // Connect buttons
    connect(ui->startButton, &QPushButton::clicked, this, &NetworkMonitorWidget::onStartMonitoringClicked);
    connect(ui->stopButton, &QPushButton::clicked, this, &NetworkMonitorWidget::onStopMonitoringClicked);
    connect(ui->clearButton, &QPushButton::clicked, this, &NetworkMonitorWidget::onClearLogsClicked);
    connect(ui->applyFilterButton, &QPushButton::clicked, this, &NetworkMonitorWidget::onApplyFilterClicked);
    connect(ui->resetFilterButton, &QPushButton::clicked, this, &NetworkMonitorWidget::onResetFilterClicked);
    connect(ui->exportLogsButton, &QPushButton::clicked, this, &NetworkMonitorWidget::onExportLogsClicked);
    
    // Connect network monitor signals
    connect(m_networkMonitor, &NetworkMonitor::newLogMessage, this, &NetworkMonitorWidget::appendNetworkLog);
    connect(m_networkMonitor, &NetworkMonitor::monitoringStateChanged, this, &NetworkMonitorWidget::handleMonitoringStateChanged); // Yeni bağlantı
    
    // Add initial message to the log
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timestamp = currentTime.toString(u"yyyy-MM-dd hh:mm:ss"_s);
    appendNetworkLog(u"==== Ağ İzleme Ekranı Hazır: "_s + timestamp + u" ===="_s);
    appendNetworkLog(u"İzlemeyi başlatmak için 'İzlemeyi Başlat' düğmesine tıklayın."_s);
}

void NetworkMonitorWidget::setupTableView()
{
    // Set the model for the table view
    ui->networkLogsTableView->setModel(m_logModel);
    
    // Configure the column widths
    ui->networkLogsTableView->setColumnWidth(0, 100);  // Time
    ui->networkLogsTableView->setColumnWidth(1, 80);   // Protocol
    ui->networkLogsTableView->setColumnWidth(2, 120);  // Source IP
    ui->networkLogsTableView->setColumnWidth(3, 60);   // Source Port
    ui->networkLogsTableView->setColumnWidth(4, 120);  // Dest IP
    ui->networkLogsTableView->setColumnWidth(5, 60);   // Dest Port
    ui->networkLogsTableView->setColumnWidth(6, 100);  // Status
    
    // Enable sorting
    ui->networkLogsTableView->setSortingEnabled(true);
    
    // Configure selection behavior
    ui->networkLogsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->networkLogsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Enable alternating row colors for better readability
    ui->networkLogsTableView->setAlternatingRowColors(true);
}

void NetworkMonitorWidget::appendNetworkLog(const QString& logMessage)
{
    // Add to the log model
    m_logModel->addLogEntry(logMessage);
    
    // Update log count
    m_logCount = m_logModel->logCount();
    ui->logCountLabel->setText(QString::number(m_logCount));
    
    // Scroll to the bottom of the table view
    ui->networkLogsTableView->scrollToBottom();
}

void NetworkMonitorWidget::clearLogs()
{
    m_logModel->clearLogs();
    m_logCount = 0;
    ui->logCountLabel->setText(u"0"_s);
    ui->activeConnectionsCountLabel->setText(u"0"_s);
}

void NetworkMonitorWidget::onStartMonitoringClicked()
{
    // İzleme durumunu doğrudan NetworkMonitor'dan kontrol et
    if (!m_networkMonitor->isMonitoring()) {
        // İzlemeyi başlat
        m_networkMonitor->startMonitoring();
        // UI güncellemeleri artık handleMonitoringStateChanged üzerinden yapılacak
        // m_isActive = true;
        // ui->statusValueLabel->setText(u"Aktif"_s);
        // ui->statusIndicator->setStyleSheet(u"background-color: rgba(46, 204, 113, 0.9); border-radius: 8px; border: 1px solid rgba(46, 204, 113, 0.7);"_s); // Green for active
        
        // Add start message with timestamp
        // QDateTime currentTime = QDateTime::currentDateTime();
        // QString timestamp = currentTime.toString(u"yyyy-MM-dd hh:mm:ss"_s);
        // appendNetworkLog(u"==== Ağ İzleme Başlatıldı: "_s + timestamp + u" ===="_s); // Bu mesaj NetworkMonitor'dan gelecek
    } else {
        // Zaten aktif olduğuna dair kullanıcıya bilgi ver
        appendNetworkLog(u"Ağ izleme zaten aktif durumda."_s);
    }
}

void NetworkMonitorWidget::onStopMonitoringClicked()
{
    appendNetworkLog(u"İzleme durdurma işlemi başlatılıyor..."_s);
    
    // İzleme durumunu doğrudan NetworkMonitor'dan kontrol et
    if (m_networkMonitor->isMonitoring()) {
        // İzlemeyi durdur
        m_networkMonitor->stopMonitoring();
        // UI güncellemeleri ve log mesajları artık handleMonitoringStateChanged ve NetworkMonitor'dan gelecek
        // m_isActive = false;
        // ui->statusValueLabel->setText(u"Pasif"_s);
        // ui->statusIndicator->setStyleSheet(u"background-color: rgba(231, 76, 60, 0.9); border-radius: 8px; border: 1px solid rgba(231, 76, 60, 0.7);"_s); // Red for inactive
        
        // QTimer::singleShot kısmı NetworkMonitor içinde daha iyi yönetiliyor.
        // Buradaki gecikmeli kontroller kaldırıldı.
        
        // Add stop message with timestamp
        // QDateTime currentTime = QDateTime::currentDateTime();
        // QString timestamp = currentTime.toString(u"yyyy-MM-dd hh:mm:ss"_s);
        // appendNetworkLog(u"==== Ağ İzleme Durduruldu: "_s + timestamp + u" ===="_s); // Bu mesaj NetworkMonitor'dan gelecek
    } else {
        // Zaten pasif olduğuna dair kullanıcıya bilgi ver
        appendNetworkLog(u"Ağ izleme zaten pasif durumda."_s);
    }
}

void NetworkMonitorWidget::onClearLogsClicked()
{
    clearLogs();
}

void NetworkMonitorWidget::updateStatusIndicator()
{
    // Bu fonksiyon artık doğrudan NetworkMonitor::isMonitoring() sonucuna göre UI güncelliyor.
    // m_isActive değişkeni ile senkronizasyon sorunlarını önlemek için doğrudan NetworkMonitor durumu kullanılır.
    bool currentStatus = m_networkMonitor->isMonitoring();
    
    if (currentStatus) {
        if (ui->statusValueLabel->text() != u"Aktif"_s) { // Sadece değişiklik varsa güncelle
            ui->statusValueLabel->setText(u"Aktif"_s);
            ui->statusIndicator->setStyleSheet(u"background-color: rgba(46, 204, 113, 0.9); border-radius: 8px; border: 1px solid rgba(46, 204, 113, 0.7);"_s); // Green for active
            // appendNetworkLog(u"Ağ izleme durumu (timer ile kontrol): Aktif"_s); // Bu log çok sık olabilir, kaldırıldı.
        }
    } else {
        if (ui->statusValueLabel->text() != u"Pasif"_s) { // Sadece değişiklik varsa güncelle
            ui->statusValueLabel->setText(u"Pasif"_s);
            ui->statusIndicator->setStyleSheet(u"background-color: rgba(231, 76, 60, 0.9); border-radius: 8px; border: 1px solid rgba(231, 76, 60, 0.7);"_s); // Red for inactive
            // appendNetworkLog(u"Ağ izleme durumu (timer ile kontrol): Pasif"_s); // Bu log çok sık olabilir, kaldırıldı.
        }
    }
    // m_isActive değişkeni artık burada güncellenmiyor, handleMonitoringStateChanged sorumluluğunda.
}

void NetworkMonitorWidget::updateActiveConnections()
{
    // Update active connections counter
    // m_isActive yerine doğrudan m_networkMonitor->isMonitoring() kullanılmalı
    if (m_networkMonitor->isMonitoring()) { 
        int activeCount = m_logModel->activeConnectionCount();
        ui->activeConnectionsCountLabel->setText(QString::number(activeCount));
    }
}

// Yeni slot implementasyonu
void NetworkMonitorWidget::handleMonitoringStateChanged(bool isMonitoring, const QString& statusMessage)
{
    m_isActive = isMonitoring; // m_isActive burada güncelleniyor
    appendNetworkLog(statusMessage); // NetworkMonitor'dan gelen mesajı logla

    if (m_isActive) {
        ui->statusValueLabel->setText(u"Aktif"_s);
        ui->statusIndicator->setStyleSheet(u"background-color: rgba(46, 204, 113, 0.9); border-radius: 8px; border: 1px solid rgba(46, 204, 113, 0.7);"_s); // Green for active
    } else {
        ui->statusValueLabel->setText(u"Pasif"_s);
        ui->statusIndicator->setStyleSheet(u"background-color: rgba(231, 76, 60, 0.9); border-radius: 8px; border: 1px solid rgba(231, 76, 60, 0.7);"_s); // Red for inactive
        ui->activeConnectionsCountLabel->setText(u"0"_s); // İzleme durduğunda aktif bağlantı sayısını sıfırla
    }
    
    // Butonların etkinleştirilme/devre dışı bırakılma durumunu da burada yönetebiliriz.
    ui->startButton->setEnabled(!m_isActive);
    ui->stopButton->setEnabled(m_isActive);
}

// Add definitions for the missing slots
void NetworkMonitorWidget::onApplyFilterClicked()
{
    // Placeholder for apply filter logic
    QString ipFilter = ui->ipFilterLineEdit->text(); // Changed from filterLineEdit
    QString protocolFilter = ui->protocolFilterComboBox->currentText(); // Assuming a QComboBox for protocol
    int portFilter = ui->portFilterLineEdit->text().toInt(); // Assuming a QLineEdit for port

    // If protocol is "All", pass an empty string to the model
    if (protocolFilter.compare(tr("All"), Qt::CaseInsensitive) == 0) {
        protocolFilter.clear();
    }

    m_logModel->applyFilter(protocolFilter, ipFilter, portFilter);
    appendNetworkLog(QStringLiteral("Filter applied: Protocol='%1', IP='%2', Port='%3'").arg(protocolFilter.isEmpty() ? QStringLiteral("All") : protocolFilter, ipFilter.isEmpty() ? QStringLiteral("All") : ipFilter, portFilter == 0 ? QStringLiteral("All") : QString::number(portFilter)));
}

void NetworkMonitorWidget::onResetFilterClicked()
{
    // Placeholder for reset filter logic
    ui->ipFilterLineEdit->clear(); // Changed from filterLineEdit
    ui->protocolFilterComboBox->setCurrentIndex(0); // Reset protocol to "All" or default
    ui->portFilterLineEdit->clear();
    m_logModel->resetFilter(); // Changed from clearFilter
    appendNetworkLog(QStringLiteral("Filter reset."));
}

void NetworkMonitorWidget::onExportLogsClicked()
{
    // Placeholder for export logs logic
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Logs"), QString(), tr("Text Files (*.txt);;All Files (*)")); // Added QString() for directory
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Export Error"), tr("Could not open file for writing: %1").arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    for (int row = 0; row < m_logModel->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < m_logModel->columnCount(); ++col) {
            rowData << m_logModel->data(m_logModel->index(row, col)).toString();
        }
        out << rowData.join(QLatin1Char('\t')) << QLatin1Char('\n');
    }
    file.close();
    appendNetworkLog(QStringLiteral("Logs exported to: %1").arg(fileName));
}
