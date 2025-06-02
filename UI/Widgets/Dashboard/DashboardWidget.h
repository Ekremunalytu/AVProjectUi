#ifndef  DASHBOARDWIDGET_H
#define  DASHBOARDWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QString>
#include <QTimer>
#include <memory> // Keep this if other parts of the header use it, otherwise it might be removable if VirusTotalManager is the only user.
#include "Network/VirusTotal/VirusTotalManager.h"
#include "Network/Monitor/NetworkMonitor.h"
#include "Interface/ScannerTypes.h" // Ensure this is present for ScannerErrorCode

QT_BEGIN_NAMESPACE
namespace Ui { class DashboardWidget; }
QT_END_NAMESPACE

// Forward declaration
class DbManager;
class BasicScanner; // Forward declaration for BasicScanner
class CDRScanner;   // Forward declaration for CDRScanner

/**
 * @brief The DashboardWidget class provides the main dashboard interface for scanning operations.
 * 
 * This widget uses a tabbed interface for different scan types (basic, advanced, CDR, sandbox, network)
 * and displays the scan results in their respective tabs. It serves as the primary user interface for initiating
 * scans and viewing their results.
 */
class DashboardWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Constructs a DashboardWidget.
     * @param parent The parent widget.
     */
    explicit DashboardWidget(DbManager* dbManager, QWidget *parent = nullptr); // Modified constructor
    
    /**
     * @brief Destroys the DashboardWidget and frees resources.
     */
    ~DashboardWidget() override;

public slots:
    void handleVirusTotalResults(const QString& results);
    void appendNetworkLog(const QString& logMessage); // New slot for network logs

private slots:
    /**
     * @brief Processes and displays scan results from BasicScanner.
     * @param results String containing the scan results.
     */
    void onBasicScanResultsReady(const QString& results); // Kept - matches connection and definition
    
    /**
     * @brief Handles and displays scanner errors from BasicScanner.
     * @param errorCode The error code.
     * @param errorMessage A descriptive error message.
     */
    void handleScanError(ScannerErrorCode errorCode, const QString& errorMessage); // Kept - matches connection and definition
    void handleDirectoryScanStarted(const QString& directoryPath);
    void handleFileProcessed(const QString& filePath, const QString& result, bool isMalicious, int progressValue);
    void handleDirectoryScanFinished(const QString& directoryPath, int filesScanned, int threatsFound);

    // Slots for UI elements (auto-connected or manually connected)
    void on_basicScanButton_dashboard_clicked();
    void on_advancedScanButton_dashboard_clicked();
    void on_cdrScanButton_dashboard_clicked();
    void on_sandboxScanButton_dashboard_clicked();
    void on_configButton_clicked(); // Assuming a configButton might exist or be planned
    void on_refreshButton_clicked(); // Assuming a refreshButton might exist or be planned
    void onRefreshButtonClicked(); // Declaration for the existing implementation
    void onAdvancedScanSelectFile(); // Declaration for the existing implementation

    // New slots for new buttons
    void on_selectFileButton_dashboard_clicked();
    void on_scanDirectoryButton_dashboard_clicked();
    void onNetworkMonitorButtonClicked(); // Added declaration
    
    // CDR scan result handling
    void handleCDRScanCompletion();
    void checkCDRScanStatus();

private:
    Ui::DashboardWidget *ui; ///< Pointer to the UI form
    DbManager* m_dbManager; // Added DbManager member
    BasicScanner* m_basicScanner; 
    CDRScanner* m_cdrScanner; ///< Scanner for CDR operations
    VirusTotalManager* m_virusTotalManager; // Added m_virusTotalManager
    NetworkMonitor *m_networkMonitor; ///< Network monitor instance
    QString m_selectedFileForBasicScan; // To store selected file path
    QTimer* m_cdrStatusTimer; // Timer to poll CDR scan status

    void setupConnections();
    void initializeScanners(); // Added for scanner initialization
    void updateBasicScanUI(bool scanning);
};

#endif // DASHBOARDWIDGET_H