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
#include <QFileDialog> // Reverted to <QFileDialog>

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
    m_networkMonitor(new NetworkMonitor(this)),
    m_networkMonitorWidget(nullptr)
{
    ui->setupUi(this);

    // Create and setup the network monitor widget
    m_networkMonitorWidget = new NetworkMonitorWidget(this); // Parent'i this olarak ayarladım

    // Add the network monitor widget to the networkPage in QStackedWidget
    ui->networkPage->setLayout(new QVBoxLayout()); // networkPage için bir layout oluştur
    ui->networkPage->layout()->addWidget(m_networkMonitorWidget);

    // Yan menü ve içerik alanı bağlantıları
    connect(ui->sideMenuWidget, &QListWidget::currentRowChanged, ui->contentStackedWidget, &QStackedWidget::setCurrentIndex);

    // Başlangıçta Genel Bakış sekmesini göster
    ui->sideMenuWidget->setCurrentRow(0);


    // Load and apply custom styles (Mevcut stil kodları buraya gelebilir, şimdilik çıkarıldı)
    // ... Stil kodları ...

    // Connect scan buttons in Malware Analysis tabs
    // Ensure these buttons (basicScanFileButton, advancedScanFileButton, cdrScanButton, sandboxScanButton) exist in your .ui file
    if (ui->basicScanFileButton) {
        connect(ui->basicScanFileButton, &QPushButton::clicked, this, &DashboardWidget::onBasicScanSelectFile);
    }
    if (ui->advancedScanFileButton) {
        connect(ui->advancedScanFileButton, &QPushButton::clicked, this, &DashboardWidget::onAdvancedScanSelectFile);
    }
    if (ui->cdrScanButton) { // Assuming this button exists in the UI for CDR
        connect(ui->cdrScanButton, &QPushButton::clicked, this, &DashboardWidget::onCdrScanButtonClicked);
    }
    if (ui->sandboxScanButton) { // Assuming this button exists in the UI for Sandbox
        connect(ui->sandboxScanButton, &QPushButton::clicked, this, &DashboardWidget::onSandboxScanButtonClicked);
    }
    
    // connect(ui->configButton, &QPushButton::clicked, this, &DashboardWidget::onConfigButtonClicked); // Keep if config button exists
    // connect(ui->refreshButton, &QPushButton::clicked, this, &DashboardWidget::onRefreshButtonClicked); // Keep if refresh button exists


    // Basic scan connections
    connect(m_basicScanner.get(), &BasicScanner::scanResultsReady, this, &DashboardWidget::onBasicScanResultsReady);
    connect(m_basicScanner.get(), &BasicScanner::scanError, this, &DashboardWidget::onBasicScanError);

    // VirusTotal connections
    connect(m_virusTotalManager.get(), &VirusTotalManager::analysisResultsReady, this, &DashboardWidget::handleVirusTotalResults);

    // Network Monitor connections
    connect(m_networkMonitor, &NetworkMonitor::newLogMessage, this, &DashboardWidget::appendNetworkLog);
    
    // Start the network monitor (NetworkMonitorWidget içinden yönetilecek)
    // m_networkMonitor->startMonitoring(); // Bu satır kaldırıldı, NetworkMonitorWidget kendi içinde başlatacak

    // NetworkMonitorWidget'ın start/stop sinyallerini bağlayalım (Eğer NetworkMonitorWidget bu sinyalleri yayıyorsa)
    // connect(m_networkMonitorWidget, &NetworkMonitorWidget::monitoringStarted, this, &DashboardWidget::updateNetworkMonitoringUI);
    // connect(m_networkMonitorWidget, &NetworkMonitorWidget::monitoringStopped, this, &DashboardWidget::updateNetworkMonitoringUI);

    // Örnek: Yan menüdeki "Network" seçildiğinde NetworkMonitorWidget'ı başlat/durdur
    // Bu mantık NetworkMonitorWidget'ın kendi içinde olmalı.
    // Şimdilik, NetworkMonitorWidget'ın kendi start/stop butonları olduğunu varsayıyoruz.
}

DashboardWidget::~DashboardWidget() {
    // m_networkMonitor'ın stopMonitoring çağrısı NetworkMonitorWidget'ın destructor'ında veya uygun bir yerde yapılmalı.
    // if (m_networkMonitor) {
    //     m_networkMonitor->stopMonitoring();
    // }
    // m_networkMonitorWidget, networkPage'in çocuğu olduğu için otomatik silinecektir.
    delete ui;
}

/**
 * @brief Handles the Basic Scan button click.
 * 
 * Initiates the file selection process for a basic scan.
 */
void DashboardWidget::onBasicScanButtonClicked() {
    // This slot is likely deprecated if basicScanFileButton is used directly.
    // If still needed for some other purpose, its logic should be reviewed.
    // For now, we assume onBasicScanSelectFile is the primary action.
    qDebug() << "onBasicScanButtonClicked called (potentially deprecated)";
    // onBasicScanSelectFile(); // Optionally call the new handler
}

/**
 * @brief Handles the Advanced Scan button click.
 * 
 * Opens a file dialog for the user to select a file for advanced scanning.
 */
void DashboardWidget::onAdvancedScanButtonClicked() {
    // This slot is likely deprecated if advancedScanFileButton is used directly.
    qDebug() << "onAdvancedScanButtonClicked called (potentially deprecated)";
    // onAdvancedScanSelectFile(); // Optionally call the new handler
}

/**
 * @brief Handles the CDR Scan button click.
 * 
 * Currently displays a debug message and updates the UI.
 */
void DashboardWidget::onCdrScanButtonClicked() {
    ui->contentStackedWidget->setCurrentWidget(ui->malwareAnalysisPage);
    if (ui->malwareAnalysisTabWidget && ui->cdrResultsTab) {
        ui->malwareAnalysisTabWidget->setCurrentWidget(ui->cdrResultsTab);
    } else {
        qDebug() << "CDR results tab or malwareAnalysisTabWidget is null!";
        return;
    }

    if (ui->cdrResultsTextEdit) {
        ui->cdrResultsTextEdit->clear();
        ui->cdrResultsTextEdit->append(QString::fromLatin1(DashboardText::SELECTING_FILE));
    }

    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Select File for CDR Scan"),
                                                    QString(), // Default directory
                                                    tr("All Files (*.*)"));

    if (filePath.isEmpty()) {
        if (ui->cdrResultsTextEdit) {
            ui->cdrResultsTextEdit->append(QString::fromLatin1(DashboardText::SELECTION_CANCELED));
        }
        // if (ui->malwareAnalysisTabWidget && ui->cdrTab) { // Assuming cdrTab is the input tab
        //     // ui->malwareAnalysisTabWidget->setCurrentWidget(ui->cdrTab);
        // }
        return;
    }

    if (ui->cdrResultsTextEdit) {
        ui->cdrResultsTextEdit->append(QString::fromLatin1(DashboardText::FILE_SELECTED).arg(filePath));
        ui->cdrResultsTextEdit->append(QString::fromLatin1(DashboardText::CDR_SCAN)); // "Starting CDR Scan..."
    }
    
    qDebug() << "CDR Scan initiated for file:" << filePath;
    // Placeholder for actual CDR scan logic
    // m_cdrScanner->scanFile(filePath); // Example
    if (ui->cdrResultsTextEdit) {
        ui->cdrResultsTextEdit->append(tr("\\nCDR functionality is not yet implemented."));
    }
    QMessageBox::information(this, tr("CDR Scan"), tr("CDR Scan for %1 would start here. (Not Implemented Yet)").arg(filePath));
}

/**
 * @brief Handles the Sandbox Scan button click.
 * 
 * Currently displays a debug message and updates the UI.
 */
void DashboardWidget::onSandboxScanButtonClicked() {
    ui->contentStackedWidget->setCurrentWidget(ui->malwareAnalysisPage);
    if (ui->malwareAnalysisTabWidget && ui->sandboxResultsTab) {
        ui->malwareAnalysisTabWidget->setCurrentWidget(ui->sandboxResultsTab);
    } else {
        qDebug() << "Sandbox results tab or malwareAnalysisTabWidget is null!";
        return;
    }
    
    if (ui->sandboxResultsTextEdit) {
        ui->sandboxResultsTextEdit->clear();
        ui->sandboxResultsTextEdit->append(QString::fromLatin1(DashboardText::SELECTING_FILE));
    }

    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Select File for Sandbox Scan"),
                                                    QString(), // Default directory
                                                    tr("All Files (*.*)"));

    if (filePath.isEmpty()) {
        if (ui->sandboxResultsTextEdit) {
            ui->sandboxResultsTextEdit->append(QString::fromLatin1(DashboardText::SELECTION_CANCELED));
        }
        // if (ui->malwareAnalysisTabWidget && ui->sandboxTab) { // Assuming sandboxTab is the input tab
        //    // ui->malwareAnalysisTabWidget->setCurrentWidget(ui->sandboxTab);
        // }
        return;
    }

    if (ui->sandboxResultsTextEdit) {
        ui->sandboxResultsTextEdit->append(QString::fromLatin1(DashboardText::FILE_SELECTED).arg(filePath));
        ui->sandboxResultsTextEdit->append(QString::fromLatin1(DashboardText::SANDBOX_SCAN)); // "Starting Sandbox Scan..."
    }

    qDebug() << "Sandbox Scan initiated for file:" << filePath;
    // Placeholder for actual Sandbox scan logic
    // m_sandboxScanner->scanFile(filePath); // Example
    if (ui->sandboxResultsTextEdit) {
        ui->sandboxResultsTextEdit->append(tr("\\nSandbox functionality is not yet implemented."));
    }
    QMessageBox::information(this, tr("Sandbox Scan"), tr("Sandbox Scan for %1 would start here. (Not Implemented Yet)").arg(filePath));
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

    // Get the current page from the main stacked widget
    QWidget* currentPage = ui->contentStackedWidget->currentWidget();

    if (currentPage == ui->networkPage) {
        // Refresh the network page
        // The ui->networkCommunicationTextEdit was causing a compile error as it's not a direct member of Ui::DashboardWidget.
        // NetworkMonitorWidget is added to networkTabLayout and likely handles its own UI components.
        if (m_networkMonitor) {
            m_networkMonitor->stopMonitoring();
            m_networkMonitor->startMonitoring();
        }
    } else if (currentPage == ui->malwareAnalysisPage) {
        // If the malware analysis page is active, get the current tab from its QTabWidget
        QWidget* currentMalwareTab = ui->malwareAnalysisTabWidget->currentWidget();

        if (currentMalwareTab == ui->basicScanResultsTab) {
            // Clear basic scan results
            if (ui->basicScanResultsTextEdit) {
                ui->basicScanResultsTextEdit->clear();
            }
        } else if (currentMalwareTab == ui->advancedScanResultsTab) {
            // Clear advanced scan results
            if (ui->advancedScanResultsTableWidget) {
                ui->advancedScanResultsTableWidget->clearContents();
                ui->advancedScanResultsTableWidget->setRowCount(0);
            }
        } else if (currentMalwareTab == ui->cdrResultsTab) {
            // Clear CDR results
            if (ui->cdrResultsTextEdit) {
                ui->cdrResultsTextEdit->clear();
            }
        } else if (currentMalwareTab == ui->sandboxResultsTab) {
            // Clear sandbox results
            if (ui->sandboxResultsTextEdit) {
                ui->sandboxResultsTextEdit->clear();
            }
        }
    }
    // Add handling for other pages like overviewPage if necessary
}

/**
 * @brief Handles file selection and initiates the basic scan process.
 * 
 * Opens a file dialog for the user to select a file, then initiates
 * the scan of that file and updates the UI accordingly.
 */
void DashboardWidget::onBasicScanSelectFile() {
    ui->contentStackedWidget->setCurrentWidget(ui->malwareAnalysisPage);
    if (ui->malwareAnalysisTabWidget && ui->basicScanResultsTab) {
        ui->malwareAnalysisTabWidget->setCurrentWidget(ui->basicScanResultsTab);
    } else {
        qDebug() << "Basic scan results tab or malwareAnalysisTabWidget is null!";
        return;
    }

    if (ui->basicScanResultsTextEdit) {
        ui->basicScanResultsTextEdit->clear();
        ui->basicScanResultsTextEdit->append(QString::fromLatin1(DashboardText::SELECTING_FILE));
    } else {
        qDebug() << "basicScanResultsTextEdit is null!";
        // Fallback or error
    }

    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Select File for Basic Scan"),
                                                    QString(), // Default directory
                                                    tr("All Files (*.*)"));

    if (filePath.isEmpty()) {
        if (ui->basicScanResultsTextEdit) {
            ui->basicScanResultsTextEdit->append(QString::fromLatin1(DashboardText::SELECTION_CANCELED));
        }
        // Optionally switch back to the input tab
        // if (ui->malwareAnalysisTabWidget && ui->basicScanTab) {
        //     ui->malwareAnalysisTabWidget->setCurrentWidget(ui->basicScanTab);
        // }
        return;
    }

    if (ui->basicScanResultsTextEdit) {
        ui->basicScanResultsTextEdit->append(QString::fromLatin1(DashboardText::FILE_SELECTED).arg(filePath));
        ui->basicScanResultsTextEdit->append(QString::fromLatin1(DashboardText::SCANNING_FILE));
    }
    m_basicScanner->scanFile(filePath);
}

/**
 * @brief Handles file selection and initiates the advanced scan process.
 * 
 * Opens a file dialog for the user to select a file, then processes
 * that file for advanced scanning and updates the UI accordingly.
 */
void DashboardWidget::onAdvancedScanSelectFile() {
    ui->contentStackedWidget->setCurrentWidget(ui->malwareAnalysisPage);
    // Switching to results tab is handled by handleVirusTotalResults or when submission starts.
    // For now, ensure malwareAnalysisPage is active.
    // if (ui->malwareAnalysisTabWidget && ui->advancedScanTab) { // Or advancedScanResultsTab
    //    ui->malwareAnalysisTabWidget->setCurrentWidget(ui->advancedScanTab); // Stay on input tab or switch to results
    // }

    qDebug() << DashboardText::VT_SELECTING;

    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Select File for Advanced Scan/VirusTotal"),
                                                    QString(), // Default directory
                                                    tr("All Files (*.*)"));

    if (filePath.isEmpty()) {
        qDebug() << DashboardText::SELECTION_CANCELED;
        return;
    }

    qDebug() << QString::fromLatin1(DashboardText::FILE_SELECTED).arg(filePath);
    
    if (m_virusTotalManager) {
        qDebug() << DashboardText::VT_SUBMITTING;
        // The VirusTotalManager should ideally handle UI updates for "Submitting..."
        // and then "Retrieving report..." before emitting analysisResultsReady.
        // TODO: Verify VirusTotalManager API. 'submitFile' method caused a compile error.
        // m_virusTotalManager->submitFile(filePath); // Assumed method, ensure it exists in VirusTotalManager.h
    } else {
        qDebug() << "VirusTotalManager is not initialized.";
        QMessageBox::critical(this, QString::fromLatin1(DashboardText::VT_ERROR), tr("VirusTotalManager not available. Cannot submit file."));
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
    ui->contentStackedWidget->setCurrentWidget(ui->malwareAnalysisPage);
    if (ui->malwareAnalysisTabWidget && ui->basicScanResultsTab) {
        ui->malwareAnalysisTabWidget->setCurrentWidget(ui->basicScanResultsTab);
    } else {
        qDebug() << "Basic scan results tab or malwareAnalysisTabWidget is null for results ready!";
        return;
    }

    if (ui->basicScanResultsTextEdit) {
        // Append results, clearing was done when scan started
        ui->basicScanResultsTextEdit->append(QStringLiteral("\n--- Scan Results ---"));
        ui->basicScanResultsTextEdit->append(results);
    } else {
        qDebug() << "basicScanResultsTextEdit is null when results are ready!";
    }
    // Potentially update other UI elements like scan counters if they exist
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
    ui->contentStackedWidget->setCurrentWidget(ui->malwareAnalysisPage);
    if (ui->malwareAnalysisTabWidget && ui->basicScanResultsTab) {
        ui->malwareAnalysisTabWidget->setCurrentWidget(ui->basicScanResultsTab);
    } else {
        qDebug() << "Basic scan results tab or malwareAnalysisTabWidget is null for scan error!";
        // No return, still try to show QMessageBox
    }

    QString fullErrorMessage = QString::fromLatin1(DashboardText::ERROR_PREFIX).arg(errorMessage);
    if (ui->basicScanResultsTextEdit) {
        ui->basicScanResultsTextEdit->append(QStringLiteral("\\n--- Scan Error ---"));
        ui->basicScanResultsTextEdit->append(QStringLiteral("Error Code: %1").arg(static_cast<int>(errorCode)));
        ui->basicScanResultsTextEdit->append(fullErrorMessage);
    } else {
        qDebug() << "basicScanResultsTextEdit is null when handling scan error!";
    }
    QMessageBox::critical(this, QString::fromLatin1(DashboardText::SCAN_ERROR), fullErrorMessage);
}

/**
 * @brief Handles VirusTotal results and parses the JSON response.
 * 
 * Logs the detailed engine results and updates the UI with summary statistics.
 * 
 * @param results The JSON response from VirusTotal.
 */
void DashboardWidget::handleVirusTotalResults(const QString& results) {
    qDebug() << "Raw VirusTotal Results (handleVirusTotalResults):" << (results.length() > 200 ? results.left(200) + u"..."_s : results);
    
    ui->contentStackedWidget->setCurrentWidget(ui->malwareAnalysisPage);
    if (ui->malwareAnalysisTabWidget && ui->advancedScanResultsTab) {
        ui->malwareAnalysisTabWidget->setCurrentWidget(ui->advancedScanResultsTab);
    } else {
        qDebug() << "Advanced scan results tab or malwareAnalysisTabWidget is null for VT results!";
        QMessageBox::critical(this, QString::fromLatin1(DashboardText::VT_ERROR), tr("UI components for VirusTotal results are missing."));
        return;
    }

    if (!ui->advancedScanResultsTableWidget) {
        qDebug() << "advancedScanResultsTableWidget is null!";
        QMessageBox::critical(this, QString::fromLatin1(DashboardText::VT_ERROR), tr("Results table for VirusTotal is missing."));
        return;
    }
    
    ui->advancedScanResultsTableWidget->clearContents();
    ui->advancedScanResultsTableWidget->setRowCount(0);

    QJsonDocument jsonResponse = QJsonDocument::fromJson(results.toUtf8());
    if (jsonResponse.isNull() || !jsonResponse.isObject()) {
        qDebug() << "Failed to parse VirusTotal JSON response or response is not an object.";
        QMessageBox::critical(this, QString::fromLatin1(DashboardText::VT_ERROR), tr("Failed to parse VirusTotal results. Response was empty or malformed."));
        ui->advancedScanResultsTableWidget->setColumnCount(1);
        ui->advancedScanResultsTableWidget->setHorizontalHeaderLabels({tr("Status")});
        ui->advancedScanResultsTableWidget->insertRow(0);
        ui->advancedScanResultsTableWidget->setItem(0, 0, new QTableWidgetItem(tr("Error: Could not parse results from VirusTotal.")));
        return;
    }

    QJsonObject responseObject = jsonResponse.object();
    // Expected structure: { "data": { "attributes": { "last_analysis_stats": {}, "last_analysis_results": {} } } }
    // Or it could be an error object from VirusTotal API itself.
    if (responseObject.contains(QStringLiteral("error"))) {
        QJsonObject errorObj = responseObject.value(QStringLiteral("error")).toObject();
        QString errorMessage = errorObj.value(QStringLiteral("message")).toString(QStringLiteral("Unknown VirusTotal API error."));
        qDebug() << "VirusTotal API Error:" << errorMessage;
        QMessageBox::critical(this, QString::fromLatin1(DashboardText::VT_ERROR), tr("VirusTotal API Error: %1").arg(errorMessage));
        ui->advancedScanResultsTableWidget->setColumnCount(1);
        ui->advancedScanResultsTableWidget->setHorizontalHeaderLabels({tr("Status")});
        ui->advancedScanResultsTableWidget->insertRow(0);
        ui->advancedScanResultsTableWidget->setItem(0, 0, new QTableWidgetItem(tr("VirusTotal API Error: %1").arg(errorMessage)));
        return;
    }


    if (!responseObject.contains(QStringLiteral("data")) || !responseObject.value(QStringLiteral("data")).isObject()) {
        qDebug() << "VirusTotal JSON response does not contain 'data' object.";
        QMessageBox::critical(this, QString::fromLatin1(DashboardText::VT_ERROR), tr("VirusTotal results format is unexpected (missing 'data')."));
        // ... (similar error display in table) ...
        return;
    }
    QJsonObject data = responseObject.value(QStringLiteral("data")).toObject();

    if (!data.contains(QStringLiteral("attributes")) || !data.value(QStringLiteral("attributes")).isObject()) {
        qDebug() << "VirusTotal JSON 'data' does not contain 'attributes' object.";
        QMessageBox::critical(this, QString::fromLatin1(DashboardText::VT_ERROR), tr("VirusTotal results format is unexpected (missing 'attributes')."));
        // ... (similar error display in table) ...
        return;
    }
    QJsonObject attributes = data.value(QStringLiteral("attributes")).toObject();

    // Populate table header
    ui->advancedScanResultsTableWidget->setColumnCount(3); // Engine, Category, Result
    ui->advancedScanResultsTableWidget->setHorizontalHeaderLabels({tr("Scan Engine"), tr("Category"), tr("Detection Result")});
    
    int currentRow = 0;
    if (attributes.contains(QStringLiteral("last_analysis_results")) && attributes.value(QStringLiteral("last_analysis_results")).isObject()) {
        QJsonObject engineResults = attributes.value(QStringLiteral("last_analysis_results")).toObject();
        for (auto it = engineResults.constBegin(); it != engineResults.constEnd(); ++it) {
            ui->advancedScanResultsTableWidget->insertRow(currentRow);
            QJsonObject engineDetail = it.value().toObject();
            ui->advancedScanResultsTableWidget->setItem(currentRow, 0, new QTableWidgetItem(it.key()));
            ui->advancedScanResultsTableWidget->setItem(currentRow, 1, new QTableWidgetItem(engineDetail.value(QStringLiteral("category")).toString()));
            ui->advancedScanResultsTableWidget->setItem(currentRow, 2, new QTableWidgetItem(engineDetail.value(QStringLiteral("result")).toString()));
            currentRow++; // Increment currentRow here
        }
    } else {
        ui->advancedScanResultsTableWidget->insertRow(currentRow);
        ui->advancedScanResultsTableWidget->setItem(currentRow, 0, new QTableWidgetItem(tr("Info")));
        ui->advancedScanResultsTableWidget->setItem(currentRow, 1, new QTableWidgetItem(QString::fromLatin1(DashboardText::VT_ANALYSIS_PENDING)));
        ui->advancedScanResultsTableWidget->setItem(currentRow, 2, new QTableWidgetItem(tr("No detailed engine results available.")));
         currentRow++;
    }

    if (attributes.contains(QStringLiteral("last_analysis_stats")) && attributes.value(QStringLiteral("last_analysis_stats")).isObject()) {
        QJsonObject stats = attributes.value(QStringLiteral("last_analysis_stats")).toObject();
        // Example of how to use stats, adapt as needed for your UI (e.g., status bar, dedicated labels)
        QString summary = tr("Scan Summary: Malicious: %1, Suspicious: %2, Undetected: %3, Harmless: %4, Timeout: %5")
                              .arg(stats.value(QStringLiteral("malicious")).toInt(0))
                              .arg(stats.value(QStringLiteral("suspicious")).toInt(0))
                              .arg(stats.value(QStringLiteral("undetected")).toInt(0))
                              .arg(stats.value(QStringLiteral("harmless")).toInt(0))
                              .arg(stats.value(QStringLiteral("timeout")).toInt(0));
        qDebug() << summary; // Log summary, or display it in the UI

        // You might want to add this summary to the table or a separate label
        // For example, adding it as a new row in the table:
        /*
        ui->advancedScanResultsTableWidget->insertRow(currentRow);
        QTableWidgetItem *summaryItem = new QTableWidgetItem(summary);
        summaryItem->setTextAlignment(Qt::AlignCenter); // Optional: center align
        ui->advancedScanResultsTableWidget->setItem(currentRow, 0, summaryItem);
        ui->advancedScanResultsTableWidget->setSpan(currentRow, 0, 1, 3); // Span across all 3 columns
        currentRow++;
        */
    }
    
    ui->advancedScanResultsTableWidget->resizeColumnsToContents();
    ui->advancedScanResultsTableWidget->resizeRowsToContents();
}

/**
 * @brief Appends a log message to the network communication text edit.
 * @param logMessage The message to append.
 */
void DashboardWidget::appendNetworkLog(const QString& logMessage) {
    if (m_networkMonitorWidget) {
        m_networkMonitorWidget->appendNetworkLog(logMessage);
    }
}

// Yeni UI için ek fonksiyonlar (gerekirse)
// void DashboardWidget::updateNetworkMonitoringUI() {
//    // Bu fonksiyon NetworkMonitorWidget'tan gelen sinyallere göre
//    // DashboardWidget üzerindeki genel bir durumu (örneğin bir status ikonu) güncelleyebilir.
//    // Şimdilik NetworkMonitorWidget kendi UI'ını yönettiği için bu gerekmeyebilir.
// }
