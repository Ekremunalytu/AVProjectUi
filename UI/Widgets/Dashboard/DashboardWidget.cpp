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
    m_networkMonitor(new NetworkMonitor(this)) // Instantiate NetworkMonitor
{
    ui->setupUi(this);
    
    // Connect click events for all buttons
    connect(ui->basicScanButton_dashboard, &QPushButton::clicked, this, &DashboardWidget::onBasicScanSelectFile);
    connect(ui->advancedScanButton_dashboard, &QPushButton::clicked, this, &DashboardWidget::onAdvancedScanClicked);
    connect(ui->cdrScanButton_dashboard, &QPushButton::clicked, this, &DashboardWidget::onCdrScanClicked);
    connect(ui->sandboxScanButton_dashboard, &QPushButton::clicked, this, &DashboardWidget::onSandboxScanClicked);
    
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
 * @brief Handles the Advanced Scan button click.
 * 
 * Opens a file dialog for the user to select a file for advanced scanning.
 */
void DashboardWidget::onAdvancedScanClicked() {
    onAdvancedScanSelectFile();
}

/**
 * @brief Handles the CDR Scan button click.
 * 
 * Currently displays a debug message and updates the UI.
 */
void DashboardWidget::onCdrScanClicked() {
    qDebug() << DashboardText::CDR_SCAN;
    ui->scanResultsStackedWidget->setCurrentWidget(ui->basicScanPage);
    ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::CDR_SCAN));
}

/**
 * @brief Handles the Sandbox Scan button click.
 * 
 * Currently displays a debug message and updates the UI.
 */
void DashboardWidget::onSandboxScanClicked() {
    qDebug() << DashboardText::SANDBOX_SCAN;
    ui->scanResultsStackedWidget->setCurrentWidget(ui->basicScanPage);
    ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::SANDBOX_SCAN));
}

/**
 * @brief Handles file selection and initiates the basic scan process.
 * 
 * Opens a file dialog for the user to select a file, then initiates
 * the scan of that file and updates the UI accordingly.
 */
void DashboardWidget::onBasicScanSelectFile() {
    ui->scanResultsStackedWidget->setCurrentWidget(ui->basicScanPage);
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
            if (m_basicScanner->getLastError() != ScannerErrorCode::NoError) {
                ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::ERROR_PREFIX).arg(
                    m_basicScanner->getLastErrorMessage()));
            }
        }
    } else {
        if (m_basicScanner->getLastError() == ScannerErrorCode::NoError) {
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
    ui->scanResultsStackedWidget->setCurrentWidget(ui->basicScanPage); // Show progress in basic view initially
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
            if (m_basicScanner->getLastError() != ScannerErrorCode::NoError) {
                ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::ERROR_PREFIX).arg(
                    m_basicScanner->getLastErrorMessage()));
            }
        }
    } else {
        if (m_basicScanner->getLastError() == ScannerErrorCode::NoError) {
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
    ui->scanResultsStackedWidget->setCurrentWidget(ui->basicScanPage);
    ui->basicScanResultsTextEdit->clear();
    
    // Set text color based on scan result
    if (results.contains(u"MALICIOUS"_s)) {
        ui->basicScanResultsTextEdit->setTextColor(Qt::red);
    } else {
        ui->basicScanResultsTextEdit->setTextColor(Qt::green);
    }
    
    ui->basicScanResultsTextEdit->append(results);
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
    ui->scanResultsStackedWidget->setCurrentWidget(ui->basicScanPage);
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
    
    // Add error message to the scan results text edit
    if (ui->basicScanResultsTextEdit->toPlainText().isEmpty()) {
        ui->basicScanResultsTextEdit->setText(QString::fromUtf8(DashboardText::ERROR_PREFIX).arg(errorMessage));
    } else if (!ui->basicScanResultsTextEdit->toPlainText().contains(errorMessage)) {
        // Only append if the error message isn't already there
        ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::ERROR_PREFIX).arg(errorMessage));
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
        ui->scanResultsStackedWidget->setCurrentWidget(ui->basicScanPage);
        ui->basicScanResultsTextEdit->clear();
        ui->basicScanResultsTextEdit->setTextColor(Qt::red);
        ui->basicScanResultsTextEdit->append(QString(u"VirusTotal Response Parse Error: %1"_s).arg(error.errorString()));
        return;
    }

    if (!doc.isObject()) {
        qWarning() << "VirusTotal response is not a JSON object.";
        QMessageBox::critical(this, QString::fromUtf8(DashboardText::VT_ERROR), u"Unexpected VirusTotal response format."_s);
        ui->scanResultsStackedWidget->setCurrentWidget(ui->basicScanPage);
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
        ui->scanResultsStackedWidget->setCurrentWidget(ui->basicScanPage);
        ui->basicScanResultsTextEdit->clear();
        ui->basicScanResultsTextEdit->setTextColor(Qt::red);
        ui->basicScanResultsTextEdit->append(QString(u"VirusTotal API Error: %1"_s).arg(errorMessage));
        return;
    }

    if (!rootObject.contains(u"data"_s) || !rootObject.value(u"data"_s).isObject()) {
        qWarning() << "VirusTotal response does not contain 'data' object. Analysis might be pending.";
        ui->scanResultsStackedWidget->setCurrentWidget(ui->basicScanPage);
        if (!ui->basicScanResultsTextEdit->toPlainText().contains(QString::fromUtf8(DashboardText::VT_ANALYSIS_PENDING))) {
            ui->basicScanResultsTextEdit->setTextColor(Qt::yellow);
            ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::VT_ANALYSIS_PENDING));
        }
        return;
    }
    
    QJsonObject dataObject = rootObject.value(u"data"_s).toObject();

    if (!dataObject.contains(u"attributes"_s) || !dataObject.value(u"attributes"_s).isObject()) {
        qWarning() << "VirusTotal data object does not contain 'attributes'. Analysis might be pending.";
        ui->scanResultsStackedWidget->setCurrentWidget(ui->basicScanPage);
        if (!ui->basicScanResultsTextEdit->toPlainText().contains(QString::fromUtf8(DashboardText::VT_ANALYSIS_PENDING))) {
            ui->basicScanResultsTextEdit->setTextColor(Qt::yellow);
            ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::VT_ANALYSIS_PENDING));
        }
        return;
    }
    QJsonObject attributesObject = dataObject.value(u"attributes"_s).toObject();

    // Check if the analysis is completed by looking at the status or presence of results
    QString analysisStatus = attributesObject.value(u"status"_s).toString();
    bool hasResults = attributesObject.contains(u"results"_s) && attributesObject.value(u"results"_s).isObject();

    if (!hasResults || analysisStatus == QLatin1String("queued")) {
        qWarning() << "VirusTotal analysis is not complete or results are not yet available. Status:" << analysisStatus;
        ui->scanResultsStackedWidget->setCurrentWidget(ui->basicScanPage);
        if (!ui->basicScanResultsTextEdit->toPlainText().contains(QString::fromUtf8(DashboardText::VT_ANALYSIS_PENDING))) {
             ui->basicScanResultsTextEdit->setTextColor(Qt::yellow);
             ui->basicScanResultsTextEdit->append(QString::fromUtf8(DashboardText::VT_ANALYSIS_PENDING));
        }
        return;
    }
    
    // At this point, we should have results
    ui->scanResultsStackedWidget->setCurrentWidget(ui->advancedScanPage);
    ui->advancedScanResultsTableWidget->clearContents();
    ui->advancedScanResultsTableWidget->setRowCount(0); 

    QJsonObject analysisResults = attributesObject.value(u"results"_s).toObject();

    // Prepare table
    ui->advancedScanResultsTableWidget->setColumnCount(3);
    QStringList headers = {u"Engine"_s, u"Category"_s, u"Result"_s};
    ui->advancedScanResultsTableWidget->setHorizontalHeaderLabels(headers);

    int row = 0;
    for (const QString& engineName : analysisResults.keys()) {
        QJsonObject engineResult = analysisResults.value(engineName).toObject();
        QString category = engineResult.value(u"category"_s).toString();
        QString result = engineResult.value(u"result"_s).toString(u"N/A"_s); // Default to N/A if no result string

        QTableWidgetItem* engineItem = new QTableWidgetItem(engineName);
        QTableWidgetItem* categoryItem = new QTableWidgetItem(category);
        QTableWidgetItem* resultItem = new QTableWidgetItem(result);

        // Define text and background brushes
        QBrush backgroundBrush(ui->advancedScanResultsTableWidget->palette().base()); // Default to theme's base color
        QBrush foregroundBrush(ui->advancedScanResultsTableWidget->palette().text()); // Default to theme's text color

        if (category.compare(u"malicious"_s, Qt::CaseInsensitive) == 0) {
            backgroundBrush = QBrush(QColor(192, 57, 43)); // Pomegranate Red
            foregroundBrush = QBrush(Qt::white);
        } else if (category.compare(u"suspicious"_s, Qt::CaseInsensitive) == 0) {
            backgroundBrush = QBrush(QColor(211, 84, 0)); // Pumpkin Orange
            foregroundBrush = QBrush(Qt::white);
        } else if (category.compare(u"undetected"_s, Qt::CaseInsensitive) == 0) {
            backgroundBrush = QBrush(QColor(40, 116, 50)); // Darker, desaturated green
            foregroundBrush = QBrush(Qt::white);
        } else if (category.compare(u"harmless"_s, Qt::CaseInsensitive) == 0) {
            backgroundBrush = QBrush(QColor(36, 113, 163)); // Darker, desaturated blue
            foregroundBrush = QBrush(Qt::white);
        } else if (category.compare(u"type-unsupported"_s, Qt::CaseInsensitive) == 0) {
            backgroundBrush = QBrush(QColor(93, 109, 126)); // Dark Slate Gray
            foregroundBrush = QBrush(Qt::white);
        } else if (category.compare(u"timeout"_s, Qt::CaseInsensitive) == 0) {
            backgroundBrush = QBrush(QColor(183, 149, 11)); // Muted Dark Gold
            foregroundBrush = QBrush(Qt::white);
        }
        // Other categories will use the default theme colors

        ui->advancedScanResultsTableWidget->insertRow(row);
        ui->advancedScanResultsTableWidget->setItem(row, 0, engineItem);
        ui->advancedScanResultsTableWidget->setItem(row, 1, categoryItem);
        ui->advancedScanResultsTableWidget->setItem(row, 2, resultItem);

        for (int col = 0; col < 3; ++col) {
            if(QTableWidgetItem* item = ui->advancedScanResultsTableWidget->item(row, col)) {
                item->setBackground(backgroundBrush);
                item->setForeground(foregroundBrush);
            }
        }
        row++;
    }
    ui->advancedScanResultsTableWidget->resizeColumnsToContents();

    // Make headers bold
    QFont font = ui->advancedScanResultsTableWidget->horizontalHeader()->font();
    font.setBold(true);
    ui->advancedScanResultsTableWidget->horizontalHeader()->setFont(font);
    
    // Display overall stats in the basicScanResultsTextEdit for a quick summary, or add new labels for this.
    QJsonObject stats = attributesObject.value(u"stats"_s).toObject();
    QString summary = QString(u"--- VirusTotal Full Report ---\nOverall Scan Status: completed\n--- Scan Statistics ---\n"_s) +
                      QString(u"Malicious: %1\n"_s).arg(stats.value(u"malicious"_s).toInt(0)) +
                      QString(u"Suspicious: %1\n"_s).arg(stats.value(u"suspicious"_s).toInt(0)) +
                      QString(u"Undetected: %1\n"_s).arg(stats.value(u"undetected"_s).toInt(0)) +
                      QString(u"Harmless: %1\n"_s).arg(stats.value(u"harmless"_s).toInt(0)) +
                      QString(u"Timeout: %1\n"_s).arg(stats.value(u"timeout"_s).toInt(0)) +
                      QString(u"Type Unsupported: %1\n"_s).arg(stats.value(u"type-unsupported"_s).toInt(0)) +
                      QString(u"--- Detailed Engine Results shown in table ---"_s);

    qDebug() << "VirusTotal results parsed and table populated.";
}

/**
 * @brief Appends a log message to the network communication text edit.
 * @param logMessage The message to append.
 */
void DashboardWidget::appendNetworkLog(const QString& logMessage) {
    if (ui && ui->networkCommunicationTextEdit) {
        ui->networkCommunicationTextEdit->append(logMessage);
    }
}
