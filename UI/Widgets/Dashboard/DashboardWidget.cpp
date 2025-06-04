#include "DashboardWidget.h"
#include "ui_dashboardwidget.h"
#include "../../../Database/DatabaseService/DatabaseService.h"
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
    m_cdrScanner(std::make_unique<CDRScanner>()), // Initialize CDRScanner
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
    
    // Config and refresh buttons
    QString configButtonStyle = buttonStyleBase;
    ui->configButton->setStyleSheet(configButtonStyle);
    ui->configButton->setText(u"Yapılandırma"_s); // Turkish localization
    
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
    connect(ui->configButton, &QPushButton::clicked, this, &DashboardWidget::onConfigButtonClicked);
    connect(ui->refreshButton, &QPushButton::clicked, this, &DashboardWidget::onRefreshButtonClicked);
    
    // Basic scan connections
    connect(m_basicScanner.get(), &BasicScanner::scanResultsReady, this, &DashboardWidget::onBasicScanResultsReady);
    connect(m_basicScanner.get(), &BasicScanner::scanError, this, &DashboardWidget::onBasicScanError);
    
    // VirusTotal connections
    connect(m_virusTotalManager.get(), &VirusTotalManager::analysisResultsReady, this, &DashboardWidget::handleVirusTotalResults);

    // Network Monitor connections
    connect(m_networkMonitor, &NetworkMonitor::newLogMessage, this, &DashboardWidget::appendNetworkLog);
    m_networkMonitor->startMonitoring(); // Start monitoring
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
 * Initiates the file selection process for a basic scan.
 */
void DashboardWidget::onBasicScanButtonClicked() {
    onBasicScanSelectFile();
}

/**
 * @brief Handles the Advanced Scan button click.
 * 
 * Opens a file dialog for the user to select a file for advanced scanning.
 */
void DashboardWidget::onAdvancedScanButtonClicked() {
    onAdvancedScanSelectFile();
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
    ui->cdrResultsTextEdit->append(tr("Initiating CDR analysis..."));    if (m_cdrManager) {
        // Perform detailed CDR analysis
        ui->cdrResultsTextEdit->append(tr("🔍 Analyzing file structure..."));
        
        CDR::CdrConfiguration config;
        config.analysisType = CDR::AnalysisType::COMPREHENSIVE_SCAN;
        config.autoSanitize = true;
        config.preserveOriginal = false;
          // Create temporary directories for CDR processing
        QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch());
        QString inputDir = tempDir + u"/cdr_dashboard_input_"_s + timestamp;
        QString outputDir = tempDir + u"/cdr_dashboard_output_"_s + timestamp;
        QString quarantineDir = tempDir + u"/cdr_dashboard_quarantine_"_s + timestamp;
        
        // Create the directories
        if (!QDir().mkpath(inputDir)) {
            ui->cdrResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
            ui->cdrResultsTextEdit->append(tr("❌ Failed to create temporary input directory"));
            return;
        }
        if (!QDir().mkpath(outputDir)) {
            ui->cdrResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
            ui->cdrResultsTextEdit->append(tr("❌ Failed to create temporary output directory"));
            return;
        }
        if (!QDir().mkpath(quarantineDir)) {
            ui->cdrResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
            ui->cdrResultsTextEdit->append(tr("❌ Failed to create temporary quarantine directory"));
            return;
        }
          // Set the CDR configuration directories
        config.inputDirectory = inputDir.toStdString();
        config.outputDirectory = outputDir.toStdString();
        config.quarantineDirectory = quarantineDir.toStdString();
          // Copy the selected file to the input directory for analysis
        QString selectedFileName = QFileInfo(filePath).fileName();
        QString inputFilePath = inputDir + u"/"_s + selectedFileName;
        
        if (!QFile::copy(filePath, inputFilePath)) {
            ui->cdrResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
            ui->cdrResultsTextEdit->append(tr("❌ Failed to copy file to input directory"));
            return;
        }
        
        ui->cdrResultsTextEdit->append(tr("📂 File prepared for analysis: %1").arg(selectedFileName));
          try {
            // Start analysis on the input directory
            std::string analysisId = m_cdrManager->startAnalysis(
                inputDir.toStdString(),
                config
            );
            
            ui->cdrResultsTextEdit->append(tr("🔄 Analysis started with ID: %1").arg(QString::fromStdString(analysisId)));
            
            // Poll for analysis completion (simplified approach)
            int maxAttempts = 30; // 30 seconds timeout
            int attempts = 0;
            CDR::CdrAnalysisResult result;
            
            while (attempts < maxAttempts) {
                result = m_cdrManager->getAnalysisStatus(analysisId);
                
                if (result.status == "completed" || result.status == "failed") {
                    break;
                }
                
                // Update progress
                if (attempts % 5 == 0) { // Update every 5 seconds
                    ui->cdrResultsTextEdit->append(tr("⏳ Analysis in progress... (%1%)").arg(result.progressPercentage));
                }
                
                QThread::msleep(1000); // Wait 1 second
                QCoreApplication::processEvents(); // Keep UI responsive
                attempts++;
            }
              if (attempts >= maxAttempts) {
                ui->cdrResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
                ui->cdrResultsTextEdit->append(tr("⏰ Analysis timed out"));
                return;
            }
            
            // Analysis completed, process results
            if (result.status == "completed" && result.isSafe) {
            ui->cdrResultsTextEdit->setTextColor(QColor(u"#4CAF50"_s)); // Green for success
            ui->cdrResultsTextEdit->append(tr("✅ Analysis completed successfully"));
            ui->cdrResultsTextEdit->append(u""_s);
            
            // Display analysis results
            ui->cdrResultsTextEdit->setTextColor(QColor(u"#E0E0E0"_s));
            ui->cdrResultsTextEdit->append(tr("📊 Analysis Results:"));
            ui->cdrResultsTextEdit->append(tr("  • Files Scanned: %1").arg(result.totalFilesScanned));
            ui->cdrResultsTextEdit->append(tr("  • Clean Files: %1").arg(result.cleanFiles.size()));
            ui->cdrResultsTextEdit->append(tr("  • Threats Detected: %1").arg(result.threatsDetected));
            ui->cdrResultsTextEdit->append(tr("  • Files Quarantined: %1").arg(result.filesQuarantined));
            
            if (result.threatsDetected > 0) {
                ui->cdrResultsTextEdit->setTextColor(QColor(u"#FF5252"_s)); // Red for threats
                ui->cdrResultsTextEdit->append(u""_s);
                ui->cdrResultsTextEdit->append(tr("⚠️  THREATS DETECTED:"));
                  for (const auto& quarantinedFile : result.quarantinedFiles) {
                    ui->cdrResultsTextEdit->append(tr("  🚨 Quarantined: %1").arg(QString::fromStdString(quarantinedFile)));
                }
                ui->cdrResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
                ui->cdrResultsTextEdit->append(u""_s);
                
                // Ask user what to do with threats
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("Threats Detected"));
                msgBox.setText(tr("Malicious content has been detected in the file.\nWhat would you like to do?"));
                msgBox.setIcon(QMessageBox::Warning);
                
                QPushButton *cleanButton = msgBox.addButton(tr("Clean & Sanitize"), QMessageBox::ActionRole);
                QPushButton *deleteButton = msgBox.addButton(tr("Delete File"), QMessageBox::DestructiveRole);
                QPushButton *ignoreButton = msgBox.addButton(tr("Ignore"), QMessageBox::RejectRole);
                
                msgBox.exec();
                
                if (msgBox.clickedButton() == cleanButton) {
                    ui->cdrResultsTextEdit->setTextColor(QColor(u"#2196F3"_s));
                    ui->cdrResultsTextEdit->append(tr("🔧 Cleaning and sanitizing file..."));
                    
                    QString outputPath = filePath + QStringLiteral("_cleaned");
                    bool sanitizeSuccess = m_cdrManager->sanitizeFile(
                        filePath.toStdString(), 
                        outputPath.toStdString(), 
                        config
                    );
                    
                    if (sanitizeSuccess) {
                        ui->cdrResultsTextEdit->setTextColor(QColor(u"#4CAF50"_s));
                        ui->cdrResultsTextEdit->append(tr("✅ File successfully cleaned!"));
                        ui->cdrResultsTextEdit->append(tr("📁 Clean file saved to: %1").arg(outputPath));
                    } else {
                        ui->cdrResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
                        ui->cdrResultsTextEdit->append(tr("❌ Failed to clean file"));
                    }
                } else if (msgBox.clickedButton() == deleteButton) {
                    ui->cdrResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
                    ui->cdrResultsTextEdit->append(tr("🗑️  Deleting file..."));
                    if (QFile::remove(filePath)) {
                        ui->cdrResultsTextEdit->append(tr("✅ File successfully deleted"));
                    } else {
                        ui->cdrResultsTextEdit->append(tr("❌ Failed to delete file"));
                    }
                } else {
                    ui->cdrResultsTextEdit->setTextColor(QColor(u"#FFA726"_s));
                    ui->cdrResultsTextEdit->append(tr("⚠️  User chose to ignore threats"));
                }
            } else {
                ui->cdrResultsTextEdit->setTextColor(QColor(u"#4CAF50"_s));
                ui->cdrResultsTextEdit->append(tr("✅ No threats detected - file is clean!"));
            }
        } else {
            ui->cdrResultsTextEdit->setTextColor(QColor(u"#FF5252"_s)); // Red for errors
            ui->cdrResultsTextEdit->append(tr("❌ CDR analysis failed"));
            ui->cdrResultsTextEdit->append(tr("Error: %1").arg(QString::fromStdString(result.errorMessage)));
        }
        } catch (const std::exception& e) {
            ui->cdrResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
            ui->cdrResultsTextEdit->append(tr("❌ CDR analysis error: %1").arg(QString::fromStdString(e.what())));
        }
    } else {
        ui->cdrResultsTextEdit->setTextColor(QColor(u"#FF5252"_s)); // Red for errors
        ui->cdrResultsTextEdit->append(tr("❌ CDR Manager not initialized"));
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
 * @brief Handles the Configuration button click.
 * 
 * Currently a placeholder for configuration functionality.
 */
void DashboardWidget::onConfigButtonClicked() {
    qDebug() << "Config button clicked"; 
    // Placeholder for configuration dialog or page
    QMessageBox::information(this, u"Configuration"_s, u"Configuration options will be available in a future update."_s);
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
 * @brief Handles file selection and initiates the advanced scan process.
 * 
 * Opens a file dialog for the user to select a file, then processes
 * that file for advanced scanning and updates the UI accordingly.
 */
void DashboardWidget::onAdvancedScanSelectFile() {
    ui->dashboardTabWidget->setCurrentWidget(ui->basicScanTab); // Show progress in basic view initially
    ui->basicScanResultsTextEdit->clear();
    ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::SELECTING_FILE));
    
    // Use BasicScanner's file selection dialog for now
    if (m_basicScanner->selectFile()) {
        ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::FILE_SELECTED).arg(
            m_basicScanner->getSelectedFile().fileName()));
        ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::ADVANCED_SCAN));
        
        // First, perform a basic scan
        if (m_basicScanner->scanFile(m_basicScanner->getSelectedFile().filePath())) {
            // Basic scan started successfully, now proceed with VirusTotal analysis
            ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::VT_SUBMITTING));
            
            // Set the file in the VirusTotal manager and submit it
            if (m_virusTotalManager->scanFile(m_basicScanner->getSelectedFile().filePath())) {
                ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::VT_SUBMIT_STATUS).arg(
                    m_virusTotalManager->getSubmissionStatus()));
            } else {
                ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::ERROR_PREFIX).arg(
                    u"Failed to submit file to VirusTotal"_s));
            }
        } else {
            // Handle scan initiation error
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
 * @brief Processes and displays the scan results in the UI.
 * 
 * Clears the current display, shows the new results, and updates
 * the total scans counter.
 * 
 * @param results The scan results as a formatted string.
 */
void DashboardWidget::onBasicScanResultsReady(const QString& results) {
    ui->dashboardTabWidget->setCurrentWidget(ui->basicScanTab);
    ui->basicScanResultsTextEdit->clear();
    
    // Get current timestamp for the scan
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timestamp = currentTime.toString(u"yyyy-MM-dd hh:mm:ss"_s);
    
    // Add a header with timestamp
    ui->basicScanResultsTextEdit->setTextColor(QColor(u"#9E9E9E"_s));
    ui->basicScanResultsTextEdit->append(u"=== "_s + timestamp + u" ==="_s);
    ui->basicScanResultsTextEdit->append(u""_s);
    
    // Format and colorize the scan results
    if (results.contains(u"MALICIOUS"_s)) {
        // Malicious result formatting
        ui->basicScanResultsTextEdit->setTextColor(QColor(u"#FFFFFF"_s));
        ui->basicScanResultsTextEdit->append(u"⚠️ SCAN RESULT:"_s);
        ui->basicScanResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
        ui->basicScanResultsTextEdit->append(u"  MALICIOUS FILE DETECTED"_s);
        ui->basicScanResultsTextEdit->append(u""_s);
        
        // Add remaining results with proper formatting
        QStringList lines = results.split(u"\n"_s);
        for(const QString& line : lines) {
            if (line.contains(u"File:"_s)) {
                ui->basicScanResultsTextEdit->setTextColor(QColor(u"#2196F3"_s));
                ui->basicScanResultsTextEdit->append(line);
            } else if (line.contains(u"Hash:"_s)) {
                ui->basicScanResultsTextEdit->setTextColor(QColor(u"#E0E0E0"_s));
                ui->basicScanResultsTextEdit->append(line);
            } else if (line.contains(u"Threat:"_s)) {
                ui->basicScanResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
                ui->basicScanResultsTextEdit->append(line);
            } else {
                ui->basicScanResultsTextEdit->setTextColor(QColor(u"#E0E0E0"_s));
                ui->basicScanResultsTextEdit->append(line);
            }
        }
    } else {
        // Clean result formatting
        ui->basicScanResultsTextEdit->setTextColor(QColor(u"#FFFFFF"_s));
        ui->basicScanResultsTextEdit->append(u"✓ SCAN RESULT:"_s);
        ui->basicScanResultsTextEdit->setTextColor(QColor(u"#4CAF50"_s));
        ui->basicScanResultsTextEdit->append(u"  FILE IS CLEAN"_s);
        ui->basicScanResultsTextEdit->append(u""_s);
        
        // Add remaining results with proper formatting
        QStringList lines = results.split(u"\n"_s);
        for(const QString& line : lines) {
            if (line.contains(u"File:"_s)) {
                ui->basicScanResultsTextEdit->setTextColor(QColor(u"#2196F3"_s));
                ui->basicScanResultsTextEdit->append(line);
            } else if (line.contains(u"Hash:"_s)) {
                ui->basicScanResultsTextEdit->setTextColor(QColor(u"#E0E0E0"_s));
                ui->basicScanResultsTextEdit->append(line);
            } else if (!line.contains(u"CLEAN"_s)) {
                ui->basicScanResultsTextEdit->setTextColor(QColor(u"#E0E0E0"_s));
                ui->basicScanResultsTextEdit->append(line);
            }
        }
    }
}

/**
 * @brief Handles and displays scanner error messages.
 * 
 * Processes error codes, updates the UI with appropriate messages,
 * and shows a message box for critical errors.
 * 
 * @param errorCode The error code from the scanner.
 * @param errorMessage The descriptive error message.
 */
void DashboardWidget::onBasicScanError(ScannerErrorCode errorCode, const QString& errorMessage) {
    ui->dashboardTabWidget->setCurrentWidget(ui->basicScanTab);
    // Display error message in a more user-friendly way
    QString errorTitle;
    QString errorIcon;
    
    switch (errorCode) {
        case ScannerErrorCode::DatabaseNotConnected:
            errorTitle = QString::fromUtf8(DashboardText::DB_ERROR);
            errorIcon = u":/UI/Resources/Images/applogo.png"_s;
            break;
        case ScannerErrorCode::FileNotFound:
        case ScannerErrorCode::FileNotReadable:
            errorTitle = QString::fromUtf8(DashboardText::FILE_ERROR);
            errorIcon = u":/UI/Resources/Images/applogo.png"_s;
            break;
        case ScannerErrorCode::HashCalculationFailed:
            errorTitle = QString::fromUtf8(DashboardText::SCAN_ERROR);
            errorIcon = u":/UI/Resources/Images/applogo.png"_s;
            break;
        case ScannerErrorCode::MaliciousFileDetected:
            errorTitle = QString::fromUtf8(DashboardText::MALICIOUS_DETECTED);
            errorIcon = u":/UI/Resources/Images/applogo.png"_s;
            break;
        default:
            errorTitle = QString::fromUtf8(DashboardText::GENERIC_ERROR);
            errorIcon = u":/UI/Resources/Images/applogo.png"_s;
            break;
    }
    
    // Log the error
    qWarning() << "Basic Scan Error:" << static_cast<int>(errorCode) << "-" << errorMessage;
    
    // Add stylized error message to the scan results
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timestamp = currentTime.toString(u"yyyy-MM-dd hh:mm:ss"_s);
    
    // If the text area is empty, start with a timestamp
    if (ui->basicScanResultsTextEdit->toPlainText().isEmpty()) {
        ui->basicScanResultsTextEdit->setTextColor(QColor(u"#9E9E9E"_s));
        ui->basicScanResultsTextEdit->append(u"=== "_s + timestamp + u" ==="_s);
        ui->basicScanResultsTextEdit->append(u""_s);
        
        ui->basicScanResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
        ui->basicScanResultsTextEdit->append(u"⚠️ ERROR:"_s);
        ui->basicScanResultsTextEdit->setTextColor(QColor(u"#FFA726"_s));
        ui->basicScanResultsTextEdit->append(u"  "_s + errorMessage);
    } else if (!ui->basicScanResultsTextEdit->toPlainText().contains(errorMessage)) {
        // Only append if the error message isn't already there
        ui->basicScanResultsTextEdit->append(u""_s);
        ui->basicScanResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
        ui->basicScanResultsTextEdit->append(u"⚠️ ERROR:"_s);
        ui->basicScanResultsTextEdit->setTextColor(QColor(u"#FFA726"_s));
        ui->basicScanResultsTextEdit->append(u"  "_s + errorMessage);
    }
    
    // For critical errors, show a message box
    if (errorCode == ScannerErrorCode::DatabaseNotConnected || 
        errorCode == ScannerErrorCode::DatabaseQueryFailed ||
        errorCode == ScannerErrorCode::MaliciousFileDetected) {
        QMessageBox::critical(this, errorTitle, errorMessage);
    }
}

/**
 * @brief Handles VirusTotal results and parses the JSON response.
 * 
 * Logs the detailed engine results and updates the UI with summary statistics.
 * 
 * @param results The JSON response from VirusTotal.
 */
void DashboardWidget::handleVirusTotalResults(const QString& results) {
    qDebug() << "Raw VirusTotal Results in DashboardWidget (handleVirusTotalResults):" << (results.length() > 200 ? results.left(200) + QStringLiteral("...") : results);

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(results.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON Parse Error:" << error.errorString();
        QMessageBox::critical(this, QString::fromUtf8(DashboardText::VT_ERROR), QString(u"Failed to parse VirusTotal response: %1"_s).arg(error.errorString()));
        ui->dashboardTabWidget->setCurrentWidget(ui->basicScanTab);
        ui->basicScanResultsTextEdit->clear();
        ui->basicScanResultsTextEdit->setTextColor(Qt::red);
        ui->basicScanResultsTextEdit->append(QString(u"VirusTotal Response Parse Error: %1"_s).arg(error.errorString()));
        return;
    }

    if (!doc.isObject()) {
        qWarning() << "VirusTotal response is not a JSON object.";
        QMessageBox::critical(this, QString::fromUtf8(DashboardText::VT_ERROR), u"Unexpected VirusTotal response format."_s);
        ui->dashboardTabWidget->setCurrentWidget(ui->basicScanTab);
        ui->basicScanResultsTextEdit->clear();
        ui->basicScanResultsTextEdit->setTextColor(Qt::red);
        ui->basicScanResultsTextEdit->append(u"Unexpected VirusTotal response format."_s);
        return;
    }

    QJsonObject rootObject = doc.object();
    
    // Check for overall error from VirusTotal API
    if (rootObject.contains(u"error"_s)) {
        QJsonObject errorObj = rootObject.value(u"error"_s).toObject();
        QString errorMessage = errorObj.value(u"message"_s).toString(u"Unknown VirusTotal API error."_s);
        qWarning() << "VirusTotal API Error:" << errorMessage;
        QMessageBox::critical(this, QString::fromUtf8(DashboardText::VT_ERROR), errorMessage);
        ui->dashboardTabWidget->setCurrentWidget(ui->basicScanTab);
        ui->basicScanResultsTextEdit->clear();
        ui->basicScanResultsTextEdit->setTextColor(Qt::red);
        ui->basicScanResultsTextEdit->append(QString(u"VirusTotal API Error: %1"_s).arg(errorMessage));
        return;
    }

    if (!rootObject.contains(u"data"_s) || !rootObject.value(u"data"_s).isObject()) {
        qWarning() << "VirusTotal response does not contain 'data' object. Analysis might be pending.";
        ui->dashboardTabWidget->setCurrentWidget(ui->basicScanTab);
        if (!ui->basicScanResultsTextEdit->toPlainText().contains(QString::fromUtf8(DashboardText::VT_ANALYSIS_PENDING))) {
            ui->basicScanResultsTextEdit->setTextColor(Qt::yellow);
            ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::VT_ANALYSIS_PENDING));
        }
        return;
    }
    
    QJsonObject dataObject = rootObject.value(u"data"_s).toObject();

    if (!dataObject.contains(u"attributes"_s) || !dataObject.value(u"attributes"_s).isObject()) {
        qWarning() << "VirusTotal data object does not contain 'attributes'. Analysis might be pending.";
        ui->dashboardTabWidget->setCurrentWidget(ui->basicScanTab);
        if (!ui->basicScanResultsTextEdit->toPlainText().contains(QString::fromUtf8(DashboardText::VT_ANALYSIS_PENDING))) {
            ui->basicScanResultsTextEdit->setTextColor(Qt::yellow);
            ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::VT_ANALYSIS_PENDING));
        }
        return;
    }
    QJsonObject attributesObject = dataObject.value(u"attributes"_s).toObject();

    // Check if the analysis is completed by looking at the status or presence of results
    QString analysisStatus = attributesObject.value(u"status"_s).toString();
    bool hasResults = attributesObject.contains(u"results"_s) && attributesObject.value(u"results"_s).isObject();    if (!hasResults || analysisStatus == QLatin1String("queued")) {
        qWarning() << "VirusTotal analysis is not complete or results are not yet available. Status:" << analysisStatus;
        ui->dashboardTabWidget->setCurrentWidget(ui->basicScanTab);
        
        QString statusMessage;
        if (analysisStatus == QLatin1String("queued")) {
            statusMessage = tr("⏳ VirusTotal: Analiz kuyrukta bekliyor...");
        } else {
            statusMessage = QString::fromUtf8(DashboardText::VT_ANALYSIS_PENDING);
        }
        
        if (!ui->basicScanResultsTextEdit->toPlainText().contains(statusMessage)) {
             ui->basicScanResultsTextEdit->setTextColor(Qt::yellow);
             ui->basicScanResultsTextEdit->append(statusMessage);
        }
        return;
    }
    
    // At this point, we should have results
    ui->dashboardTabWidget->setCurrentWidget(ui->advancedScanTab);
    ui->advancedScanResultsTableWidget->clearContents();
    ui->advancedScanResultsTableWidget->setRowCount(0);
    
    // Add visual improvements to the table
    ui->advancedScanResultsTableWidget->setShowGrid(false);
    ui->advancedScanResultsTableWidget->setAlternatingRowColors(true);
    ui->advancedScanResultsTableWidget->verticalHeader()->setVisible(false);
    ui->advancedScanResultsTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->advancedScanResultsTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->advancedScanResultsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // Get file metadata for displaying in the header
    QString fileName = attributesObject.value(u"meaningful_name"_s).toString();
    QString fileSha256 = dataObject.value(u"id"_s).toString();
    
    QJsonObject analysisResults = attributesObject.value(u"results"_s).toObject();

    // Prepare table with improved headers
    ui->advancedScanResultsTableWidget->setColumnCount(3);
    QStringList headers = {u"Tarama Motoru"_s, u"Kategori"_s, u"Sonuç"_s}; // Localized headers for Turkish
    ui->advancedScanResultsTableWidget->setHorizontalHeaderLabels(headers);
    
    // Configure header appearance
    QFont headerFont = ui->advancedScanResultsTableWidget->horizontalHeader()->font();
    headerFont.setBold(true);
    headerFont.setPointSize(headerFont.pointSize() + 1);
    ui->advancedScanResultsTableWidget->horizontalHeader()->setFont(headerFont);

    // Add file info header to the results
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timestamp = currentTime.toString(u"yyyy-MM-dd hh:mm:ss"_s);
    
    // Count categories for summary and populate the table
    int maliciousCount = 0;
    int suspiciousCount = 0;
    int cleanCount = 0;
    int totalEngines = analysisResults.keys().count();
    
    // Process each engine's result and populate the table
    int row = 0;
    ui->advancedScanResultsTableWidget->setRowCount(totalEngines);
    
    for (auto it = analysisResults.constBegin(); it != analysisResults.constEnd(); ++it) {
        QString engineName = it.key();
        QJsonObject engineResult = it.value().toObject();
        
        QString category = engineResult.value(u"category"_s).toString();
        QString result = engineResult.value(u"result"_s).toString();
        
        // Set engine name in first column
        QTableWidgetItem* engineItem = new QTableWidgetItem(engineName);
        ui->advancedScanResultsTableWidget->setItem(row, 0, engineItem);
        
        // Set category in second column
        QTableWidgetItem* categoryItem = new QTableWidgetItem(category);
        ui->advancedScanResultsTableWidget->setItem(row, 1, categoryItem);
        
        // Set result in third column
        QTableWidgetItem* resultItem = new QTableWidgetItem(result.isEmpty() ? u"--"_s : result);
        ui->advancedScanResultsTableWidget->setItem(row, 2, resultItem);
        
        // Set colors based on category
        QColor rowColor;
        if (category == u"malicious"_s) {
            rowColor = QColor(u"#FFEBEE"_s); // Light red background
            maliciousCount++;
            engineItem->setForeground(QColor(u"#C62828"_s)); // Dark red text
            categoryItem->setForeground(QColor(u"#C62828"_s));
            resultItem->setForeground(QColor(u"#C62828"_s));
        } else if (category == u"suspicious"_s) {
            rowColor = QColor(u"#FFF8E1"_s); // Light yellow background
            suspiciousCount++;
            engineItem->setForeground(QColor(u"#F57F17"_s)); // Dark orange text
            categoryItem->setForeground(QColor(u"#F57F17"_s));
            resultItem->setForeground(QColor(u"#F57F17"_s));
        } else if (category == u"undetected"_s || category == u"clean"_s) {
            cleanCount++;
            engineItem->setForeground(QColor(u"#2E7D32"_s)); // Dark green text
            categoryItem->setForeground(QColor(u"#2E7D32"_s));
            resultItem->setForeground(QColor(u"#2E7D32"_s));
        }
        
        row++;
    }
    
    // Display a summary message at the top
    QString threatSummary = QString(u"Dosya analizi tamamlandı: %1 motordan %2 zararlı, %3 şüpheli, %4 temiz tespit."_s)
                            .arg(totalEngines)
                            .arg(maliciousCount)
                            .arg(suspiciousCount)
                            .arg(cleanCount);
    
    // Update basic scan results text with summary as well
    ui->basicScanResultsTextEdit->clear();
    ui->basicScanResultsTextEdit->setTextColor(QColor(u"#9E9E9E"_s));
    ui->basicScanResultsTextEdit->append(u"=== "_s + timestamp + u" ==="_s);
    ui->basicScanResultsTextEdit->append(u""_s);
    
    if (maliciousCount > 0) {
        ui->basicScanResultsTextEdit->setTextColor(QColor(u"#FF5252"_s));
        ui->basicScanResultsTextEdit->append(u"⚠️ VIRUS TOTAL ANALİZİ: ZARARLI"_s);
    } else if (suspiciousCount > 0) {
        ui->basicScanResultsTextEdit->setTextColor(QColor(u"#FFA726"_s));
        ui->basicScanResultsTextEdit->append(u"⚠️ VIRUS TOTAL ANALİZİ: ŞÜPHELİ"_s);
    } else {
        ui->basicScanResultsTextEdit->setTextColor(QColor(u"#4CAF50"_s));
        ui->basicScanResultsTextEdit->append(u"✓ VIRUS TOTAL ANALİZİ: TEMİZ"_s);
    }
    
    ui->basicScanResultsTextEdit->append(u""_s);
    ui->basicScanResultsTextEdit->setTextColor(QColor(u"#FFFFFF"_s));
    ui->basicScanResultsTextEdit->append(threatSummary);
    
    // Optimize table display
    ui->advancedScanResultsTableWidget->resizeColumnsToContents();
    ui->advancedScanResultsTableWidget->horizontalHeader()->setStretchLastSection(true);
}

/**
 * @brief Appends a log message to the network communication text edit.
 * @param logMessage The message to append.
 */
void DashboardWidget::appendNetworkLog(const QString& logMessage) {
    if (ui && ui->networkCommunicationTextEdit) {
        // Get current timestamp
        QDateTime currentTime = QDateTime::currentDateTime();
        QString timestamp = currentTime.toString(u"hh:mm:ss.zzz"_s);
        
        // Set color based on message type
        if (logMessage.contains(u"ERROR"_s, Qt::CaseInsensitive) || 
            logMessage.contains(u"FAILED"_s, Qt::CaseInsensitive)) {
            ui->networkCommunicationTextEdit->setTextColor(QColor(u"#FF5252"_s));
        } else if (logMessage.contains(u"WARNING"_s, Qt::CaseInsensitive) ||
                   logMessage.contains(u"polling"_s, Qt::CaseInsensitive) ||
                   logMessage.contains(u"attempt"_s, Qt::CaseInsensitive)) {
            ui->networkCommunicationTextEdit->setTextColor(QColor(u"#FFA726"_s));
        } else if (logMessage.contains(u"INFO"_s, Qt::CaseInsensitive)) {
            ui->networkCommunicationTextEdit->setTextColor(QColor(u"#2196F3"_s));
        } else if (logMessage.contains(u"CONNECTION"_s, Qt::CaseInsensitive) || 
                  logMessage.contains(u"CONNECTED"_s, Qt::CaseInsensitive)) {
            ui->networkCommunicationTextEdit->setTextColor(QColor(u"#4CAF50"_s));
        } else {
            ui->networkCommunicationTextEdit->setTextColor(QColor(u"#E0E0E0"_s));
        }
        
        // Format and append message with timestamp
        ui->networkCommunicationTextEdit->append(u"["_s + timestamp + u"] "_s + logMessage);
    }
}
