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
    m_virusTotalManager(std::make_unique<VirusTotalManager>()) 
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
}

/**
 * @brief Destroys the DashboardWidget and releases resources.
 */
DashboardWidget::~DashboardWidget() {
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
    ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::CDR_SCAN));
}

/**
 * @brief Handles the Sandbox Scan button click.
 * 
 * Currently displays a debug message and updates the UI.
 */
void DashboardWidget::onSandboxScanClicked() {
    qDebug() << DashboardText::SANDBOX_SCAN;
    ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::SANDBOX_SCAN));
}

/**
 * @brief Handles file selection and initiates the basic scan process.
 * 
 * Opens a file dialog for the user to select a file, then initiates
 * the scan of that file and updates the UI accordingly.
 */
void DashboardWidget::onBasicScanSelectFile() {
    ui->scanResultsTextEdit_dashboard->clear();
    ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::SELECTING_FILE));
    
    if (m_basicScanner->selectFile()) {
        ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::FILE_SELECTED).arg(
            m_basicScanner->getSelectedFile().fileName()));
        ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::SCANNING_FILE));
        
        // Initiate scan with selected file
        if (!m_basicScanner->scanFile(m_basicScanner->getSelectedFile().filePath())) {
            // Handle scan initiation error - already handled by error signal, 
            // but we can add additional UI updates if needed
            if (m_basicScanner->getLastError() != ScannerErrorCode::NoError) {
                ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::ERROR_PREFIX).arg(
                    m_basicScanner->getLastErrorMessage()));
            }
        }
    } else {
        if (m_basicScanner->getLastError() == ScannerErrorCode::NoError) {
            // User canceled file selection, not an error
            ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::SELECTION_CANCELED));
        } else {
            // Handle file selection error
            ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::ERROR_PREFIX).arg(
                m_basicScanner->getLastErrorMessage()));
        }
    }
}

// This method has been removed as part of the UI redesign to simplify the scanning process

/**
 * @brief Handles file selection and initiates the advanced scan process.
 * 
 * Opens a file dialog for the user to select a file, then processes
 * that file for advanced scanning and updates the UI accordingly.
 */
void DashboardWidget::onAdvancedScanSelectFile() {
    ui->scanResultsTextEdit_dashboard->clear();
    ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::SELECTING_FILE));
    
    // Use BasicScanner's file selection dialog for now
    if (m_basicScanner->selectFile()) {
        ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::FILE_SELECTED).arg(
            m_basicScanner->getSelectedFile().fileName()));
        ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::ADVANCED_SCAN));
        
        // First, perform a basic scan
        if (m_basicScanner->scanFile(m_basicScanner->getSelectedFile().filePath())) {
            // Basic scan started successfully, now proceed with VirusTotal analysis
            ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::VT_SUBMITTING));
            
            // Set the file in the VirusTotal manager and submit it
            if (m_virusTotalManager->scanFile(m_basicScanner->getSelectedFile().filePath())) {
                ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::VT_SUBMIT_STATUS).arg(
                    m_virusTotalManager->getSubmissionStatus()));
            } else {
                ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::ERROR_PREFIX).arg(
                    "Failed to submit file to VirusTotal"));
            }
        } else {
            // Handle scan initiation error
            if (m_basicScanner->getLastError() != ScannerErrorCode::NoError) {
                ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::ERROR_PREFIX).arg(
                    m_basicScanner->getLastErrorMessage()));
            }
        }
        
        // Update total scans count
        int currentCount = ui->totalScansValue_dashboard->text().toInt();
        ui->totalScansValue_dashboard->setText(QString::number(currentCount + 1));
    } else {
        if (m_basicScanner->getLastError() == ScannerErrorCode::NoError) {
            // User canceled file selection, not an error
            ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::SELECTION_CANCELED));
        } else {
            // Handle file selection error
            ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::ERROR_PREFIX).arg(
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
    ui->scanResultsTextEdit_dashboard->clear();
    
    // Set text color based on scan result
    if (results.contains(u"MALICIOUS"_s)) {
        ui->scanResultsTextEdit_dashboard->setTextColor(Qt::red);
    } else {
        ui->scanResultsTextEdit_dashboard->setTextColor(Qt::green);
    }
    
    ui->scanResultsTextEdit_dashboard->append(results);
    
    // Update total scans count
    int currentCount = ui->totalScansValue_dashboard->text().toInt();
    ui->totalScansValue_dashboard->setText(QString::number(currentCount + 1));
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
    // Display error message in a more user-friendly way
    QString errorTitle;
    QString errorIcon;
    
    switch (errorCode) {
        case ScannerErrorCode::DatabaseNotConnected:
            errorTitle = tr(DashboardText::DB_ERROR);
            errorIcon = u":/UI/Resources/Images/applogo.png"_s;
            break;
        case ScannerErrorCode::FileNotFound:
        case ScannerErrorCode::FileNotReadable:
            errorTitle = tr(DashboardText::FILE_ERROR);
            errorIcon = u":/UI/Resources/Images/applogo.png"_s;
            break;
        case ScannerErrorCode::HashCalculationFailed:
            errorTitle = tr(DashboardText::SCAN_ERROR);
            errorIcon = u":/UI/Resources/Images/applogo.png"_s;
            break;
        case ScannerErrorCode::MaliciousFileDetected:
            errorTitle = tr(DashboardText::MALICIOUS_DETECTED);
            errorIcon = u":/UI/Resources/Images/applogo.png"_s;
            break;
        default:
            errorTitle = tr(DashboardText::GENERIC_ERROR);
            errorIcon = u":/UI/Resources/Images/applogo.png"_s;
            break;
    }
    
    // Log the error
    qWarning() << "Basic Scan Error:" << static_cast<int>(errorCode) << "-" << errorMessage;
    
    // Add error message to the scan results text edit
    if (ui->scanResultsTextEdit_dashboard->toPlainText().isEmpty()) {
        ui->scanResultsTextEdit_dashboard->setText(tr(DashboardText::ERROR_PREFIX).arg(errorMessage));
    } else if (!ui->scanResultsTextEdit_dashboard->toPlainText().contains(errorMessage)) {
        // Only append if the error message isn't already there
        ui->scanResultsTextEdit_dashboard->append(tr(DashboardText::ERROR_PREFIX).arg(errorMessage));
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
        qWarning() << "Failed to parse VirusTotal JSON:" << error.errorString();
        ui->scanResultsTextEdit_dashboard->setTextColor(Qt::red);
        ui->scanResultsTextEdit_dashboard->append(tr("Error parsing VirusTotal results: %1").arg(error.errorString()));
        ui->scanResultsTextEdit_dashboard->setTextColor(Qt::black);
        return;
    }

    if (!doc.isObject()) {
        qWarning() << "VirusTotal JSON is not an object.";
        ui->scanResultsTextEdit_dashboard->setTextColor(Qt::red);
        ui->scanResultsTextEdit_dashboard->append(tr("Error: VirusTotal results are not in the expected object format."));
        ui->scanResultsTextEdit_dashboard->setTextColor(Qt::black);
        return;
    }

    QJsonObject rootObj = doc.object();

    if (rootObj.contains(QStringLiteral("error"))) {
        QJsonObject errorObj = rootObj[QStringLiteral("error")].toObject();
        QString errorMessage = errorObj.value(QStringLiteral("message")).toString(QStringLiteral("Unknown API error"));
        QString errorCode = errorObj.value(QStringLiteral("code")).toString(QStringLiteral("N/A"));
        qWarning() << "VirusTotal API Error:" << errorCode << "-" << errorMessage;
        ui->scanResultsTextEdit_dashboard->setTextColor(Qt::red);
        ui->scanResultsTextEdit_dashboard->append(tr("VirusTotal API Error (%1): %2").arg(errorCode, errorMessage));
        ui->scanResultsTextEdit_dashboard->setTextColor(Qt::black);
        return;
    }

    if (!rootObj.contains(QStringLiteral("data")) || !rootObj[QStringLiteral("data")].isObject()) {
        qWarning() << "VirusTotal JSON does not contain 'data' object or it's not an object.";
        ui->scanResultsTextEdit_dashboard->setTextColor(Qt::red);
        ui->scanResultsTextEdit_dashboard->append(tr("Error: Missing or invalid 'data' field in VirusTotal results."));
        ui->scanResultsTextEdit_dashboard->setTextColor(Qt::black);
        return;
    }
    QJsonObject dataObj = rootObj[QStringLiteral("data")].toObject();

    if (!dataObj.contains(QStringLiteral("attributes")) || !dataObj[QStringLiteral("attributes")].isObject()) {
        qWarning() << "VirusTotal JSON does not contain 'attributes' object or it's not an object.";
        ui->scanResultsTextEdit_dashboard->setTextColor(Qt::red);
        ui->scanResultsTextEdit_dashboard->append(tr("Error: Missing or invalid 'attributes' field in VirusTotal results."));
        ui->scanResultsTextEdit_dashboard->setTextColor(Qt::black);
        return;
    }
    QJsonObject attributesObj = dataObj[QStringLiteral("attributes")].toObject();

    QString analysisStatus = attributesObj.value(QStringLiteral("status")).toString(QStringLiteral("N/A"));
    qDebug() << "Analysis Status:" << analysisStatus;
    ui->scanResultsTextEdit_dashboard->append(tr("--- VirusTotal Full Report ---"));
    ui->scanResultsTextEdit_dashboard->append(tr("Overall Scan Status: %1").arg(analysisStatus));

    if (analysisStatus == QLatin1String("queued")) {
        ui->scanResultsTextEdit_dashboard->append(tr("Analysis is queued. Please wait for completion."));
        return;
    }
    if (analysisStatus != QLatin1String("completed")) {
         ui->scanResultsTextEdit_dashboard->append(tr("Analysis status is '%1'. Detailed results may not be available yet.").arg(analysisStatus));
    }

    if (attributesObj.contains(QStringLiteral("stats")) && attributesObj[QStringLiteral("stats")].isObject()) {
        QJsonObject statsObj = attributesObj[QStringLiteral("stats")].toObject();
        int malicious = statsObj.value(QStringLiteral("malicious")).toInt(0);
        int suspicious = statsObj.value(QStringLiteral("suspicious")).toInt(0);
        int undetected = statsObj.value(QStringLiteral("undetected")).toInt(0);
        int harmless = statsObj.value(QStringLiteral("harmless")).toInt(0);
        int timeout = statsObj.value(QStringLiteral("timeout")).toInt(0);
        int confirmed_timeout = statsObj.value(QStringLiteral("confirmed-timeout")).toInt(0);
        int failure = statsObj.value(QStringLiteral("failure")).toInt(0);
        int type_unsupported = statsObj.value(QStringLiteral("type-unsupported")).toInt(0);

        qDebug() << "Scan Summary: Malicious:" << malicious << ", Suspicious:" << suspicious
                 << ", Undetected:" << undetected << ", Harmless:" << harmless << ", Timeout:" << timeout;
        
        ui->scanResultsTextEdit_dashboard->append(tr("\n--- Scan Statistics ---"));
        if (malicious > 0 || suspicious > 0) {
            ui->scanResultsTextEdit_dashboard->setTextColor(Qt::red);
        } else if (undetected > 0 || harmless > 0) {
            ui->scanResultsTextEdit_dashboard->setTextColor(Qt::darkGreen);
        } else {
            ui->scanResultsTextEdit_dashboard->setTextColor(Qt::black);
        }
        ui->scanResultsTextEdit_dashboard->append(tr("Malicious: %1").arg(malicious));
        ui->scanResultsTextEdit_dashboard->append(tr("Suspicious: %1").arg(suspicious));
        ui->scanResultsTextEdit_dashboard->setTextColor(Qt::black);
        ui->scanResultsTextEdit_dashboard->append(tr("Undetected: %1").arg(undetected));
        ui->scanResultsTextEdit_dashboard->append(tr("Harmless: %1").arg(harmless));
        ui->scanResultsTextEdit_dashboard->append(tr("Timeout: %1").arg(timeout));
        if (confirmed_timeout > 0) ui->scanResultsTextEdit_dashboard->append(tr("Confirmed Timeout: %1").arg(confirmed_timeout));
        if (failure > 0) ui->scanResultsTextEdit_dashboard->append(tr("Failure: %1").arg(failure));
        if (type_unsupported > 0) ui->scanResultsTextEdit_dashboard->append(tr("Type Unsupported: %1").arg(type_unsupported));
    } else {
        qDebug() << "No summary statistics (stats object) found in VirusTotal results.";
        ui->scanResultsTextEdit_dashboard->append(tr("No summary statistics available."));
    }

    if (attributesObj.contains(QStringLiteral("results")) && attributesObj[QStringLiteral("results")].isObject()) {
        QJsonObject engineResultsObj = attributesObj[QStringLiteral("results")].toObject();
        qDebug() << "Detailed Scan Engine Results (" << engineResultsObj.size() << " engines):";
        ui->scanResultsTextEdit_dashboard->append(tr("\n--- Detailed Engine Results ---"));
        
        QStringList detections;
        int engineCount = 0;
        const int maxEnginesToShow = 20;

        for (auto it = engineResultsObj.constBegin(); it != engineResultsObj.constEnd(); ++it) {
            engineCount++;
            QJsonObject engineDetail = it.value().toObject();
            QString engineName = engineDetail.value(QStringLiteral("engine_name")).toString(QStringLiteral("Unknown Engine"));
            QString category = engineDetail.value(QStringLiteral("category")).toString(QStringLiteral("N/A"));
            QString method = engineDetail.value(QStringLiteral("method")).toString(QStringLiteral("N/A"));
            QString engine_version = engineDetail.value(QStringLiteral("engine_version")).toString(QStringLiteral("N/A"));
            QString engine_update = engineDetail.value(QStringLiteral("engine_update")).toString(QStringLiteral("N/A"));
            QString malwareName = engineDetail.value(QStringLiteral("result")).toString();

            QString line = tr("Engine: %1 (v%2, updated: %3) | Category: %4 | Method: %5")
                               .arg(engineName, engine_version, engine_update, category, method);

            if (!malwareName.isNull() && !malwareName.isEmpty()) {
                line.append(tr(" | Detection: %1").arg(malwareName));
                detections.append(tr("%1: %2 (%3)").arg(engineName, malwareName, category));
            }
            
            if (engineCount <= maxEnginesToShow) {
                 if (category == QLatin1String("malicious") || category == QLatin1String("suspicious")) {
                    ui->scanResultsTextEdit_dashboard->setTextColor(Qt::red);
                } else if (category == QLatin1String("undetected") || category == QLatin1String("harmless")) {
                    ui->scanResultsTextEdit_dashboard->setTextColor(Qt::darkGreen);
                } else {
                    ui->scanResultsTextEdit_dashboard->setTextColor(Qt::gray);
                }
                ui->scanResultsTextEdit_dashboard->append(line);
                ui->scanResultsTextEdit_dashboard->setTextColor(Qt::black);
            }

            qDebug() << "  Engine:" << engineName << ", Category:" << category << ", Result:" << malwareName << ", Method:" << method;
        }
        if (engineCount > maxEnginesToShow) {
            ui->scanResultsTextEdit_dashboard->append(tr("... and %1 more engine results not shown.").arg(engineCount - maxEnginesToShow));
        }

        if (!detections.isEmpty()) {
            ui->scanResultsTextEdit_dashboard->append(tr("\n--- Summary of Detections ---"));
            ui->scanResultsTextEdit_dashboard->setTextColor(Qt::red);
            for (const QString& detection : detections) {
                ui->scanResultsTextEdit_dashboard->append(detection);
            }
            ui->scanResultsTextEdit_dashboard->setTextColor(Qt::black);
        }
    } else {
        qDebug() << "No detailed scan engine results (results object) found.";
        ui->scanResultsTextEdit_dashboard->append(tr("No detailed scan engine results available."));
    }
    ui->scanResultsTextEdit_dashboard->append(tr("--- End of VirusTotal Report ---"));
}
