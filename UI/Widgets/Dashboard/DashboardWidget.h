#ifndef  DASHBOARDWIDGET_H
#define  DASHBOARDWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QString>
#include <memory>
#include "Scanner/Dashboard/BasicScanner/BasicScanner.h"
#include "Network/VirusTotal/VirusTotalManager.h"
#include "Network/Monitor/NetworkMonitor.h" // Added include
#include "Scanner/CDRScanner.h" // Added for CDRScanner
#include "../../Docker/include/docker/DockerManager.h" // Added for Docker management
#include "../../Docker/include/cdr/CdrManager.h" // Added for CDR management
#include "../../Sandbox/SandboxManager.h" // Added for Sandbox management
#include "../../Docker/include/docker/DockerTypes.h" // Added for Docker types
#include "../../Docker/include/cdr/CdrTypes.h" // Added for CDR types

QT_BEGIN_NAMESPACE
namespace Ui { class DashboardWidget; }
QT_END_NAMESPACE

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
    explicit DashboardWidget(QWidget *parent = nullptr);
    
    /**
     * @brief Destroys the DashboardWidget and frees resources.
     */
    ~DashboardWidget();

public slots:
    void handleVirusTotalResults(const QString& results);
    void appendNetworkLog(const QString& logMessage); // New slot for network logs

private slots:
    /**
     * @brief Handles click on the Basic Scan button in the Basic Scan tab.
     */
    void onBasicScanButtonClicked();
    
    /**
     * @brief Handles click on the Advanced Scan button in the Advanced Scan tab.
     */
    void onAdvancedScanButtonClicked();
    
    /**
     * @brief Handles click on the CDR Scan button in the CDR tab.
     */
    void onCdrScanButtonClicked();
    
    /**
     * @brief Handles click on the Sandbox Scan button in the Sandbox tab.
     */
    void onSandboxScanButtonClicked();
    
    /**
     * @brief Handles click on the Network Monitor button in the Network tab.
     */
    void onNetworkMonitorButtonClicked();
    
    /**
     * @brief Handles click on the Configuration button at the bottom of the interface.
     */
    void onConfigButtonClicked();
    
    /**
     * @brief Handles click on the Refresh button at the bottom of the interface.
     */
    void onRefreshButtonClicked();
    
    /**
     * @brief Handles the file selection for Basic Scan.
     * 
     * Opens a file dialog and initiates scanning of the selected file.
     */
    void onBasicScanSelectFile();
    
    /**
     * @brief Handles the file selection for Advanced Scan.
     * 
     * Opens a file dialog and processes the selected file for advanced scanning.
     */
    void onAdvancedScanSelectFile();
    
    /**
     * @brief Processes and displays scan results.
     * @param results String containing the scan results.
     */
    void onBasicScanResultsReady(const QString& results);
    
    /**
     * @brief Handles and displays scanner errors.
     * @param errorCode The error code.
     * @param errorMessage A descriptive error message.
     */
    void onBasicScanError(ScannerErrorCode errorCode, const QString& errorMessage);

private:
    Ui::DashboardWidget *ui; ///< Pointer to the UI form
    std::unique_ptr<BasicScanner> m_basicScanner; ///< Scanner for basic file scanning
    std::unique_ptr<VirusTotalManager> m_virusTotalManager; ///< Manager for VirusTotal API integration
    std::unique_ptr<CDRScanner> m_cdrScanner; ///< Scanner for CDR operations
    NetworkMonitor *m_networkMonitor; ///< Network monitor instance    std::unique_ptr<Docker::DockerManager> m_dockerManager; ///< Docker container management
    std::unique_ptr<CDR::CdrManager> m_cdrManager; ///< CDR analysis and sanitization management
    std::unique_ptr<Sandbox::SandboxManager> m_sandboxManager; ///< Sandbox analysis and execution management
};

#endif // DASHBOARDWIDGET_H