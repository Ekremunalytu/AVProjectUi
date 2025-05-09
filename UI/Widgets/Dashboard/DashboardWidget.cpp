#include "DashboardWidget.h"
#include "ui_dashboardwidget.h"
#include "../../../Database/DatabaseService/DatabaseService.h"
#include <QDebug>
#include <QAction>
#include <QMessageBox>

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
    m_basicScanMenu(new QMenu(this))
{
    ui->setupUi(this);
    
    // Initialize basic scan menu
    QAction* selectFileAction = new QAction(tr("Select File..."), this);
    m_basicScanMenu->addAction(selectFileAction);
    
    // Connect click events for all buttons
    connect(ui->basicScanButton_dashboard, &QPushButton::clicked, this, &DashboardWidget::onBasicScanClicked);
    connect(ui->advancedScanButton_dashboard, &QPushButton::clicked, this, &DashboardWidget::onAdvancedScanClicked);
    connect(ui->cdrScanButton_dashboard, &QPushButton::clicked, this, &DashboardWidget::onCdrScanClicked);
    connect(ui->sandboxScanButton_dashboard, &QPushButton::clicked, this, &DashboardWidget::onSandboxScanClicked);
    
    // Basic scan connections
    connect(selectFileAction, &QAction::triggered, this, &DashboardWidget::onBasicScanSelectFile);
    connect(m_basicScanner.get(), &BasicScanner::scanResultsReady, this, &DashboardWidget::onBasicScanResultsReady);
    connect(m_basicScanner.get(), &BasicScanner::scanError, this, &DashboardWidget::onBasicScanError);
}

/**
 * @brief Destroys the DashboardWidget and releases resources.
 * 
 * The basic scan menu is automatically destroyed as it's a child of this widget.
 */
DashboardWidget::~DashboardWidget() {
    delete ui;
    // m_basicScanMenu will be automatically destroyed as it's a child of this widget
}

/**
 * @brief Shows the basic scan options menu when the Basic Scan button is clicked.
 */
void DashboardWidget::onBasicScanClicked() {
    // Show menu under the button
    m_basicScanMenu->exec(ui->basicScanButton_dashboard->mapToGlobal(
        QPoint(0, ui->basicScanButton_dashboard->height())));
}

/**
 * @brief Handles the Advanced Scan button click.
 * 
 * Currently displays a debug message and updates the UI.
 */
void DashboardWidget::onAdvancedScanClicked() {
    qDebug() << "Starting Advanced Scan...";
    ui->scanResultsTextEdit_dashboard->append(tr("Starting Advanced Scan..."));
}

/**
 * @brief Handles the CDR Scan button click.
 * 
 * Currently displays a debug message and updates the UI.
 */
void DashboardWidget::onCdrScanClicked() {
    qDebug() << "Starting CDR Scan...";
    ui->scanResultsTextEdit_dashboard->append(tr("Starting CDR Scan..."));
}

/**
 * @brief Handles the Sandbox Scan button click.
 * 
 * Currently displays a debug message and updates the UI.
 */
void DashboardWidget::onSandboxScanClicked() {
    qDebug() << "Starting Sandbox Scan...";
    ui->scanResultsTextEdit_dashboard->append(tr("Starting Sandbox Scan..."));
}

/**
 * @brief Handles file selection and initiates the basic scan process.
 * 
 * Opens a file dialog for the user to select a file, then initiates
 * the scan of that file and updates the UI accordingly.
 */
void DashboardWidget::onBasicScanSelectFile() {
    ui->scanResultsTextEdit_dashboard->clear();
    ui->scanResultsTextEdit_dashboard->append(tr("Selecting file..."));
    
    if (m_basicScanner->selectFile()) {
        ui->scanResultsTextEdit_dashboard->append(tr("File selected: %1").arg(
            m_basicScanner->getSelectedFile().fileName()));
        ui->scanResultsTextEdit_dashboard->append(tr("Scanning file..."));
        
        // Initiate scan with selected file
        if (!m_basicScanner->scanFile(m_basicScanner->getSelectedFile().filePath())) {
            // Handle scan initiation error - already handled by error signal, 
            // but we can add additional UI updates if needed
            if (m_basicScanner->getLastError() != ScannerErrorCode::NoError) {
                ui->scanResultsTextEdit_dashboard->append(tr("Error: %1").arg(
                    m_basicScanner->getLastErrorMessage()));
            }
        }
    } else {
        if (m_basicScanner->getLastError() == ScannerErrorCode::NoError) {
            // User canceled file selection, not an error
            ui->scanResultsTextEdit_dashboard->append(tr("File selection canceled."));
        } else {
            // Handle file selection error
            ui->scanResultsTextEdit_dashboard->append(tr("Error: %1").arg(
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
            errorTitle = tr("Database Error");
            errorIcon = u":/UI/Resources/Images/applogo.png"_qs;  // Using Unicode string literal with _qs suffix
            break;
        case ScannerErrorCode::FileNotFound:
        case ScannerErrorCode::FileNotReadable:
            errorTitle = tr("File Error");
            errorIcon = u":/UI/Resources/Images/applogo.png"_qs;
            break;
        case ScannerErrorCode::HashCalculationFailed:
            errorTitle = tr("Scanning Error");
            errorIcon = u":/UI/Resources/Images/applogo.png"_qs;
            break;
        default:
            errorTitle = tr("Error");
            errorIcon = u":/UI/Resources/Images/applogo.png"_qs;
            break;
    }
    
    // Log the error
    qWarning() << "Basic Scan Error:" << static_cast<int>(errorCode) << "-" << errorMessage;
    
    // Add error message to the scan results text edit
    if (ui->scanResultsTextEdit_dashboard->toPlainText().isEmpty()) {
        ui->scanResultsTextEdit_dashboard->setText(tr("Error: %1").arg(errorMessage));
    } else if (!ui->scanResultsTextEdit_dashboard->toPlainText().contains(errorMessage)) {
        // Only append if the error message isn't already there
        ui->scanResultsTextEdit_dashboard->append(tr("Error: %1").arg(errorMessage));
    }
    
    // For critical errors, show a message box
    if (errorCode == ScannerErrorCode::DatabaseNotConnected || 
        errorCode == ScannerErrorCode::DatabaseQueryFailed) {
        QMessageBox::critical(this, errorTitle, errorMessage);
    }
}
