#include "DashboardWidget.h"
#include "ui_dashboardwidget.h"
#include "storage/database/DatabaseService/DatabaseService.h"
#include "../../../core/config/SettingsManager.h"
#include <QDebug>
#include <QAction>
#include <QMessageBox>
#include <QString>
#include <QStringView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFont>
#include <QDateTime>
#include <QColor>
#include <QHeaderView>
#include <QFileDialog> // Added for file dialog
#include <QStandardPaths>
#include <QProgressDialog>
#include <QThread>
#include <QTimer>
#include <QCoreApplication>


/**
 * @brief Handles file selection for advanced scan.
 * 
 * Opens a file dialog and updates the selected path label.
 */
void DashboardWidget::onSelectFileClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, 
                                                    tr("Select File for Advanced Scan"), 
                                                    QDir::homePath(), 
                                                    tr("All Files (*.*)"));
    
    if (!filePath.isEmpty()) {
        ui->selectedPathLabel->setText(tr("File: %1").arg(QFileInfo(filePath).fileName()));
        ui->selectedPathLabel->setToolTip(filePath); // Show full path in tooltip
        
        // Store the selected path for later use
        ui->selectedPathLabel->setProperty("selectedPath", filePath);
        ui->selectedPathLabel->setProperty("selectionType", "file");
        
        // Enable advanced scan button if a valid path is selected
        ui->advancedScanButton->setEnabled(true);
    }
}

/**
 * @brief Handles directory selection for advanced scan.
 * 
 * Opens a directory dialog and updates the selected path label.
 */
void DashboardWidget::onSelectDirectoryClicked()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, 
                                                        tr("Select Directory for Advanced Scan"), 
                                                        QDir::homePath(), 
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (!dirPath.isEmpty()) {
        ui->selectedPathLabel->setText(tr("Directory: %1").arg(QFileInfo(dirPath).baseName()));
        ui->selectedPathLabel->setToolTip(dirPath); // Show full path in tooltip
        
        // Store the selected path for later use
        ui->selectedPathLabel->setProperty("selectedPath", dirPath);
        ui->selectedPathLabel->setProperty("selectionType", "directory");
        
        // Enable advanced scan button if a valid path is selected
        ui->advancedScanButton->setEnabled(true);
    }
}

using namespace Qt::StringLiterals;

/**
 * @brief Standard text constants for UI display
 */
namespace DashboardText {
    // Scan types
    constexpr auto BASIC_SCAN = "Basic Scan";
    constexpr auto ADVANCED_SCAN = "Starting Advanced Scan...";
    constexpr auto CDR_SCAN = "Starting CDR Scan...";
    constexpr auto SANDBOX_SCAN = "Starting Sandbox Scan...";
    constexpr auto VT_SCAN = "VirusTotal Analysis";
    
    // Status messages
    constexpr auto SELECTING_FILE = "Selecting file...";
    constexpr auto FILE_SELECTED = "File selected: %1";
    constexpr auto SCANNING_FILE = "Scanning file...";
    constexpr auto SELECTION_CANCELED = "File selection canceled.";
    constexpr auto ERROR_PREFIX = "Error: %1";
    
    // VirusTotal messages
    constexpr auto VT_SELECTING = "Selecting file for VirusTotal analysis...";
    constexpr auto VT_SUBMITTING = "Submitting to VirusTotal...";
    constexpr auto VT_SUBMIT_STATUS = "File submission status: %1";
    constexpr auto VT_SUBMIT_SUCCESS = "File successfully submitted to VirusTotal.";
    constexpr auto VT_RETRIEVING = "Retrieving analysis report...";
    constexpr auto VT_ANALYSIS_RESULTS = "Analysis Results:";
    constexpr auto VT_ANALYSIS_PENDING = "Analysis is pending. Check back later or resubmit.";
    
    // Dialog titles
    constexpr auto DB_ERROR = "Database Error";
    constexpr auto FILE_ERROR = "File Error";
    constexpr auto SCAN_ERROR = "Scanning Error";
    constexpr auto MALICIOUS_DETECTED = "Malicious File Detected";
    constexpr auto GENERIC_ERROR = "Error";
    constexpr auto VT_ERROR = "VirusTotal Error";
}

/**
 * @brief Constructs the DashboardWidget with UI setup and connection initialization.
 * 
 * This constructor initializes the UI, creates the basic scanner with database connectivity,
 * and sets up signal-slot connections for all buttons and scanner events.
 * 
 * @param parent The parent widget.
 */
DashboardWidget::DashboardWidget(QWidget *parent):
    QWidget(parent),
    ui(new Ui::DashboardWidget),
    m_basicScanner(std::make_unique<BasicScanner>(this, DatabaseService::getInstance().getDbManager())),
    m_virusTotalManager(std::make_unique<VirusTotalManager>()),
    m_cdrScanner(std::make_unique<CDRScanner>(this)), // Initialize CDRScanner with parent
    m_networkMonitor(new NetworkMonitor(this)), // Instantiate NetworkMonitor    m_dockerManager(std::make_unique<Docker::DockerManager>()), // Initialize DockerManager
    m_cdrManager(std::make_unique<CDR::CdrManager>()), // Initialize CdrManager
    m_sandboxManager(std::make_unique<Sandbox::SandboxManager>()) // Initialize SandboxManager
{
    ui->setupUi(this);
    
    // Load and apply custom styles
    QFile styleFile(u":/styles/dashboard.css"_s);
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString styleSheet = QString::fromUtf8(styleFile.readAll());
        styleFile.close();
        
        // Apply styles to specific elements
        ui->basicScanResultsFrame->setStyleSheet(styleSheet);
        ui->advancedScanResultsFrame->setStyleSheet(styleSheet);
        ui->cdrResultsFrame->setStyleSheet(styleSheet);
        ui->sandboxResultsFrame->setStyleSheet(styleSheet);
        ui->networkResultsFrame->setStyleSheet(styleSheet);
        
        // Direct styling of text edit widgets for better appearance
        QString textEditStyle = u"QTextEdit {"
                               "border-radius: 6px;"
                               "background-color: rgba(30, 30, 30, 0.8);"
                               "color: #E0E0E0;"
                               "padding: 8px;"
                               "}"_s;
        
        ui->basicScanResultsTextEdit->setStyleSheet(textEditStyle);
        ui->cdrResultsTextEdit->setStyleSheet(textEditStyle);
        ui->sandboxResultsTextEdit->setStyleSheet(textEditStyle);
        ui->networkCommunicationTextEdit->setStyleSheet(textEditStyle);
        
        // Table widget styling
        QString tableStyle = u"QTableWidget {"
                            "border-radius: 6px;"
                            "background-color: rgba(30, 30, 30, 0.8);"
                            "color: #E0E0E0;"
                            "gridline-color: #4A4A4A;"
                            "}"
                            "QTableWidget::item {"
                            "padding: 5px;"
                            "border-radius: 3px;"
                            "}"
                            "QHeaderView::section {"
                            "background-color: #2A2A2A;"
                            "color: #FFFFFF;"
                            "padding: 6px;"
                            "font-weight: bold;"
                            "border: 1px solid #4A4A4A;"
                            "border-radius: 0px;"
                            "}"_s;
        
        ui->advancedScanResultsTableWidget->setStyleSheet(tableStyle);
        
        // Set font for better readability
        QFont monoFont(u"Menlo"_s);
        monoFont.setStyleHint(QFont::Monospace);
        ui->basicScanResultsTextEdit->setFont(monoFont);
        ui->cdrResultsTextEdit->setFont(monoFont);
        ui->sandboxResultsTextEdit->setFont(monoFont);
        ui->networkCommunicationTextEdit->setFont(monoFont);
        ui->advancedScanResultsTableWidget->setFont(monoFont);
    }
    
    // Apply styles to buttons
    QString buttonStyleBase = u"QPushButton {"
                             "background-color: #2A2A2A;"
                             "color: #FFFFFF;"
                             "border: 1px solid #3A3A3A;"
                             "border-radius: 4px;"
                             "padding: 8px 16px;"
                             "font-weight: bold;"
                             "min-height: 36px;"
                             "}"
                             "QPushButton:hover {"
                             "background-color: #3A3A3A;"
                             "border: 1px solid #4A4A4A;"
                             "}"
                             "QPushButton:pressed {"
                             "background-color: #1A1A1A;"
                             "border: 1px solid #2A2A2A;"
                             "}"_s;

    // Action buttons with themed colors
    QString basicScanButtonStyle = buttonStyleBase;
    basicScanButtonStyle.replace(u"#2A2A2A"_s, u"#1976D2"_s); // Blue
    ui->basicScanButton->setStyleSheet(basicScanButtonStyle);
    
    QString advancedScanButtonStyle = buttonStyleBase;
    advancedScanButtonStyle.replace(u"#2A2A2A"_s, u"#00796B"_s); // Teal
    ui->advancedScanButton->setStyleSheet(advancedScanButtonStyle);
    
    QString cdrScanButtonStyle = buttonStyleBase;
    cdrScanButtonStyle.replace(u"#2A2A2A"_s, u"#512DA8"_s); // Deep Purple
    ui->cdrScanButton->setStyleSheet(cdrScanButtonStyle);
    
    QString sandboxScanButtonStyle = buttonStyleBase;
    sandboxScanButtonStyle.replace(u"#2A2A2A"_s, u"#D32F2F"_s); // Red
    ui->sandboxScanButton->setStyleSheet(sandboxScanButtonStyle);
    
    // Network monitor button starts in active mode
    QString networkButtonStyle = buttonStyleBase;
    networkButtonStyle.replace(u"#2A2A2A"_s, u"#F44336"_s); // Red for active state
    ui->networkMonitorButton->setStyleSheet(networkButtonStyle);
    ui->networkMonitorButton->setText(u"Ağ İzlemeyi Durdur"_s);
    
    // Refresh button
    QString refreshButtonStyle = buttonStyleBase;
    refreshButtonStyle.replace(u"#2A2A2A"_s, u"#607D8B"_s); // Blue Grey
    ui->refreshButton->setStyleSheet(refreshButtonStyle);
    ui->refreshButton->setText(u"Yenile"_s); // Turkish localization
    
    // Update button text for Turkish users
    ui->basicScanButton->setText(u"Temel Tarama"_s);
    ui->advancedScanButton->setText(u"Gelişmiş Tarama"_s);
    ui->cdrScanButton->setText(u"CDR Taraması"_s);
    ui->sandboxScanButton->setText(u"Sandbox Taraması"_s);
    
    // Update tab names for consistency
    ui->dashboardTabWidget->setTabText(ui->dashboardTabWidget->indexOf(ui->basicScanTab), u"Temel Tarama"_s);
    ui->dashboardTabWidget->setTabText(ui->dashboardTabWidget->indexOf(ui->advancedScanTab), u"Gelişmiş Tarama"_s);
    ui->dashboardTabWidget->setTabText(ui->dashboardTabWidget->indexOf(ui->cdrTab), u"CDR"_s);
    ui->dashboardTabWidget->setTabText(ui->dashboardTabWidget->indexOf(ui->sandboxTab), u"Sandbox"_s);
    ui->dashboardTabWidget->setTabText(ui->dashboardTabWidget->indexOf(ui->networkTab), u"Ağ İzleme"_s);
    
    // Connect click events for all buttons in the tabbed interface
    connect(ui->basicScanButton, &QPushButton::clicked, this, &DashboardWidget::onBasicScanButtonClicked);
    connect(ui->advancedScanButton, &QPushButton::clicked, this, &DashboardWidget::onAdvancedScanButtonClicked);
    connect(ui->cdrScanButton, &QPushButton::clicked, this, &DashboardWidget::onCdrScanButtonClicked);
    connect(ui->sandboxScanButton, &QPushButton::clicked, this, &DashboardWidget::onSandboxScanButtonClicked);
    connect(ui->networkMonitorButton, &QPushButton::clicked, this, &DashboardWidget::onNetworkMonitorButtonClicked);
    connect(ui->refreshButton, &QPushButton::clicked, this, &DashboardWidget::onRefreshButtonClicked);
    
    // Basic scan connections
    connect(m_basicScanner.get(), &BasicScanner::scanResultsReady, this, &DashboardWidget::onBasicScanResultsReady);
    connect(m_basicScanner.get(), &BasicScanner::scanError, this, &DashboardWidget::onBasicScanError);
    
    // CDR scan connections
    connect(m_cdrScanner.get(), &CDRScanner::scanResultsReady, this, &DashboardWidget::onCdrScanResultsReady);
    connect(m_cdrScanner.get(), &CDRScanner::scanError, this, &DashboardWidget::onCdrScanError);
    
    // VirusTotal connections
    connect(m_virusTotalManager.get(), &VirusTotalManager::analysisResultsReady, this, &DashboardWidget::handleVirusTotalResults);

    // Network Monitor connections
    connect(m_networkMonitor, &NetworkMonitor::newLogMessage, this, &DashboardWidget::appendNetworkLog);
    m_networkMonitor->startMonitoring(); // Start monitoring
    
    // Initialize advanced scan results table
    if (ui->advancedScanResultsTableWidget) {
        ui->advancedScanResultsTableWidget->setColumnCount(3);
        QStringList headers;
        headers << tr("Scan Type") << tr("Status") << tr("Details");
        ui->advancedScanResultsTableWidget->setHorizontalHeaderLabels(headers);
        ui->advancedScanResultsTableWidget->horizontalHeader()->setStretchLastSection(true);
        ui->advancedScanResultsTableWidget->setWordWrap(true);
        ui->advancedScanResultsTableWidget->verticalHeader()->setDefaultSectionSize(60);
    }
    
    // Initialize UI states - Advanced scan button is now always enabled
    ui->selectedPathLabel->setText(tr("Click 'Advanced Scan' to select file or directory"));
}

/**
 * @brief Destroys the DashboardWidget and releases resources.
 */
DashboardWidget::~DashboardWidget() {
    if (m_networkMonitor) {
        m_networkMonitor->stopMonitoring();
        // m_networkMonitor is a child of DashboardWidget, so it will be deleted automatically by Qt's parent-child mechanism.
        // No explicit delete m_networkMonitor; needed if it's set as a child.
    }
    delete ui;
}

/**
 * @brief Handles the Basic Scan button click.
 * 
 * Checks the scan type radio buttons and initiates either file or directory scanning.
 */
void DashboardWidget::onBasicScanButtonClicked() {
    // Check which scan type is selected
    if (ui->fileScanRadio->isChecked()) {
        onBasicScanSelectFile();
    } else if (ui->directoryScanRadio->isChecked()) {
        onBasicScanSelectDirectory();
    } else {
        // Default to file scan if neither is checked
        ui->fileScanRadio->setChecked(true);
        onBasicScanSelectFile();
    }
}

/**
 * @brief Handles the Advanced Scan button click.
 * 
 * Opens a file dialog for advanced scanning (file only, no directory option).
 */
void DashboardWidget::onAdvancedScanButtonClicked() {
    // Only allow file scanning for advanced scan
    QString selectedPath = QFileDialog::getOpenFileName(this,
        tr("Select File for Advanced Scan"),
        QDir::homePath(),
        tr("All Files (*.*)"));
    
    if (!selectedPath.isEmpty()) {
        // Switch to advanced scan tab and clear previous results
        ui->dashboardTabWidget->setCurrentWidget(ui->advancedScanTab);
        ui->advancedScanResultsTableWidget->clearContents();
        ui->advancedScanResultsTableWidget->setRowCount(0);
        
        // Start advanced scan immediately
        startAdvancedScanWithPath(selectedPath, "file");
    }
}

/**
 * @brief Handles the CDR Scan button click.
 * 
 * Currently displays a debug message and updates the UI.
 */
void DashboardWidget::onCdrScanButtonClicked() {
    qDebug() << DashboardText::CDR_SCAN;
    ui->dashboardTabWidget->setCurrentWidget(ui->cdrTab);
    
    // Clear previous content
    ui->cdrResultsTextEdit->clear();
    
    // Add timestamp header
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timestamp = currentTime.toString(u"yyyy-MM-dd hh:mm:ss"_s);
    
    ui->cdrResultsTextEdit->setTextColor(QColor(u"#9E9E9E"_s));
    ui->cdrResultsTextEdit->append(u"=== "_s + timestamp + u" ==="_s);
    ui->cdrResultsTextEdit->append(u""_s);
    
    // Add stylized starting message
    ui->cdrResultsTextEdit->setTextColor(QColor(u"#FFFFFF"_s));
    ui->cdrResultsTextEdit->append(u"🔍 PROCESS:"_s);
    ui->cdrResultsTextEdit->setTextColor(QColor(u"#2196F3"_s)); // Blue for process name
    ui->cdrResultsTextEdit->append(u"  "_s + QString::fromUtf8(DashboardText::CDR_SCAN));
    ui->cdrResultsTextEdit->append(u""_s);

    // Open file dialog to select a file
    QString filePath = QFileDialog::getOpenFileName(this, 
                                                    tr("Select File for CDR Scan"), 
                                                    QDir::homePath(), 
                                                    tr("All Files (*.*)"));

    if (filePath.isEmpty()) {
        ui->cdrResultsTextEdit->setTextColor(QColor(u"#FFA726"_s)); // Orange for warnings/cancellations
        ui->cdrResultsTextEdit->append(tr("File selection canceled."));
        return;
    }

    ui->cdrResultsTextEdit->setTextColor(QColor(u"#E0E0E0"_s)); // Default text color
    ui->cdrResultsTextEdit->append(tr("Selected file: %1").arg(filePath));
    ui->cdrResultsTextEdit->append(tr("Initiating CDR analysis..."));

    // Use the m_cdrScanner instead of m_cdrManager to ensure signals are emitted
    if (m_cdrScanner) {
        ui->cdrResultsTextEdit->append(tr("🔍 Starting CDR scan with scanner..."));
        
        // Start the CDR scan - this will emit signals when completed
        bool scanStarted = m_cdrScanner->scanFile(filePath);
        
        if (!scanStarted) {
            ui->cdrResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
            ui->cdrResultsTextEdit->append(tr("❌ Failed to start CDR scan: %1").arg(m_cdrScanner->getLastError()));
            return;
        }
        
        ui->cdrResultsTextEdit->append(tr("✅ CDR scan initiated successfully"));
        ui->cdrResultsTextEdit->append(tr("📡 Scan results will be displayed when ready..."));
    } else {
        ui->cdrResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
        ui->cdrResultsTextEdit->append(tr("❌ CDR Scanner not available"));
    }
}

/**
 * @brief Handles the Sandbox Scan button click.
 * 
 * Currently displays a debug message and updates the UI.
 */
void DashboardWidget::onSandboxScanButtonClicked() {
    qDebug() << DashboardText::SANDBOX_SCAN;
    ui->dashboardTabWidget->setCurrentWidget(ui->sandboxTab);
    
    // Clear previous content
    ui->sandboxResultsTextEdit->clear();
    
    // Add timestamp header
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timestamp = currentTime.toString(u"yyyy-MM-dd hh:mm:ss"_s);
    
    ui->sandboxResultsTextEdit->setTextColor(QColor(u"#9E9E9E"_s));
    ui->sandboxResultsTextEdit->append(u"=== "_s + timestamp + u" ==="_s);
    ui->sandboxResultsTextEdit->append(u""_s);
    
    // Add stylized starting message
    ui->sandboxResultsTextEdit->setTextColor(QColor(u"#FFFFFF"_s));
    ui->sandboxResultsTextEdit->append(u"🔍 PROCESS:"_s);
    ui->sandboxResultsTextEdit->setTextColor(QColor(u"#2196F3"_s));
    ui->sandboxResultsTextEdit->append(u"  "_s + QString::fromUtf8(DashboardText::SANDBOX_SCAN));
    ui->sandboxResultsTextEdit->append(u""_s);
    
    // Open file dialog to select a file
    QString filePath = QFileDialog::getOpenFileName(this, 
                                                    tr("Select File for Sandbox Analysis"), 
                                                    QDir::homePath(), 
                                                    tr("Executable Files (*.exe *.dll *.bat *.ps1);;All Files (*.*)"));

    if (filePath.isEmpty()) {
        ui->sandboxResultsTextEdit->setTextColor(QColor(u"#FFA726"_s));
        ui->sandboxResultsTextEdit->append(tr("File selection canceled."));
        return;
    }

    ui->sandboxResultsTextEdit->setTextColor(QColor(u"#E0E0E0"_s));
    ui->sandboxResultsTextEdit->append(tr("Selected file: %1").arg(filePath));
    ui->sandboxResultsTextEdit->append(u""_s);
    
    if (!m_sandboxManager) {
        ui->sandboxResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
        ui->sandboxResultsTextEdit->append(tr("❌ Sandbox manager not initialized"));
        return;
    }
    
    // Check if Docker daemon is running
    if (!m_sandboxManager->isDaemonRunning()) {
        ui->sandboxResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
        ui->sandboxResultsTextEdit->append(tr("❌ Docker daemon is not running"));
        ui->sandboxResultsTextEdit->append(tr("Please start Docker Desktop and try again"));
        return;
    }
    
    // Step 1: Analyze file for threats in sandbox
    ui->sandboxResultsTextEdit->setTextColor(QColor(u"#2196F3"_s));
    ui->sandboxResultsTextEdit->append(tr("🏗️  Creating isolated sandbox environment..."));
    ui->sandboxResultsTextEdit->append(tr("🔍 Analyzing file for threats..."));
    
    // Use the comprehensive sandbox analysis
    Sandbox::SandboxAnalysisResult analysisResult = m_sandboxManager->analyzeFileForThreats(
        filePath.toStdString(), 
        Sandbox::MonitoringLevel::STANDARD
    );
    
    // Display analysis results
    ui->sandboxResultsTextEdit->setTextColor(QColor(u"#4CAF50"_s));
    ui->sandboxResultsTextEdit->append(tr("✅ Sandbox analysis completed"));
    ui->sandboxResultsTextEdit->append(u""_s);
    
    // Display threat level
    ui->sandboxResultsTextEdit->setTextColor(QColor(u"#FFFFFF"_s));
    ui->sandboxResultsTextEdit->append(tr("📋 SANDBOX ANALYSIS RESULTS:"));
    ui->sandboxResultsTextEdit->append(u""_s);
    
    // Threat level display
    QString threatLevelStr;
    QColor threatColor;
    switch (analysisResult.overallThreatLevel) {
        case Sandbox::ThreatLevel::NONE:
            threatLevelStr = tr("NONE (Safe)");
            threatColor = QColor(u"#4CAF50"_s);
            break;
        case Sandbox::ThreatLevel::LOW:
            threatLevelStr = tr("LOW");
            threatColor = QColor(u"#FFEB3B"_s);
            break;
        case Sandbox::ThreatLevel::MEDIUM:
            threatLevelStr = tr("MEDIUM");
            threatColor = QColor(u"#FF9800"_s);
            break;
        case Sandbox::ThreatLevel::HIGH:
            threatLevelStr = tr("HIGH");
            threatColor = QColor(u"#FF5722"_s);
            break;
        case Sandbox::ThreatLevel::CRITICAL:
            threatLevelStr = tr("CRITICAL");
            threatColor = QColor(u"#F44336"_s);
            break;
        default:
            threatLevelStr = tr("UNKNOWN");
            threatColor = QColor(u"#9E9E9E"_s);
    }
    
    ui->sandboxResultsTextEdit->setTextColor(threatColor);
    ui->sandboxResultsTextEdit->append(tr("⚠️  THREAT LEVEL: %1").arg(threatLevelStr));
    ui->sandboxResultsTextEdit->append(u""_s);
    
    // Display detected threats
    if (!analysisResult.detectedThreats.empty()) {
        ui->sandboxResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
        ui->sandboxResultsTextEdit->append(tr("🚨 DETECTED THREATS (%1):").arg(analysisResult.detectedThreats.size()));
        ui->sandboxResultsTextEdit->setTextColor(QColor(u"#E0E0E0"_s));
        
        for (const auto& threat : analysisResult.detectedThreats) {
            QString threatTypeStr;
            switch (threat.type) {
                case Sandbox::ThreatType::MALWARE: threatTypeStr = tr("Malware"); break;
                case Sandbox::ThreatType::VIRUS: threatTypeStr = tr("Virus"); break;
                case Sandbox::ThreatType::TROJAN: threatTypeStr = tr("Trojan"); break;
                case Sandbox::ThreatType::RANSOMWARE: threatTypeStr = tr("Ransomware"); break;
                case Sandbox::ThreatType::SPYWARE: threatTypeStr = tr("Spyware"); break;
                case Sandbox::ThreatType::ADWARE: threatTypeStr = tr("Adware"); break;
                case Sandbox::ThreatType::POTENTIALLY_UNWANTED_PROGRAM: threatTypeStr = tr("PUP"); break;
                case Sandbox::ThreatType::SUSPICIOUS_BEHAVIOR: threatTypeStr = tr("Suspicious Behavior"); break;
                default: threatTypeStr = tr("Unknown");
            }
            
            ui->sandboxResultsTextEdit->append(tr("  • %1: %2").arg(threatTypeStr, QString::fromStdString(threat.description)));
            if (!threat.details.empty()) {
                ui->sandboxResultsTextEdit->append(tr("    Details: %1").arg(QString::fromStdString(threat.details)));
            }
        }
        ui->sandboxResultsTextEdit->append(u""_s);
    } else {
        ui->sandboxResultsTextEdit->setTextColor(QColor(u"#4CAF50"_s));
        ui->sandboxResultsTextEdit->append(tr("✅ No threats detected"));
        ui->sandboxResultsTextEdit->append(u""_s);
    }
    
    // Display behavioral analysis
    if (!analysisResult.behaviorAnalysis.networkConnections.empty()) {
        ui->sandboxResultsTextEdit->setTextColor(QColor(u"#2196F3"_s));
        ui->sandboxResultsTextEdit->append(tr("🌐 NETWORK ACTIVITY (%1 connections):").arg(analysisResult.behaviorAnalysis.networkConnections.size()));
        ui->sandboxResultsTextEdit->setTextColor(QColor(u"#E0E0E0"_s));
        
        for (const auto& conn : analysisResult.behaviorAnalysis.networkConnections) {
            ui->sandboxResultsTextEdit->append(tr("  • %1:%2 (%3)").arg(
                QString::fromStdString(conn.destinationHost),
                QString::number(conn.destinationPort),
                QString::fromStdString(conn.protocol)
            ));
        }
        ui->sandboxResultsTextEdit->append(u""_s);
    }
    
    // Display file system activity
    if (!analysisResult.behaviorAnalysis.fileSystemActivity.empty()) {
        ui->sandboxResultsTextEdit->setTextColor(QColor(u"#9C27B0"_s));
        ui->sandboxResultsTextEdit->append(tr("📁 FILE SYSTEM ACTIVITY (%1 operations):").arg(analysisResult.behaviorAnalysis.fileSystemActivity.size()));
        ui->sandboxResultsTextEdit->setTextColor(QColor(u"#E0E0E0"_s));
        
        int displayCount = 0;
        for (const auto& fsActivity : analysisResult.behaviorAnalysis.fileSystemActivity) {
            if (displayCount >= 10) { // Limit display to first 10
                ui->sandboxResultsTextEdit->append(tr("  ... and %1 more operations").arg(analysisResult.behaviorAnalysis.fileSystemActivity.size() - displayCount));
                break;
            }
            
            QString operationStr;
            switch (fsActivity.operation) {
                case Sandbox::FileOperation::CREATE: operationStr = tr("CREATE"); break;
                case Sandbox::FileOperation::MODIFY: operationStr = tr("MODIFY"); break;
                case Sandbox::FileOperation::DELETE: operationStr = tr("DELETE"); break;
                case Sandbox::FileOperation::READ: operationStr = tr("READ"); break;
                case Sandbox::FileOperation::WRITE: operationStr = tr("WRITE"); break;
                default: operationStr = tr("UNKNOWN");
            }
            
            ui->sandboxResultsTextEdit->append(tr("  • %1: %2").arg(operationStr, QString::fromStdString(fsActivity.filePath)));
            displayCount++;
        }
        ui->sandboxResultsTextEdit->append(u""_s);
    }
    
    // Show execution summary
    ui->sandboxResultsTextEdit->setTextColor(QColor(u"#607D8B"_s));
    ui->sandboxResultsTextEdit->append(tr("📊 EXECUTION SUMMARY:"));
    ui->sandboxResultsTextEdit->setTextColor(QColor(u"#E0E0E0"_s));
    ui->sandboxResultsTextEdit->append(tr("  • Analysis Duration: %1 seconds").arg(analysisResult.executionDurationSeconds));
    ui->sandboxResultsTextEdit->append(tr("  • Exit Code: %1").arg(analysisResult.processExitCode));
    ui->sandboxResultsTextEdit->append(tr("  • Success: %1").arg(analysisResult.success ? tr("Yes") : tr("No")));
    
    if (!analysisResult.errorMessage.empty()) {
        ui->sandboxResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
        ui->sandboxResultsTextEdit->append(tr("  • Error: %1").arg(QString::fromStdString(analysisResult.errorMessage)));
    }
    ui->sandboxResultsTextEdit->append(u""_s);
    
    // If threats were detected, ask user what to do
    if (analysisResult.overallThreatLevel != Sandbox::ThreatLevel::NONE && 
        !analysisResult.detectedThreats.empty()) {
        
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Threats Detected in Sandbox"));
        msgBox.setText(tr("The sandbox analysis detected %1 threats.\nWhat would you like to do with this file?").arg(analysisResult.detectedThreats.size()));
        msgBox.setIcon(QMessageBox::Warning);
        
        QPushButton *quarantineButton = msgBox.addButton(tr("Quarantine File"), QMessageBox::ActionRole);
        QPushButton *deleteButton = msgBox.addButton(tr("Delete File"), QMessageBox::DestructiveRole);
        QPushButton *allowButton = msgBox.addButton(tr("Allow (Ignore)"), QMessageBox::AcceptRole);
        QPushButton *moreAnalysisButton = msgBox.addButton(tr("Deep Analysis"), QMessageBox::ActionRole);
        
        msgBox.exec();
        
        ui->sandboxResultsTextEdit->setTextColor(QColor(u"#FFFFFF"_s));
        ui->sandboxResultsTextEdit->append(tr("🎯 USER ACTION:"));
        
        if (msgBox.clickedButton() == quarantineButton) {
            ui->sandboxResultsTextEdit->setTextColor(QColor(u"#FF9800"_s));
            ui->sandboxResultsTextEdit->append(tr("🔒 Quarantining file..."));
              bool success = m_sandboxManager->processFileWithUserChoice(
                filePath.toStdString(),
                Sandbox::UserAction::QUARANTINE
            );
            
            if (success) {
                ui->sandboxResultsTextEdit->setTextColor(QColor(u"#4CAF50"_s));
                ui->sandboxResultsTextEdit->append(tr("✅ File successfully quarantined"));
            } else {
                ui->sandboxResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
                ui->sandboxResultsTextEdit->append(tr("❌ Failed to quarantine file"));
            }
            
        } else if (msgBox.clickedButton() == deleteButton) {
            ui->sandboxResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
            ui->sandboxResultsTextEdit->append(tr("🗑️  Deleting file..."));
              bool success = m_sandboxManager->processFileWithUserChoice(
                filePath.toStdString(),
                Sandbox::UserAction::DELETE
            );
            
            if (success) {
                ui->sandboxResultsTextEdit->setTextColor(QColor(u"#4CAF50"_s));
                ui->sandboxResultsTextEdit->append(tr("✅ File successfully deleted"));
            } else {
                ui->sandboxResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
                ui->sandboxResultsTextEdit->append(tr("❌ Failed to delete file"));
            }
            
        } else if (msgBox.clickedButton() == allowButton) {
            ui->sandboxResultsTextEdit->setTextColor(QColor(u"#FFA726"_s));
            ui->sandboxResultsTextEdit->append(tr("⚠️  File allowed despite threats"));
            ui->sandboxResultsTextEdit->append(tr("   User chose to ignore sandbox warnings"));
            
        } else if (msgBox.clickedButton() == moreAnalysisButton) {
            ui->sandboxResultsTextEdit->setTextColor(QColor(u"#2196F3"_s));
            ui->sandboxResultsTextEdit->append(tr("🔬 Performing deep analysis..."));
            
            // Perform deep analysis
            Sandbox::SandboxAnalysisResult deepResult = m_sandboxManager->analyzeFileForThreats(
                filePath.toStdString(), 
                Sandbox::MonitoringLevel::DEEP
            );
            
            ui->sandboxResultsTextEdit->append(tr("📊 Deep analysis completed"));
            ui->sandboxResultsTextEdit->append(tr("   Additional monitoring data collected"));
            
            // Display additional deep analysis results if different
            if (deepResult.detectedThreats.size() != analysisResult.detectedThreats.size()) {
                ui->sandboxResultsTextEdit->append(tr("   New threats found: %1").arg(deepResult.detectedThreats.size() - analysisResult.detectedThreats.size()));
            }
        }
    } else {
        ui->sandboxResultsTextEdit->setTextColor(QColor(u"#4CAF50"_s));
        ui->sandboxResultsTextEdit->append(tr("✅ File appears safe based on sandbox analysis"));
        ui->sandboxResultsTextEdit->append(tr("   No malicious behavior detected"));
    }
    
    ui->sandboxResultsTextEdit->append(u""_s);
    ui->sandboxResultsTextEdit->setTextColor(QColor(u"#9E9E9E"_s));
    ui->sandboxResultsTextEdit->append(tr("=== Sandbox Analysis Complete ==="));
}

/**
 * @brief Handles the Network Monitor button click.
 * 
 * Shows the network monitoring page and logs.
 */
void DashboardWidget::onNetworkMonitorButtonClicked() {
    qDebug() << "Network Monitor button clicked";
    ui->dashboardTabWidget->setCurrentWidget(ui->networkTab);
    
    // Get current timestamp
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timestamp = currentTime.toString(u"yyyy-MM-dd hh:mm:ss"_s);
    
    // Toggle the monitoring state with improved visual feedback
    if (m_networkMonitor->isMonitoring()) {
        m_networkMonitor->stopMonitoring();
        ui->networkMonitorButton->setText(u"Ağ İzlemeyi Başlat"_s);
        ui->networkMonitorButton->setStyleSheet(u"QPushButton { background-color: #2196F3; color: white; }"_s);
        
        // Add stop message with timestamp
        ui->networkCommunicationTextEdit->setTextColor(QColor(u"#FFA726"_s));
        ui->networkCommunicationTextEdit->append(u""_s);
        ui->networkCommunicationTextEdit->append(u"==== Ağ İzleme Durduruldu: "_s + timestamp + u" ===="_s);
    } else {
        // Clear previous content when starting new monitoring session
        ui->networkCommunicationTextEdit->clear();
        
        m_networkMonitor->startMonitoring();
        ui->networkMonitorButton->setText(u"Ağ İzlemeyi Durdur"_s);
        ui->networkMonitorButton->setStyleSheet(u"QPushButton { background-color: #F44336; color: white; }"_s);
        
        // Add start message with timestamp and header
        ui->networkCommunicationTextEdit->setTextColor(QColor(u"#4CAF50"_s));
        ui->networkCommunicationTextEdit->append(u"==== Ağ İzleme Başlatıldı: "_s + timestamp + u" ===="_s);
        ui->networkCommunicationTextEdit->append(u""_s);
        ui->networkCommunicationTextEdit->setTextColor(QColor(u"#E0E0E0"_s));
        ui->networkCommunicationTextEdit->append(u"Ağ trafiği izleniyor..."_s);
        ui->networkCommunicationTextEdit->append(u"Olası bağlantılar ve paketler burada gösterilecek."_s);
        ui->networkCommunicationTextEdit->append(u""_s);
    }
}

/**
 * @brief Handles the Refresh button click.
 * 
 * Refreshes the current view and any associated data.
 */
void DashboardWidget::onRefreshButtonClicked() {
    qDebug() << "Refresh button clicked";
    
    // Refresh the current active page
    QWidget* currentWidget = ui->dashboardTabWidget->currentWidget();
    
    if (currentWidget == ui->networkTab) {
        // If on network page, clear and restart monitoring
        if (ui->networkCommunicationTextEdit) {
            ui->networkCommunicationTextEdit->clear();
        }
        if (m_networkMonitor) {
            m_networkMonitor->stopMonitoring();
            m_networkMonitor->startMonitoring();
        }
    } else if (currentWidget == ui->basicScanTab) {
        // Clear basic scan results
        if (ui->basicScanResultsTextEdit) {
            ui->basicScanResultsTextEdit->clear();
        }
    } else if (currentWidget == ui->advancedScanTab) {
        // Clear advanced scan results
        if (ui->advancedScanResultsTableWidget) {
            ui->advancedScanResultsTableWidget->clearContents();
            ui->advancedScanResultsTableWidget->setRowCount(0);
        }
    } else if (currentWidget == ui->cdrTab) {
        // Clear CDR results
        if (ui->cdrResultsTextEdit) {
            ui->cdrResultsTextEdit->clear();
        }
    } else if (currentWidget == ui->sandboxTab) {
        // Clear sandbox results
        if (ui->sandboxResultsTextEdit) {
            ui->sandboxResultsTextEdit->clear();
        }
    }
}

/**
 * @brief Handles settings changes from the settings dialog.
 * 
 * Slot that responds to settings changes and updates the dashboard accordingly.
 */
void DashboardWidget::onSettingsChanged() {
    qDebug() << "Settings changed - updating dashboard";
    applySettingsChanges();
}

/**
 * @brief Applies settings changes to the dashboard.
 * 
 * Updates dashboard appearance, behavior, and configuration based on current settings.
 */
void DashboardWidget::applySettingsChanges() {
    // Get the current settings
    SettingsManager& settings = SettingsManager::getInstance();
    
    // Apply language settings
    QString language = settings.getLanguage();
    if (language == "tr") {
        // Apply Turkish localization
        ui->basicScanButton->setText(u"Temel Tarama"_s);
        ui->advancedScanButton->setText(u"Gelişmiş Tarama"_s);
        ui->cdrScanButton->setText(u"CDR Taraması"_s);
        ui->sandboxScanButton->setText(u"Sandbox Taraması"_s);
        ui->refreshButton->setText(u"Yenile"_s);
        
        // Update tab names
        ui->dashboardTabWidget->setTabText(ui->dashboardTabWidget->indexOf(ui->basicScanTab), u"Temel Tarama"_s);
        ui->dashboardTabWidget->setTabText(ui->dashboardTabWidget->indexOf(ui->advancedScanTab), u"Gelişmiş Tarama"_s);
        ui->dashboardTabWidget->setTabText(ui->dashboardTabWidget->indexOf(ui->cdrTab), u"CDR"_s);
        ui->dashboardTabWidget->setTabText(ui->dashboardTabWidget->indexOf(ui->sandboxTab), u"Sandbox"_s);
        ui->dashboardTabWidget->setTabText(ui->dashboardTabWidget->indexOf(ui->networkTab), u"Ağ İzleme"_s);
    } else {
        // Apply English localization
        ui->basicScanButton->setText("Basic Scan");
        ui->advancedScanButton->setText("Advanced Scan");
        ui->cdrScanButton->setText("CDR Scan");
        ui->sandboxScanButton->setText("Sandbox Scan");
        ui->refreshButton->setText("Refresh");
        
        // Update tab names
        ui->dashboardTabWidget->setTabText(ui->dashboardTabWidget->indexOf(ui->basicScanTab), "Basic Scan");
        ui->dashboardTabWidget->setTabText(ui->dashboardTabWidget->indexOf(ui->advancedScanTab), "Advanced Scan");
        ui->dashboardTabWidget->setTabText(ui->dashboardTabWidget->indexOf(ui->cdrTab), "CDR");
        ui->dashboardTabWidget->setTabText(ui->dashboardTabWidget->indexOf(ui->sandboxTab), "Sandbox");
        ui->dashboardTabWidget->setTabText(ui->dashboardTabWidget->indexOf(ui->networkTab), "Network Monitor");
    }
    
    // Apply theme settings
    QString theme = settings.getTheme();
    if (theme == "light") {
        // Apply light theme (currently not implemented, but placeholder for future)
        qDebug() << "Light theme not yet implemented";
    } else {
        // Keep the current dark theme
        qDebug() << "Using dark theme";
    }
    
    // Apply auto-scan settings to scanner if needed
    if (settings.getAutoScanEnabled() && m_basicScanner) {
        qDebug() << "Auto-scan is enabled";
        // Auto-scan logic could be implemented here
    }
    
    // Update network monitoring if settings changed
    if (m_networkMonitor) {
        // Could restart with new settings if needed
        qDebug() << "Network monitor settings updated";
    }
    
    qDebug() << "Dashboard settings applied successfully";
}

/**
 * @brief Handles file selection and initiates the basic scan process.
 * 
 * Opens a file dialog for the user to select a file, then initiates
 * the scan of that file and updates the UI accordingly.
 */
void DashboardWidget::onBasicScanSelectFile() {
    ui->dashboardTabWidget->setCurrentWidget(ui->basicScanTab);
    ui->basicScanResultsTextEdit->clear();
    ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::SELECTING_FILE));
    
    if (m_basicScanner->selectFile()) {
        ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::FILE_SELECTED).arg(
            m_basicScanner->getSelectedFile().fileName()));
        ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::SCANNING_FILE));
        
        // Initiate scan with selected file
        if (!m_basicScanner->scanFile(m_basicScanner->getSelectedFile().filePath())) {
            // Handle scan initiation error - already handled by error signal, 
            // but we can add additional UI updates if needed
            if (m_basicScanner->getLastErrorCode() != ScannerErrorCode::NoError) {
                ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::ERROR_PREFIX).arg(
                    m_basicScanner->getLastErrorMessage()));
            }
        }
    } else {
        if (m_basicScanner->getLastErrorCode() == ScannerErrorCode::NoError) {
            // User canceled file selection, not an error
            ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::SELECTION_CANCELED));
        } else {
            // Handle file selection error
            ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::ERROR_PREFIX).arg(
                m_basicScanner->getLastErrorMessage()));
        }
    }
}

/**
 * @brief Handles directory selection and initiates the basic scan process for all files in the directory.
 * 
 * Opens a directory dialog for the user to select a directory, then iterates through
 * all files in the directory and scans each one.
 */
void DashboardWidget::onBasicScanSelectDirectory() {
    ui->dashboardTabWidget->setCurrentWidget(ui->basicScanTab);
    ui->basicScanResultsTextEdit->clear();
    ui->basicScanResultsTextEdit->append(QString::fromUtf8("Dizin seçiliyor..."));
    
    QString dirPath = QFileDialog::getExistingDirectory(this, 
                                                        tr("Tarama için Dizin Seçin"), 
                                                        QDir::homePath(), 
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (!dirPath.isEmpty()) {
        QDir directory(dirPath);
        QStringList filters;
        filters << "*"; // Scan all files
        QFileInfoList fileList = directory.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
        
        ui->basicScanResultsTextEdit->append(QString::fromUtf8("Seçilen dizin: %1").arg(QFileInfo(dirPath).baseName()));
        ui->basicScanResultsTextEdit->append(QString::fromUtf8("Bulunan dosya sayısı: %1").arg(fileList.size()));
        ui->basicScanResultsTextEdit->append(QString::fromUtf8("Dizin taraması başlatılıyor...\n"));
        
        if (fileList.isEmpty()) {
            ui->basicScanResultsTextEdit->append(QString::fromUtf8("⚠️ Seçilen dizinde taranabilir dosya bulunamadı."));
            return;
        }
        
        int totalFiles = fileList.size();
        int scannedFiles = 0;
        int maliciousFiles = 0;
        
        // Scan each file in the directory
        for (const QFileInfo& fileInfo : fileList) {
            scannedFiles++;
            QString filePath = fileInfo.absoluteFilePath();
            
            // Update progress
            ui->basicScanResultsTextEdit->append(QString::fromUtf8("\n[%1/%2] Taranıyor: %3")
                                                 .arg(scannedFiles)
                                                 .arg(totalFiles)
                                                 .arg(fileInfo.fileName()));
            
            // Scan the file synchronously for directory scanning
            if (m_basicScanner->scanFile(filePath)) {
                // Process the app events to prevent freezing
                QCoreApplication::processEvents();
                
                // Wait for scan to complete (simplified synchronous approach for directory scanning)
                // In a real implementation, you might want to use a more sophisticated approach
                // For now, we'll assume the scan completes quickly for each file
                QString results = m_basicScanner->getResults();
                
                if (results.contains("MALICIOUS", Qt::CaseInsensitive)) {
                    maliciousFiles++;
                    ui->basicScanResultsTextEdit->setTextColor(QColor("#FF5252"));
                    ui->basicScanResultsTextEdit->append(QString::fromUtf8("  ⚠️ MALİCİOUS - %1").arg(fileInfo.fileName()));
                } else {
                    ui->basicScanResultsTextEdit->setTextColor(QColor("#4CAF50"));
                    ui->basicScanResultsTextEdit->append(QString::fromUtf8("  ✓ TEMİZ - %1").arg(fileInfo.fileName()));
                }
                ui->basicScanResultsTextEdit->setTextColor(QColor("#FFFFFF")); // Reset color
                
            } else {
                ui->basicScanResultsTextEdit->setTextColor(QColor("#FFA726"));
                ui->basicScanResultsTextEdit->append(QString::fromUtf8("  ⚠️ HATA - %1: %2")
                                                     .arg(fileInfo.fileName())
                                                     .arg(m_basicScanner->getLastErrorMessage()));
                ui->basicScanResultsTextEdit->setTextColor(QColor("#FFFFFF")); // Reset color
            }
        }
        
        // Display summary
        ui->basicScanResultsTextEdit->append(QString::fromUtf8("\n=== DIZIN TARAMA SONUCU ==="));
        ui->basicScanResultsTextEdit->append(QString::fromUtf8("Toplam dosya: %1").arg(totalFiles));
        ui->basicScanResultsTextEdit->append(QString::fromUtf8("Taranan dosya: %1").arg(scannedFiles));
        ui->basicScanResultsTextEdit->append(QString::fromUtf8("Zararlı dosya: %1").arg(maliciousFiles));
        ui->basicScanResultsTextEdit->append(QString::fromUtf8("Temiz dosya: %1").arg(scannedFiles - maliciousFiles));
        
        if (maliciousFiles > 0) {
            ui->basicScanResultsTextEdit->setTextColor(QColor("#FF5252"));
            ui->basicScanResultsTextEdit->append(QString::fromUtf8("\n⚠️ DİKKAT: Dizinde zararlı dosya tespit edildi!"));
        } else {
            ui->basicScanResultsTextEdit->setTextColor(QColor("#4CAF50"));
            ui->basicScanResultsTextEdit->append(QString::fromUtf8("\n✓ Dizin temiz - zararlı dosya bulunamadı."));
        }
        ui->basicScanResultsTextEdit->setTextColor(QColor("#FFFFFF")); // Reset color
        
    } else {
        ui->basicScanResultsTextEdit->append(QString::fromUtf8("Dizin seçimi iptal edildi."));
    }
}

/**
 * @brief Append new network log messages to the network monitoring display.
 * 
 * Updates the network monitoring interface with new log messages
 * from the network monitor component.
 * 
 * @param logMessage New network log message to display
 */
void DashboardWidget::appendNetworkLog(const QString& logMessage) {
    if (ui->networkCommunicationTextEdit) {
        ui->networkCommunicationTextEdit->setTextColor(QColor(u"#E0E0E0"_s));
        ui->networkCommunicationTextEdit->append(logMessage);
    }
}

/**
 * @brief Handles and displays scanner errors.
 * 
 * @param errorCode The error code.
 * @param errorMessage A descriptive error message.
 */
void DashboardWidget::onBasicScanError(ScannerErrorCode errorCode, const QString& errorMessage) {
    ui->dashboardTabWidget->setCurrentWidget(ui->basicScanTab);
    
    if (ui->basicScanResultsTextEdit) {
        ui->basicScanResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
        ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::ERROR_PREFIX).arg(errorMessage));
        ui->basicScanResultsTextEdit->setTextColor(QColor(u"#FFFFFF"_s)); // Reset color
    }
    
    qDebug() << "Basic scan error:" << static_cast<int>(errorCode) << errorMessage;
}

/**
 * @brief Adds a result entry to the advanced scan results table.
 * 
 * Helper method to add scan results to the advanced scan table with proper formatting.
 * 
 * @param scanType The type of scan performed
 * @param status The status of the scan
 * @param details Detailed information about the scan result
 */
void DashboardWidget::addAdvancedScanResult(const QString& scanType, const QString& status, const QString& details) {
    if (!ui->advancedScanResultsTableWidget) {
        return;
    }
    
    int rowCount = ui->advancedScanResultsTableWidget->rowCount();
    ui->advancedScanResultsTableWidget->insertRow(rowCount);
    
    ui->advancedScanResultsTableWidget->setItem(rowCount, 0, new QTableWidgetItem(scanType));
    ui->advancedScanResultsTableWidget->setItem(rowCount, 1, new QTableWidgetItem(status));
    ui->advancedScanResultsTableWidget->setItem(rowCount, 2, new QTableWidgetItem(details));
}

/**
 * @brief Handle results from VirusTotal analysis.
 * 
 * Processes and displays analysis results received from the VirusTotal
 * cloud scanning service.
 * 
 * @param results Formatted string containing VirusTotal analysis results
 */
void DashboardWidget::handleVirusTotalResults(const QString& results) {
    ui->dashboardTabWidget->setCurrentWidget(ui->advancedScanTab);
    
    // Parse the JSON and add engine results to table
    addVirusTotalEngineResults(results);
    
    qDebug() << "VirusTotal results received:" << results;
}

/**
 * @brief Processes and displays scan results.
 * 
 * @param results String containing the scan results.
 */
void DashboardWidget::onBasicScanResultsReady(const QString& results) {
    ui->dashboardTabWidget->setCurrentWidget(ui->basicScanTab);
    
    if (ui->basicScanResultsTextEdit) {
        ui->basicScanResultsTextEdit->setTextColor(QColor(u"#4CAF50"_s));
        ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::VT_ANALYSIS_RESULTS));
        ui->basicScanResultsTextEdit->setTextColor(QColor(u"#E0E0E0"_s));
        ui->basicScanResultsTextEdit->append(results);
        ui->basicScanResultsTextEdit->setTextColor(QColor(u"#FFFFFF"_s)); // Reset color
    }
    
    qDebug() << "Basic scan results ready:" << results;
}

/**
 * @brief Processes and displays CDR scan results.
 * 
 * @param results String containing the CDR scan results.
 */
void DashboardWidget::onCdrScanResultsReady(const QString& results) {
    qDebug() << "DashboardWidget::onCdrScanResultsReady - Received CDR results. Updating CDR tab.";

    // Ensure we are on the CDR tab or switch to it.
    ui->dashboardTabWidget->setCurrentWidget(ui->cdrTab);

    // Update the QTextEdit within the CDR tab.
    // ASSUMPTION: The QTextEdit is named ui->cdrResultsTextEdit.
    // If this is not the correct name, please provide the correct one.
    if (ui->cdrResultsTextEdit) {
        // The 'results' string from CDRScanner should be formatted for display.
        // Initial messages (like "Starting CDR Scan...") are likely already in ui->cdrResultsTextEdit
        // from the onCdrScanButtonClicked method. We will append the new results.
        
        // Append the full, formatted results.
        // Using a common text color, adjust if your theme requires something specific.
        ui->cdrResultsTextEdit->setTextColor(QColor(u"#E0E0E0"_s)); 
        ui->cdrResultsTextEdit->append(results);
        ui->cdrResultsTextEdit->append(""); // Add a blank line for better separation.
    } else {
        qWarning() << "ui->cdrResultsTextEdit is null! Cannot display CDR scan results on the CDR tab.";
        // Fallback: If the specific CDR text edit isn't found, try to log to basic scan results.
        if (ui->basicScanResultsTextEdit) {
            ui->basicScanResultsTextEdit->append(tr("ERROR: CDR results widget (ui->cdrResultsTextEdit) not found. CDR Results: %1").arg(results));
        }
    }

    // Show a QMessageBox to notify the user that the scan is complete.
    QMessageBox::information(this, tr("CDR Scan Completed"),
                             tr("The CDR scan has finished.\\nView detailed results in the CDR tab."));

    qDebug() << "CDR scan results processed by DashboardWidget. Results should be on CDR tab and QMessageBox shown.";
}

/**
 * @brief Handles and displays CDR scanner errors.
 * 
 * @param errorCode The error code.
 * @param errorMessage A descriptive error message.
 */
void DashboardWidget::onCdrScanError(ScannerErrorCode errorCode, const QString& errorMessage) {
    qDebug() << "DashboardWidget::onCdrScanError - Received CDR error: Code %1, Message: %2";

    // Switch to the CDR tab to display the error.
    ui->dashboardTabWidget->setCurrentWidget(ui->cdrTab);

    // Update the QTextEdit within the CDR tab with error information.
    // ASSUMPTION: The QTextEdit is named ui->cdrResultsTextEdit.
    if (ui->cdrResultsTextEdit) {
        ui->cdrResultsTextEdit->setTextColor(QColorConstants::Red); // Use red for errors.
        ui->cdrResultsTextEdit->append(tr("--- CDR SCAN ERROR ---"));
        ui->cdrResultsTextEdit->append(tr("An error occurred during the CDR scan:"));
        ui->cdrResultsTextEdit->append(tr("Error Code: %1").arg(static_cast<int>(errorCode)));
        ui->cdrResultsTextEdit->append(tr("Message: %1").arg(errorMessage));
        ui->cdrResultsTextEdit->append(""); // Add a blank line.
    } else {
        qWarning() << "ui->cdrResultsTextEdit is null! Cannot display CDR scan error on the CDR tab.";
        // Fallback: If the specific CDR text edit isn't found, try to log to basic scan results.
        if (ui->basicScanResultsTextEdit) {
            ui->basicScanResultsTextEdit->append(tr("CDR SCAN ERROR: %1 (Code: %2)").arg(errorMessage).arg(static_cast<int>(errorCode)));
        }
    }

    // Show a QMessageBox to notify the user about the error.
    QMessageBox::critical(this, tr("CDR Scan Error"),
                          tr("An error occurred during the CDR scan:\\n%1").arg(errorMessage));
    
    qDebug() << "CDR scan error processed by DashboardWidget. Error should be on CDR tab and QMessageBox shown.";
}

/**
 * @brief Handles the file selection for Advanced Scan.
 * 
 * Opens a file dialog and processes the selected file for advanced scanning.
 */
void DashboardWidget::onAdvancedScanSelectFile() {
    QString filePath = QFileDialog::getOpenFileName(this, 
                                                    tr("Select File for Advanced Scan"), 
                                                    QDir::homePath(), 
                                                    tr("All Files (*.*)"));
    
    if (!filePath.isEmpty()) {
        startAdvancedScanWithPath(filePath, "file");
    }
}

/**
 * @brief Starts advanced scanning with the specified path and selection type.
 * 
 * Initiates an advanced scan operation using the provided path and type.
 * This function is called internally after path selection in advanced scan.
 * 
 * @param path The file or directory path to scan
 * @param type The selection type (file or directory)
 */
void DashboardWidget::startAdvancedScanWithPath(const QString &path, const QString &type) {
    ui->dashboardTabWidget->setCurrentWidget(ui->advancedScanTab);
    
    // Clear previous results
    if (ui->advancedScanResultsTableWidget) {
        ui->advancedScanResultsTableWidget->clearContents();
        ui->advancedScanResultsTableWidget->setRowCount(0);
    }
    
    // Update UI to show selected path
    if (ui->selectedPathLabel) {
        ui->selectedPathLabel->setText(tr("Selected %1: %2").arg(type, path));
    }
    
    // Add initial scan entry
    addAdvancedScanResult(QString::fromUtf8(DashboardText::ADVANCED_SCAN), tr("Starting"), 
                         tr("Initiating advanced scan for: %1").arg(path));
    
    // Start actual VirusTotal scanning process
    if (m_virusTotalManager) {
        // Set the file to be scanned
        m_virusTotalManager->setFile(path);
        
        // Add status update for file submission
        addAdvancedScanResult(tr("File Submission"), tr("In Progress"), tr("Submitting file to VirusTotal..."));
        
        // Start the scan
        bool scanStarted = m_virusTotalManager->scanFile(path);
        
        if (scanStarted) {
            addAdvancedScanResult(tr("VirusTotal Analysis"), tr("In Progress"), 
                                tr("File uploaded successfully. Analysis in progress..."));
            qDebug() << "VirusTotal scan started for file:" << path;
        } else {
            QString error = m_virusTotalManager->getLastError();
            addAdvancedScanResult(tr("Scan Error"), tr("Failed"), 
                                tr("Failed to start scan: %1").arg(error.isEmpty() ? tr("Unknown error") : error));
            qDebug() << "Failed to start VirusTotal scan:" << error;
        }
    } else {
        addAdvancedScanResult(tr("System Error"), tr("Failed"), 
                            tr("VirusTotal manager not available"));
        qDebug() << "VirusTotal manager not initialized";
    }
    
    qDebug() << "Starting advanced scan for" << type << ":" << path;
}
 
/**
 * @brief Format VirusTotal JSON results into user-friendly text
 * 
 * Parses the raw JSON response from VirusTotal and formats it into
 * a readable summary for display in the UI.
 * 
 * @param jsonResults Raw JSON string from VirusTotal API
 * @return Formatted string suitable for display
 */
QString DashboardWidget::formatVirusTotalResults(const QString& jsonResults) {
    QJsonDocument doc = QJsonDocument::fromJson(jsonResults.toUtf8());
    
    if (doc.isNull() || !doc.isObject()) {
        return tr("Error: Invalid VirusTotal response format");
    }
    
    QJsonObject root = doc.object();
    
    // Check if this is still in queue or analysing
    if (root.contains("data") && root["data"].isObject()) {
        QJsonObject data = root["data"].toObject();
        
        if (data.contains("attributes") && data["attributes"].isObject()) {
            QJsonObject attrs = data["attributes"].toObject();
            QString status = attrs.value("status").toString();
            
            // Handle non-completed analysis
            if (status == "queued") {
                return tr("Analysis Status: Queued\nYour file is waiting in the analysis queue. Please wait...");
            } else if (status == "analysing") {
                return tr("Analysis Status: In Progress\nVirusTotal engines are currently analyzing your file...");
            }
            
            // Handle completed analysis
            if (status == "completed" && attrs.contains("results") && attrs["results"].isObject()) {
                QJsonObject results = attrs["results"].toObject();
                QJsonObject stats = attrs.value("stats").toObject();
                
                // Extract key statistics
                int malicious = stats.value("malicious").toInt();
                int suspicious = stats.value("suspicious").toInt();
                int undetected = stats.value("undetected").toInt();
                int harmless = stats.value("harmless").toInt();
                int timeout = stats.value("timeout").toInt();
                int failure = stats.value("failure").toInt();
                int typeUnsupported = stats.value("type-unsupported").toInt();
                
                int totalEngines = malicious + suspicious + undetected + harmless + timeout + failure;
                
                QString summary;
                summary += tr("=== VirusTotal Analysis Summary ===\n\n");
                
                // Overall threat assessment
                if (malicious > 0) {
                    summary += tr("⚠️  THREAT DETECTED!\n");
                    summary += tr("Malicious detections: %1/%2 engines\n\n").arg(malicious).arg(totalEngines);
                } else if (suspicious > 0) {
                    summary += tr("⚠️  SUSPICIOUS ACTIVITY\n");
                    summary += tr("Suspicious detections: %1/%2 engines\n\n").arg(suspicious).arg(totalEngines);
                } else {
                    summary += tr("✅ CLEAN\n");
                    summary += tr("No malicious content detected\n\n");
                }
                
                // Detailed statistics
                summary += tr("📊 Detection Statistics:\n");
                if (malicious > 0) summary += tr("• Malicious: %1\n").arg(malicious);
                if (suspicious > 0) summary += tr("• Suspicious: %1\n").arg(suspicious);
                if (undetected > 0) summary += tr("• Undetected: %1\n").arg(undetected);
                if (harmless > 0) summary += tr("• Harmless: %1\n").arg(harmless);
                if (timeout > 0) summary += tr("• Timeout: %1\n").arg(timeout);
                if (failure > 0) summary += tr("• Failed: %1\n").arg(failure);
                if (typeUnsupported > 0) summary += tr("• Type Unsupported: %1\n").arg(typeUnsupported);
                
                // List malicious detections
                if (malicious > 0) {
                    summary += tr("\n🔍 Malicious Detections:\n");
                    for (auto it = results.begin(); it != results.end(); ++it) {
                        QJsonObject engine = it.value().toObject();
                        QString category = engine.value("category").toString();
                        QString result = engine.value("result").toString();
                        
                        if (category == "malicious" && !result.isEmpty()) {
                            summary += tr("• %1: %2\n").arg(it.key(), result);
                        }
                    }
                }
                
                // List suspicious detections
                if (suspicious > 0) {
                    summary += tr("\n⚠️  Suspicious Detections:\n");
                    for (auto it = results.begin(); it != results.end(); ++it) {
                        QJsonObject engine = it.value().toObject();
                        QString category = engine.value("category").toString();
                        QString result = engine.value("result").toString();
                        
                        if (category == "suspicious" && !result.isEmpty()) {
                            summary += tr("• %1: %2\n").arg(it.key(), result);
                        }
                    }
                }
                
                // File metadata if available
                if (root.contains("meta") && root["meta"].isObject()) {
                    QJsonObject meta = root["meta"].toObject();
                    if (meta.contains("file_info") && meta["file_info"].isObject()) {
                        QJsonObject fileInfo = meta["file_info"].toObject();
                        summary += tr("\n📄 File Information:\n");
                        if (fileInfo.contains("size")) {
                            int size = fileInfo.value("size").toInt();
                            summary += tr("• File Size: %1 bytes\n").arg(size);
                        }
                        if (fileInfo.contains("md5")) {
                            summary += tr("• MD5: %1\n").arg(fileInfo.value("md5").toString());
                        }
                        if (fileInfo.contains("sha1")) {
                            summary += tr("• SHA1: %1\n").arg(fileInfo.value("sha1").toString());
                        }
                        if (fileInfo.contains("sha256")) {
                            summary += tr("• SHA256: %1\n").arg(fileInfo.value("sha256").toString());
                        }
                    }
                }
                
                return summary;
            }
        }
    }
    
    // Fallback for unexpected format
    return tr("Analysis completed but results format is unexpected.\n\nRaw response: %1").arg(jsonResults.left(500));
}

/**
 * @brief Add VirusTotal engine results to the advanced scan table
 * 
 * Parses VirusTotal JSON response and adds each engine's result 
 * as a separate row in the advanced scan results table.
 * 
 * @param jsonResults Raw JSON string from VirusTotal API
 */
void DashboardWidget::addVirusTotalEngineResults(const QString& jsonResults) {
    QJsonDocument doc = QJsonDocument::fromJson(jsonResults.toUtf8());
    
    if (doc.isNull() || !doc.isObject()) {
        addAdvancedScanResult(tr("VirusTotal Error"), tr("Failed"), tr("Invalid response format"));
        return;
    }
    
    QJsonObject root = doc.object();
    
    // Check if this is still in queue or analysing
    if (root.contains("data") && root["data"].isObject()) {
        QJsonObject data = root["data"].toObject();
        
        if (data.contains("attributes") && data["attributes"].isObject()) {
            QJsonObject attrs = data["attributes"].toObject();
            QString status = attrs.value("status").toString();
            
            // Handle non-completed analysis
            if (status == "queued") {
                addAdvancedScanResult(tr("VirusTotal"), tr("Queued"), 
                                    tr("File is waiting in the analysis queue..."));
                return;
            } else if (status == "analysing") {
                addAdvancedScanResult(tr("VirusTotal"), tr("Analyzing"), 
                                    tr("Engines are currently analyzing the file..."));
                return;
            }
            
            // Handle completed analysis
            if (status == "completed" && attrs.contains("results") && attrs["results"].isObject()) {
                QJsonObject results = attrs["results"].toObject();
                QJsonObject stats = attrs.value("stats").toObject();
                
                // Extract key statistics
                int malicious = stats.value("malicious").toInt();
                int suspicious = stats.value("suspicious").toInt();
                int undetected = stats.value("undetected").toInt();
                int harmless = stats.value("harmless").toInt();
                int timeout = stats.value("timeout").toInt();
                int failure = stats.value("failure").toInt();
                int totalEngines = malicious + suspicious + undetected + harmless + timeout + failure;
                
                // Add summary row first
                QString summaryStatus;
                QString summaryDetails;
                if (malicious > 0) {
                    summaryStatus = tr("⚠️ THREAT DETECTED");
                    summaryDetails = tr("Malicious: %1/%2 engines detected threats").arg(malicious).arg(totalEngines);
                } else if (suspicious > 0) {
                    summaryStatus = tr("⚠️ SUSPICIOUS");
                    summaryDetails = tr("Suspicious: %1/%2 engines flagged suspicious activity").arg(suspicious).arg(totalEngines);
                } else {
                    summaryStatus = tr("✅ CLEAN");
                    summaryDetails = tr("Clean: %1/%2 engines found no threats").arg(totalEngines).arg(totalEngines);
                }
                
                addAdvancedScanResult(tr("📊 VirusTotal Summary"), summaryStatus, summaryDetails);
                
                // Add file information if available
                if (root.contains("meta") && root["meta"].isObject()) {
                    QJsonObject meta = root["meta"].toObject();
                    if (meta.contains("file_info") && meta["file_info"].isObject()) {
                        QJsonObject fileInfo = meta["file_info"].toObject();
                        QString fileDetails;
                        if (fileInfo.contains("size")) {
                            int size = fileInfo.value("size").toInt();
                            fileDetails += tr("Size: %1 bytes").arg(size);
                        }
                        if (fileInfo.contains("sha256")) {
                            if (!fileDetails.isEmpty()) fileDetails += "\n";
                            fileDetails += tr("SHA256: %1").arg(fileInfo.value("sha256").toString());
                        }
                        addAdvancedScanResult(tr("📄 File Info"), tr("Details"), fileDetails);
                    }
                }
                
                // Now add each engine result as a separate row
                QStringList engineKeys = results.keys();
                std::sort(engineKeys.begin(), engineKeys.end());
                
                for (const QString& engineName : engineKeys) {
                    QJsonObject engine = results[engineName].toObject();
                    QString category = engine.value("category").toString();
                    QString result = engine.value("result").toString();
                    QString method = engine.value("method").toString();
                    QString engineVersion = engine.value("engine_version").toString();
                    
                    QString status;
                                       QString details;
                    
                    if (category == "malicious") {
                        status = tr("🔴 MALICIOUS");
                        details = result.isEmpty() ? tr("Threat detected") : result;
                    } else if (category == "suspicious") {
                        status = tr("🟡 SUSPICIOUS");
                        details = result.isEmpty() ? tr("Suspicious activity") : result;
                    } else if (category == "undetected") {
                        status = tr("✅ CLEAN");
                        details = tr("No threats detected");
                    } else if (category == "harmless") {
                        status = tr("✅ HARMLESS");
                        details = tr("File is harmless");
                    } else if (category == "timeout") {
                        status = tr("⏱️ TIMEOUT");
                        details = tr("Analysis timed out");
                    } else if (category == "failure") {
                        status = tr("❌ FAILED");
                        details = tr("Analysis failed");
                    } else if (category == "type-unsupported") {
                        status = tr("❓ UNSUPPORTED");
                        details = tr("File type not supported");
                    } else {
                        status = category.toUpper();
                        details = result.isEmpty() ? tr("Unknown result") : result;
                    }
                    
                    // Add engine version info if available
                    if (!engineVersion.isEmpty()) {
                        details += tr("\nEngine: v%1").arg(engineVersion);
                    }
                    
                    addAdvancedScanResult(engineName, status, details);
                }
                
                return;
            }
        }
    }
    
    // Fallback for unexpected format
    addAdvancedScanResult(tr("VirusTotal Error"), tr("Unexpected Format"), 
                         tr("Analysis completed but response format is unexpected"));
}